#include "BasePCH.h"
#include "Core2.h"
#include "WindowHandler.h"
#include "Device.h"

//////////////////////////////////////////////////////////////////////////
// window init parameter structure

CCoreWindowParameters::CCoreWindowParameters()
{
	Device = NULL;
	hInstance = 0;
	FullScreen = false;
	XRes = 800;
	YRes = 600;
	WindowTitle = NULL;
	Icon = 0;
	Maximized = false;
	ResizeDisabled = false;
}

CCoreWindowParameters::CCoreWindowParameters(HINSTANCE hinst, TBOOL fs, TS32 x, TS32 y, TCHAR *title, HICON icon, TBOOL maximize, TBOOL noresize)
{
	Initialize(new CCore(), hinst, fs, x, y, title, icon, maximize, noresize);
}

void CCoreWindowParameters::Initialize(CCoreDevice *device, HINSTANCE hinst, TBOOL fs, TS32 x, TS32 y, TCHAR *title, HICON icon, TBOOL maximize, TBOOL noresize)
{
	Device = device;
	hInstance = hinst;
	FullScreen = fs;
	XRes = x;
	YRes = y;
	WindowTitle = title;
	Icon = icon;
	Maximized = maximize;
	ResizeDisabled = noresize;
}


//////////////////////////////////////////////////////////////////////////
// windowhandler baseclass

CCoreWindowHandler::CCoreWindowHandler()
{
	Device = NULL;
	Done = false;
	XRes = YRes = 0;
	CurrentMouseCursor = CM_ARROW;

	InactiveFrameLimiter = true;
	LimitedFPS = 20;
	LastRenderedFrame = globalTimer.GetTime();
}

CCoreWindowHandler::~CCoreWindowHandler()
{
	Destroy();
	SAFEDELETE(Device);
}

void CCoreWindowHandler::Destroy()
{
	Done = true;
}

TS32 CCoreWindowHandler::GetXRes()
{
	return XRes;
}

TS32 CCoreWindowHandler::GetYRes()
{
	return YRes;
}

CCoreWindowParameters &CCoreWindowHandler::GetInitParameters()
{
	return InitParameters;
}

void CCoreWindowHandler::SelectMouseCursor(COREMOUSECURSOR m)
{
	CurrentMouseCursor = m;
}

CPoint CCoreWindowHandler::GetMousePos()
{
	return MousePos;
}

CPoint CCoreWindowHandler::GetLeftDownPos()
{
	return LeftDownPos;
}

CPoint CCoreWindowHandler::GetRightDownPos()
{
	return RightDownPos;
}

CPoint CCoreWindowHandler::GetMidDownPos()
{
	return MidDownPos;
}

void CCoreWindowHandler::SetInactiveFrameLimiter(TBOOL set)
{
	InactiveFrameLimiter = set;
}


//////////////////////////////////////////////////////////////////////////
// windows windowhandler

CCoreWindowHandlerWin::CCoreWindowHandlerWin() :CCoreWindowHandler()
{
	hWnd = NULL;
	WindowPlacement.length = sizeof(WINDOWPLACEMENT);
	dwStyle = 0;
	FullScreenX = FullScreenY = 0;
}

CCoreWindowHandlerWin::~CCoreWindowHandlerWin()
{
	while (MouseCursors.NumItems())
	{
		DeleteObject(MouseCursors[MouseCursors.NumItems() - 1]);
		MouseCursors.DeleteByIndex(MouseCursors.NumItems() - 1);
	}
}

TBOOL CCoreWindowHandlerWin::Initialize(const CCoreWindowParameters &wp)
{
  XRes = wp.XRes;
	YRes = wp.YRes;
	InitParameters = wp;

	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc = WndProcProxy;
	wc.hInstance = wp.hInstance;
	wc.hIcon = wp.Icon;
	wc.lpszClassName = _T("CoRE2");
	RegisterClass(&wc);

	RECT WindowRect;
	WindowRect.left = 0;
	WindowRect.right = XRes;
	WindowRect.top = 0;
	WindowRect.bottom = YRes;

	if (!wp.FullScreen)
	{
		dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED | WS_MINIMIZEBOX | ((WS_MAXIMIZEBOX | WS_SIZEBOX)*(!wp.ResizeDisabled)) | (WS_MAXIMIZE*wp.Maximized);
		FullScreenX = GetSystemMetrics(SM_CXSCREEN);
		FullScreenY = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		dwStyle = WS_POPUP | WS_OVERLAPPED;
		FullScreenX = XRes;
		FullScreenY = YRes;
	}

	if (!wp.OverrideWindowStyleEx)
	{
		dwStyle = dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		if (wp.OverrideWindowStyle)
			dwStyle = wp.OverrideWindowStyle;
		AdjustWindowRect(&WindowRect, dwStyle, FALSE);
    hWnd = CreateWindow( _T( "CoRE2" ), wp.WindowTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, wp.hInstance, this );
	}
	else
	{
		dwStyle = wp.OverrideWindowStyle;
		AdjustWindowRect(&WindowRect, dwStyle, FALSE);
    hWnd = CreateWindowEx( wp.OverrideWindowStyleEx, _T( "CoRE2" ), wp.WindowTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, wp.hInstance, this );
	}

	Device = wp.Device;

	if (!Device)
	{
		LOG_ERR("[init] Device object is NULL during init.");
		return false;
	}

	if (!Device->Initialize(this))
	{
		SAFEDELETE(Device);
		return false;
	}

	ShowWindow(hWnd, Maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	MouseCursors += LoadCursor(NULL, IDC_ARROW);
	MouseCursors += LoadCursor(NULL, IDC_CROSS);
	MouseCursors += LoadCursor(NULL, IDC_SIZEWE);
	MouseCursors += LoadCursor(NULL, IDC_SIZENS);
	MouseCursors += LoadCursor(NULL, IDC_SIZENESW);
	MouseCursors += LoadCursor(NULL, IDC_SIZENWSE);
	MouseCursors += LoadCursor(NULL, IDC_IBEAM);
	MouseCursors += LoadCursor(NULL, IDC_WAIT);

	Maximized = wp.Maximized;
	Minimized = false;
	Active = true;

	RECT r;
	GetClientRect(hWnd, &r);
	ClientRect = CRect(r.left, r.top, r.right, r.bottom);

	if (wp.Maximized)
	{
		RECT r2;
		GetClientRect(hWnd, &r2);

		XRes = r2.right - r2.left;
		YRes = r2.bottom - r2.top;
	}

	return true;
}

TBOOL CCoreWindowHandlerWin::HandleMessages()
{
	return HandleOSMessages();
}

TBOOL CCoreWindowHandlerWin::HandleOSMessages()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return !Done;
}

TBOOL CCoreWindowHandlerWin::DeviceOK()
{
	if (!Active)
	{
		if (!InactiveFrameLimiter) return Device && Device->DeviceOk();

		TS32 time = globalTimer.GetTime();
		if (time - LastRenderedFrame >= 1000 / LimitedFPS)
		{
			LastRenderedFrame = time;
			return true;
		}
		return false;
	}

	if (!Device) return false;
	return Device->DeviceOk();
}

void CCoreWindowHandlerWin::Destroy()
{
	Done = true;
	if (hWnd)
	{
		DestroyWindow(hWnd);
		hWnd = NULL;
	}
}

void CCoreWindowHandlerWin::ToggleFullScreen()
{
	if (!Device) return;
	if (Device->IsWindowed())
	{
		//go to fullscreen
		dwStyle = GetWindowLong(hWnd, GWL_STYLE);
		GetWindowPlacement(hWnd, &WindowPlacement);
		ShowWindow(hWnd, SW_HIDE);
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		Device->SetFullScreenMode(true, FullScreenX, FullScreenY);
		//Device->Resize(FullScreenX,FullScreenY,false);
		HandleResize();
		ShowWindow(hWnd, SW_SHOW);
	}
	else
	{
		//go to window mode
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, 0);
		SetWindowLongPtr(hWnd, GWL_STYLE, dwStyle);
		SetWindowPlacement(hWnd, &WindowPlacement);
		//Device->Resize(0,0,true);
		Device->SetFullScreenMode(false, 0, 0);
		ShowWindow(hWnd, SW_SHOW);
		HandleResize();
	}
}

void CCoreWindowHandlerWin::HandleAltEnter()
{
	switch (Device->GetAPIType())
	{
		case COREAPI_DX9:
			ToggleFullScreen();
			break;
		case COREAPI_DX11:
			//handled by dxgi <3
			break;
		case COREAPI_OPENGL:
			break;
		default:
			break;
	}
}

LRESULT CALLBACK CCoreWindowHandlerWin::WndProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCoreWindowHandlerWin *wnd = NULL;

	if (uMsg == WM_NCCREATE)
	{
		wnd = (CCoreWindowHandlerWin*)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)wnd);
		wnd->hWnd = hWnd;
	}
	else
		wnd = (CCoreWindowHandlerWin*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (wnd)
		return wnd->WindowProc(uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CCoreWindowHandlerWin::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!hWnd) return 0;

	switch (uMsg)
	{
		case WM_CLOSE:
			Done = true;
			return 0;
			break;
		case WM_ACTIVATE:
		{
			Active = wParam != WA_INACTIVE;
			break;
		}

		case WM_ENTERSIZEMOVE:
		{
			Active = false;
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			Active = true;
			HandleResize();
		}
			break;
		case WM_SIZE:
		{
			if (Device && Device->IsWindowed() && hWnd)
				dwStyle = GetWindowLong(hWnd, GWL_STYLE);

			if (wParam == SIZE_MINIMIZED)
			{
				Active = false;
				Minimized = true;
				Maximized = false;
			}

			if (wParam == SIZE_MAXIMIZED)
			{
				Active = true;
				Minimized = false;
				Maximized = true;
				HandleResize();
			}

			if (wParam == SIZE_RESTORED)
			{
				if (Maximized)
				{
					Maximized = false;
					HandleResize();
				}
				else
					if (Minimized)
					{
						Active = true;
						Minimized = false;
						HandleResize();
					}

				// If we're neither maximized nor minimized, the window size
				// is changing by the user dragging the window edges.  In this 
				// case, we don't reset the device yet -- we wait until the 
				// user stops dragging, and a WM_EXITSIZEMOVE message comes.
			}

		}
			break;
		case WM_SYSKEYDOWN:
		{
			LOG_ERR( "[wndproc] WM_SYSKEYDOWN %d %d", wParam, lParam );
			if (wParam == VK_F10) return 0; // if we dont do this, system menu opens up on F10 = bad for fraps
			if ( wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU )
			{
				return 0;
			}
			if (wParam == VK_RETURN)
			{
				HandleAltEnter();
				break;
			}
			break;
		}
		case WM_SYSCOMMAND:
		{
      LOG_ERR( "[wndproc] WM_SYSCOMMAND %d %d", wParam, lParam );
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
				case SC_KEYMENU: //don't ding on alt+enter dammit
					return 0;
			}
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			SetCapture(hWnd);
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			ReleaseCapture();
			break;
		default:
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CCoreWindowHandlerWin::HandleResize()
{
	CRect old = ClientRect;

	RECT r;
	GetClientRect(hWnd, &r);
	ClientRect = CRect(r.left, r.top, r.right, r.bottom);
	XRes = ClientRect.Width();
	YRes = ClientRect.Height();

	if (Device && (old.Width() != XRes || old.Height() != YRes))
		Device->Resize(XRes, YRes);
}

TU32 CCoreWindowHandlerWin::GetHandle()
{
	return (TU32)hWnd;
}

void CCoreWindowHandlerWin::FinalizeMouseCursor()
{
	POINT ap;
	GetCursorPos(&ap);
	ScreenToClient(hWnd, &ap);
	CPoint mp = CPoint(ap.x, ap.y);
	RECT ClientRect;
	GetClientRect(hWnd, &ClientRect);

	if (CRect(0, 0, ClientRect.right, ClientRect.bottom).Contains(mp))
		SetCursor(MouseCursors[CurrentMouseCursor]);
}

void CCoreWindowHandlerWin::SetWindowTitle(CString &Title)
{
	SetWindowText(hWnd, Title.GetPointer());
}
