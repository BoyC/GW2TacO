#include "OverlayApplication.h"
#include "MumbleLink.h"
#include "InputHooks.h"
#include "MarkerEditor.h"

bool ShiftState = false;
bool HooksInitialized = false;
DWORD hookThreadID = 0;
bool disableHooks = false;
extern HWND gameWindow;
extern CWBApplication* App;
TBOOL keyboardHookActive = false;
TBOOL mouseHookActive = false;

LRESULT __stdcall MyKeyboardProc( int ccode, WPARAM wParam, LPARAM lParam )
{
  if ( disableHooks )
    return CallNextHookEx( 0, ccode, wParam, lParam );

  if ( ccode == HC_ACTION )
  {
    KBDLLHOOKSTRUCT* pkbdllhook = (KBDLLHOOKSTRUCT*)lParam;
    HKL dwhkl = 0;
    BYTE dbKbdState[ 256 ];
    TCHAR szCharBuf[ 32 ];
    static KBDLLHOOKSTRUCT lastState = { 0 };

    GetKeyboardState( dbKbdState );
    dwhkl = GetKeyboardLayout( GetWindowThreadProcessId( GetForegroundWindow(), NULL ) );

    if ( ToAsciiEx( pkbdllhook->vkCode, pkbdllhook->scanCode, dbKbdState, (LPWORD)szCharBuf, 0, dwhkl ) == -1 )
    {
      //PostMessage( (HWND)App->GetHandle(), WM_DEADCHAR, pkbdllhook->vkCode, 1 | ( pkbdllhook->scanCode << 16 ) + ( pkbdllhook->flags << 24 ) );

      //Save the current keyboard state.
      lastState = *pkbdllhook;
      //You might also need to hang onto the dbKbdState array... I'm thinking not.
      //Clear out the buffer to return to the previous state - wait for ToAsciiEx to return a value other than -1 by passing the same key again. It should happen after 1 call.
      while ( ToAsciiEx( pkbdllhook->vkCode, pkbdllhook->scanCode, dbKbdState, (LPWORD)szCharBuf, 0, dwhkl ) < 0 );
    }
    else
    {
      //for ( int x = 0; x < 32; x++ )
      //  if ( szCharBuf[ x ] )
      //  {
          //PostMessage( (HWND)App->GetHandle(), WM_CHAR, szCharBuf[ 0 ], 1 | ( pkbdllhook->scanCode << 16 ) + ( pkbdllhook->flags << 24 ) );
        //}
        //else break;

      //Do something with szCharBuf here since this will overwrite it...
      //If we have a saved vkCode from last call, it was a dead key we need to place back in the buffer.
      if ( lastState.vkCode != 0 )
      {
        //Safest to just clear this.
        memset( dbKbdState, 0, 256 );
        //Put the old vkCode back into the locale's buffer. 
        ToAsciiEx( lastState.vkCode, lastState.scanCode, dbKbdState, (LPWORD)szCharBuf, 0, dwhkl );
        //Set vkCode to 0, we can use this as a flag as a vkCode of 0 is invalid.
        lastState.vkCode = 0;
      }
    }
  }

  if ( ccode < 0 )
    return CallNextHookEx( 0, ccode, wParam, lParam );

  if ( wParam != WM_KEYDOWN && wParam != WM_KEYUP && wParam != WM_CHAR && wParam != WM_DEADCHAR && wParam != WM_UNICHAR )
    return CallNextHookEx( 0, ccode, wParam, lParam );

  auto wnd = GetForegroundWindow();
  if ( ccode != HC_ACTION || !lParam || ( wnd != gameWindow && App && wnd != (HWND)App->GetHandle() ) )
    return CallNextHookEx( 0, ccode, wParam, lParam );

  if ( App && wnd == (HWND)App->GetHandle() )
    return CallNextHookEx( 0, ccode, wParam, lParam );

  KBDLLHOOKSTRUCT* kbdat = (KBDLLHOOKSTRUCT*)lParam;
  PostMessage( (HWND)App->GetHandle(), wParam, kbdat->vkCode, 1 | ( kbdat->scanCode << 16 ) + ( kbdat->flags << 24 ) );

  return ( CallNextHookEx( 0, ccode, wParam, lParam ) );
}

LRESULT __stdcall KeyboardHook( int code, WPARAM wParam, LPARAM lParam )
{
  if ( disableHooks || mumbleLink.textboxHasFocus )
    return CallNextHookEx( 0, code, wParam, lParam );

  // !!!!!!!!!!!!! https://stackoverflow.com/questions/3548932/keyboard-hook-changes-the-behavior-of-keys

  if ( code < 0 )
    return CallNextHookEx( 0, code, wParam, lParam );

  if ( wParam != WM_KEYDOWN && wParam != WM_KEYUP && wParam != WM_CHAR && wParam != WM_DEADCHAR && wParam != WM_UNICHAR )
    return CallNextHookEx( 0, code, wParam, lParam );

  auto wnd = GetForegroundWindow();
  if ( code != HC_ACTION || !lParam || ( wnd != gameWindow && App && wnd != (HWND)App->GetHandle() ) )
    return CallNextHookEx( 0, code, wParam, lParam );

  if ( App && wnd == (HWND)App->GetHandle() )
    return CallNextHookEx( 0, code, wParam, lParam );

  KBDLLHOOKSTRUCT* kbdat = (KBDLLHOOKSTRUCT*)lParam;
  UINT mapped = MapVirtualKey( kbdat->vkCode, MAPVK_VK_TO_CHAR );

  //App->InjectMessage( wParam, kbdat->vkCode, 1 | ( kbdat->scanCode << 16 ) + ( kbdat->flags << 24 ) );

  bool inFocus = App->GetFocusItem() && App->GetFocusItem()->InstanceOf( "textbox" );

  //if ( !( mapped & ( 1 << 31 ) ) && !inFocus && wParam == WM_KEYDOWN )
  //  App->InjectMessage( WM_CHAR, mapped, 0 );

  if ( mapped & ( 1 << 31 ) && !inFocus )
    return CallNextHookEx( 0, 0, wParam, (LPARAM)lParam );

  if ( !inFocus )
  {
    if ( wParam == WM_KEYDOWN )
    {
      //(*(void(__thiscall**)(_DWORD*, signed int, UINT, _DWORD))(*app + 68))(app, 258, mapped, 0);
      App->InjectMessage( WM_CHAR, mapped, 0 );
      return CallNextHookEx( 0, 0, WM_KEYDOWN, (LPARAM)lParam );
    }
    return CallNextHookEx( 0, 0, wParam, (LPARAM)lParam );
  }

  PostMessage( (HWND)App->GetHandle(), wParam, kbdat->vkCode, 1 | ( kbdat->scanCode << 16 ) + ( kbdat->flags << 24 ) );
  return 1;

  //if ( inFocus )
  //  return 1;

  //FILE *f = fopen( "keylog.txt", "at" );
  //fprintf( f, "keyboard hook: %d %x %d %d %d %x\n", wParam, kbdat->flags, kbdat->scanCode, kbdat->time, kbdat->vkCode, kbdat->dwExtraInfo );
  //fclose( f );

 // if (wParam == WM_KEYDOWN /*|| wParam == WM_SYSKEYDOWN*/)
 // {
 // 	if (kbdat->vkCode == VK_SHIFT || kbdat->vkCode == VK_LSHIFT || kbdat->vkCode == VK_RSHIFT)
 // 		ShiftState = true;
 // 	WORD charcode;
 // 	unsigned char state[256];
 // 	GetKeyboardState(state);
 // 	state[0x10] = 0x80 * ShiftState;
 // 	int len = ToAscii(kbdat->vkCode, kbdat->scanCode, state, &charcode, 0);
 // 	if (len > 0)
 // 	{
 // 		PostMessage((HWND)App->GetHandle(), (wParam == WM_KEYDOWN) ? WM_CHAR : WM_SYSCHAR, charcode, 0);
 // 	}
 // }

 // if (wParam == WM_KEYUP /*|| wParam == WM_SYSKEYUP*/)
 // {
 // 	if (kbdat->vkCode == VK_SHIFT || kbdat->vkCode == VK_LSHIFT || kbdat->vkCode == VK_RSHIFT)
 // 		ShiftState = false;
 // }

 //// if (kbdat->vkCode != VK_MENU && kbdat->vkCode != VK_LMENU && kbdat->vkCode != VK_RMENU)
    //if ( ( wParam == WM_KEYUP || wParam == WM_KEYDOWN || wParam == WM_CHAR ) && App->GetFocusItem()->InstanceOf( "textbox" ) )
     // return 1;

  //return CallNextHookEx( 0, code, wParam, lParam );
}

LRESULT __stdcall MouseHook( int code, WPARAM wParam, LPARAM lParam )
{
  if ( disableHooks )
    return CallNextHookEx( 0, code, wParam, lParam );

  auto wnd = GetForegroundWindow();
  if ( code < 0 || !lParam || ( wnd != gameWindow && App && wnd != (HWND)App->GetHandle() ) )
    return CallNextHookEx( 0, code, wParam, lParam );

  MSLLHOOKSTRUCT* mousedat = (MSLLHOOKSTRUCT*)lParam;

  POINT ap = mousedat->pt;
  //SetCursorPos(ap.x, ap.y);
  ScreenToClient( (HWND)App->GetHandle(), &ap );

  if ( wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_LBUTTONDBLCLK )
  {
    PostMessage( (HWND)App->GetHandle(), wParam, 0, ap.x + ( ap.y << 16 ) );
    auto item = App->GetItemUnderMouse( CPoint( ap.x, ap.y ), wParam == WM_LBUTTONDOWN ? WBM_LEFTBUTTONDOWN : WBM_RIGHTBUTTONDOWN );
    auto markerEditor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( markerEditor && !markerEditor->IsHidden() )
    {
      if ( markerEditor->ShouldPassMouseEvent() )
        return CallNextHookEx( 0, code, wParam, lParam );
    }
    if ( item && item != App->GetRoot() && item->GetType() != "clickthroughbutton" )
      return 1;
  }

  if ( wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MOUSEMOVE )
  {
    PostMessage( (HWND)App->GetHandle(), wParam, 0, ap.x + ( ap.y << 16 ) );
    return CallNextHookEx( 0, code, wParam, lParam ); //let these through so we don't mess up dragging etc
  }

  return CallNextHookEx( 0, code, wParam, lParam );
}

void InitInputHooks()
{
  if ( HooksInitialized )
    return;

  if ( IsDebuggerPresent() )
    return;

  auto hookThread = CreateThread( NULL, 0,
                                  []( LPVOID data )
                                  {
                                    auto keyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, KeyboardHook, NULL, 0 );
                                    auto mouseHook = SetWindowsHookEx( WH_MOUSE_LL, MouseHook, NULL, 0 );
                                    MSG msg;

                                    keyboardHookActive = true;
                                    mouseHookActive = true;

                                    while ( GetMessage( &msg, 0, 0, 0 ) > 0 )
                                    {
                                      if ( msg.message == WM_QUIT )
                                        break;
                                      TranslateMessage( &msg );
                                    }
                                    DispatchMessage( &msg );

                                    UnhookWindowsHookEx( keyboardHook );
                                    UnhookWindowsHookEx( mouseHook );

                                    keyboardHookActive = false;
                                    mouseHookActive = false;

                                    return (DWORD)0;
                                  },
                                  0, 0, &hookThreadID );
  HooksInitialized = true;
}

void ShutDownInputHooks()
{
  if ( hookThreadID )
    PostThreadMessage( hookThreadID, WM_QUIT, 0, 0 );

  while ( keyboardHookActive );
  while ( mouseHookActive );
}

