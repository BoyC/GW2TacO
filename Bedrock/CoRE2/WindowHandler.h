#pragma once
#include "../BaseLib/BaseLib.h"

class CCoreDevice;

enum COREMOUSECURSOR
{
	CM_ARROW,
	CM_CROSS,
	CM_SIZEWE,
	CM_SIZENS,
	CM_SIZENESW,
	CM_SIZENWSE,
	CM_TEXT,
	CM_WAIT,
};

//////////////////////////////////////////////////////////////////////////
// window init parameter structure

class CCoreWindowParameters
{
public:

  CCoreDevice *Device = nullptr;
	HINSTANCE hInstance;
	TBOOL FullScreen;
	TS32 XRes;
	TS32 YRes;
	TCHAR *WindowTitle;
	HICON Icon;
	TBOOL Maximized;
	TBOOL ResizeDisabled;

	DWORD OverrideWindowStyle = 0;
	DWORD OverrideWindowStyleEx = 0;

	CCoreWindowParameters();
	CCoreWindowParameters(HINSTANCE hinst, TBOOL FullScreen, TS32 XRes, TS32 YRes, TCHAR *WindowTitle, HICON Icon = 0, TBOOL Maximized = false, TBOOL ResizeDisabled = false);

	virtual void Initialize(CCoreDevice *device, HINSTANCE hinst, TBOOL FullScreen, TS32 XRes, TS32 YRes, TCHAR *WindowTitle, HICON Icon = 0, TBOOL Maximized = false, TBOOL ResizeDisabled = false);
};


//////////////////////////////////////////////////////////////////////////
// interface

class CCoreWindowHandler
{
protected:

	TBOOL Done;
	CCoreDevice *Device;
	TBOOL Active;
	TBOOL Maximized;
	TBOOL Minimized;
	CRect ClientRect;

	TBOOL InactiveFrameLimiter;
	TS32 LimitedFPS;
	TS32 LastRenderedFrame;

	TS32 XRes, YRes;

	CCoreWindowParameters InitParameters;

	COREMOUSECURSOR CurrentMouseCursor;

	CPoint MousePos, LeftDownPos, RightDownPos, MidDownPos;

	virtual void HandleResize() = 0;
	virtual void HandleAltEnter() = 0;

public:

	CCoreWindowHandler();
	virtual ~CCoreWindowHandler();

	//this initializer will change to accommodate multiple platforms at once once we get to that point:
	virtual TBOOL Initialize(const CCoreWindowParameters &WindowParams) = 0;

	virtual void Destroy();
	virtual TBOOL HandleMessages() = 0;
	virtual TBOOL HandleOSMessages() = 0;
	virtual TBOOL DeviceOK() = 0;
	virtual void ToggleFullScreen() = 0;

	virtual TU32 GetHandle() = 0;

	virtual TS32 GetXRes();
	virtual TS32 GetYRes();
	virtual CCoreWindowParameters &GetInitParameters();

	virtual void SelectMouseCursor(COREMOUSECURSOR Cursor);
	virtual void FinalizeMouseCursor() = 0;
	CPoint GetMousePos();
	CPoint GetLeftDownPos();
	CPoint GetRightDownPos();
	CPoint GetMidDownPos();

	INLINE CCoreDevice *GetDevice() { return Device; }

	virtual void SetWindowTitle(CString &Title) = 0;
	virtual void SetInactiveFrameLimiter(TBOOL set);

	TBOOL IsMinimized() { return Minimized; }

};

//////////////////////////////////////////////////////////////////////////
// windows implementation

class CCoreWindowHandlerWin : public CCoreWindowHandler
{
protected:

	HWND hWnd;
	WINDOWPLACEMENT WindowPlacement;
	TS32 dwStyle;
	TS32 FullScreenX, FullScreenY;

	static LRESULT CALLBACK WndProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	CArray<HCURSOR> MouseCursors;

	virtual void HandleResize();
	virtual void HandleAltEnter();

public:

	CCoreWindowHandlerWin();
	virtual ~CCoreWindowHandlerWin();

	virtual TBOOL Initialize(const CCoreWindowParameters &WindowParams);
	virtual void Destroy();
	virtual TBOOL HandleMessages();
	virtual TBOOL HandleOSMessages();
	virtual TBOOL DeviceOK();
	virtual void ToggleFullScreen();

	TU32 GetHandle();

	virtual void FinalizeMouseCursor();
	virtual void SetWindowTitle(CString &Title);
};