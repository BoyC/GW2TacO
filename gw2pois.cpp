#include "OverlayApplication.h"
#include "ProFont.h"
#include <dwmapi.h>
#include "gw2tactical.h"
#include "TrailLogger.h"
#include "MapTimer.h"
#include "MouseHighlight.h"
#include "MumbleLink.h"
#include <process.h>
#include <windowsx.h>
#include "OverlayConfig.h"
#include "GW2TacO.h"
#include "OverlayWindow.h"
#include "resource.h"
#include "LocationalTimer.h"
#include "BuildCount.h"
#include "RangeDisplay.h"
#include "TacticalCompass.h"
#include "HPGrid.h"
#include "GW2API.h"
#include "SpecialGUIItems.h"
#include "Language.h"
#include <locale>
#include <codecvt>

#include "ThirdParty/BugSplat/inc/BugSplat.h"
#pragma comment(lib,"ThirdParty/BugSplat/lib/BugSplat.lib")

MiniDmpSender* bugSplat = nullptr;
CLoggerOutput_RingBuffer* ringbufferLog = nullptr;

#pragma comment(lib,"Dwmapi.lib")

CWBApplication* App = NULL;
HWND gw2Window;
HWND gw2WindowFromPid = nullptr;

bool disableHooks = false;

TBOOL InitGUI( CWBApplication* App )
{
  CreateUniFontOutlined( App, "UniFontOutlined" ); // default font
  CreateProFontOutlined( App, "ProFontOutlined" );
  CreateUniFont( App, "UniFont" );
  CreateProFont( App, "ProFont" );

  if ( !App->LoadSkinFromFile( _T( "UI.wbs" ), localization->GetUsedGlyphs() ) )
  {
    MessageBox( NULL, _T( "TacO can't find the UI.wbs ui skin file!\nPlease make sure you extracted all the files from the archive to a separate folder!" ), _T( "Missing File!" ), MB_ICONERROR );
    return false;
  }
  if ( !App->LoadXMLLayoutFromFile( _T( "UI.xml" ) ) )
  {
    MessageBox( NULL, _T( "TacO can't find the UI.xml ui layout file!\nPlease make sure you extracted all the files from the archive to a separate folder!" ), _T( "Missing File!" ), MB_ICONERROR );
    return false;
  }
  if ( !App->LoadCSSFromFile( UIFileNames[ GetConfigValue( "InterfaceSize" ) ] ) )
  {
    MessageBox( NULL, _T( "TacO can't find a required UI css style file!\nPlease make sure you extracted all the files from the archive to a separate folder!" ), _T( "Missing File!" ), MB_ICONERROR );
    return false;
  }
  App->RegisterUIFactoryCallback( "GW2TacticalDisplay", GW2TacticalDisplay::Factory );
  App->RegisterUIFactoryCallback( "GW2TrailDisplay", GW2TrailDisplay::Factory );
  App->RegisterUIFactoryCallback( "GW2MapTimer", GW2MapTimer::Factory );
  App->RegisterUIFactoryCallback( "MouseHighlight", GW2MouseHighlight::Factory );
  App->RegisterUIFactoryCallback( "GW2TacO", GW2TacO::Factory );
  App->RegisterUIFactoryCallback( "OverlayWindow", OverlayWindow::Factory );
  App->RegisterUIFactoryCallback( "TimerDisplay", TimerDisplay::Factory );
  App->RegisterUIFactoryCallback( "GW2TacticalCompass", GW2TacticalCompass::Factory );
  App->RegisterUIFactoryCallback( "GW2RangeDisplay", GW2RangeDisplay::Factory );
  App->RegisterUIFactoryCallback( "HPGrid", GW2HPGrid::Factory );
  App->RegisterUIFactoryCallback( "clickthroughbutton", ClickThroughButton::Factory );

  App->GenerateGUI( App->GetRoot(), _T( "gw2pois" ) );

  App->ReApplyStyle();

  return true;
}

void OpenWindows( CWBApplication* App )
{
  auto root = App->GetRoot();
  GW2TacO* taco = (GW2TacO*)root->FindChildByID( "tacoroot", "GW2TacO" );
  if ( !taco )
    return;

  if ( HasWindowData( "MapTimer" ) && IsWindowOpen( "MapTimer" ) )
    taco->OpenWindow( "MapTimer" );

  if ( HasWindowData( "TS3Control" ) && IsWindowOpen( "TS3Control" ) )
    taco->OpenWindow( "TS3Control" );

  if ( HasWindowData( "MarkerEditor" ) && IsWindowOpen( "MarkerEditor" ) )
    taco->OpenWindow( "MarkerEditor" );

  if ( HasWindowData( "Notepad" ) && IsWindowOpen( "Notepad" ) )
    taco->OpenWindow( "Notepad" );

  if ( HasWindowData( "RaidProgress" ) && IsWindowOpen( "RaidProgress" ) )
    taco->OpenWindow( "RaidProgress" );

  if ( HasWindowData( "DungeonProgress" ) && IsWindowOpen( "DungeonProgress" ) )
    taco->OpenWindow( "DungeonProgress" );

  if ( HasWindowData( "TPTracker" ) && IsWindowOpen( "TPTracker" ) )
    taco->OpenWindow( "TPTracker" );
}

bool ShiftState = false;

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
  if ( ccode != HC_ACTION || !lParam || ( wnd != gw2Window && App && wnd != (HWND)App->GetHandle() ) )
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
  if ( code != HC_ACTION || !lParam || ( wnd != gw2Window && App && wnd != (HWND)App->GetHandle() ) )
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
  if ( code < 0 || !lParam || ( wnd != gw2Window && App && wnd != (HWND)App->GetHandle() ) )
    return CallNextHookEx( 0, code, wParam, lParam );

  MSLLHOOKSTRUCT* mousedat = (MSLLHOOKSTRUCT*)lParam;

  POINT ap = mousedat->pt;
  //SetCursorPos(ap.x, ap.y);
  ScreenToClient( (HWND)App->GetHandle(), &ap );

  if ( wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN )
  {
    PostMessage( (HWND)App->GetHandle(), wParam, 0, ap.x + ( ap.y << 16 ) );
    if ( App->GetMouseItem() && App->GetMouseItem() != App->GetRoot() && App->GetMouseItem()->GetType() != "clickthroughbutton" )
      return 1;
  }

  if ( wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP || wParam == WM_MOUSEMOVE )
  {
    PostMessage( (HWND)App->GetHandle(), wParam, 0, ap.x + ( ap.y << 16 ) );
    return CallNextHookEx( 0, code, wParam, lParam ); //let these through so we don't mess up dragging etc
  }

  return CallNextHookEx( 0, code, wParam, lParam );
}

LONG WINAPI CrashOverride( struct _EXCEPTION_POINTERS* excpInfo )
{
  if ( IsDebuggerPresent() ) return EXCEPTION_CONTINUE_SEARCH;

#ifndef _DEBUG
  bool bs = !HasConfigValue( "SendCrashDump" ) || GetConfigValue( "SendCrashDump" );
  if ( bugSplat && bs )
  {
    if ( ringbufferLog )
    {
      ringbufferLog->Dump( "CrashLog.log" );
      bugSplat->sendAdditionalFile( L"CrashLog.log" );
    }
    bugSplat->unhandledExceptionHandler( excpInfo );
    MessageBox( NULL, _T( "TacO has crashed :(\nCrash has been reported." ), _T( "Crash" ), MB_ICONERROR );
    return EXCEPTION_EXECUTE_HANDLER;
  }
  else
#endif
  {
    LONG res = baseCrashTracker( excpInfo );// FullDumpCrashTracker( excpInfo );// baseCrashTracker( excpInfo );
    return res;
  }
}

#include <TlHelp32.h>

DWORD GetProcessIntegrityLevel( HANDLE hProcess )
{
  HANDLE hToken;

  DWORD dwLengthNeeded;
  DWORD dwError = ERROR_SUCCESS;

  PTOKEN_MANDATORY_LABEL pTIL = NULL;
  DWORD dwIntegrityLevel = 0;

  //hProcess = GetCurrentProcess();
  if ( OpenProcessToken( hProcess, TOKEN_QUERY, &hToken ) )
  {
    // Get the Integrity level.
    if ( !GetTokenInformation( hToken, TokenIntegrityLevel,
                               NULL, 0, &dwLengthNeeded ) )
    {
      dwError = GetLastError();
      if ( dwError == ERROR_INSUFFICIENT_BUFFER )
      {
        pTIL = (PTOKEN_MANDATORY_LABEL)LocalAlloc( 0,
                                                   dwLengthNeeded );
        if ( pTIL != NULL )
        {
          if ( GetTokenInformation( hToken, TokenIntegrityLevel,
                                    pTIL, dwLengthNeeded, &dwLengthNeeded ) )
          {
            dwIntegrityLevel = *GetSidSubAuthority( pTIL->Label.Sid,
                                                    (DWORD)(UCHAR)( *GetSidSubAuthorityCount( pTIL->Label.Sid ) - 1 ) );

                                                  //if (dwIntegrityLevel == SECURITY_MANDATORY_LOW_RID)
                                                  //else if (dwIntegrityLevel >= SECURITY_MANDATORY_MEDIUM_RID &&
                                                  //		 dwIntegrityLevel < SECURITY_MANDATORY_HIGH_RID)
                                                  //else if (dwIntegrityLevel >= SECURITY_MANDATORY_HIGH_RID)
                                                  //else if (dwIntegrityLevel >= SECURITY_MANDATORY_SYSTEM_RID)
          }
          LocalFree( pTIL );
        }
      }
    }
    else
      return -1;
    CloseHandle( hToken );
  }
  else
    return -1;

  return dwIntegrityLevel;
}

bool IsProcessRunning( DWORD pid )
{
  bool procRunning = false;

  HANDLE hProcessSnap;
  PROCESSENTRY32 pe32;
  hProcessSnap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

  if ( hProcessSnap == INVALID_HANDLE_VALUE )
    return false;

  pe32.dwSize = sizeof( PROCESSENTRY32 );
  if ( Process32First( hProcessSnap, &pe32 ) )
  {
    if ( pe32.th32ProcessID == pid )
    {
      CloseHandle( hProcessSnap );
      return true;
    }

    while ( Process32Next( hProcessSnap, &pe32 ) )
    {
      if ( pe32.th32ProcessID == pid )
      {
        CloseHandle( hProcessSnap );
        return true;
      }
    }
    CloseHandle( hProcessSnap );
  }

  return procRunning;
}

#include <winhttp.h>
#pragma comment(lib,"winhttp.lib")

CString FetchHTTP( LPCWSTR url, LPCWSTR path )
{
  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;
  LPSTR pszOutBuffer;

  BOOL  bResults = FALSE;
  HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;

  hSession = WinHttpOpen( L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

  if ( hSession )
    hConnect = WinHttpConnect( hSession, url, INTERNET_DEFAULT_HTTP_PORT, 0 );

  if ( hConnect )
    hRequest = WinHttpOpenRequest( hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, NULL, NULL );

  if ( hRequest )
    bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );

  if ( bResults )
    bResults = WinHttpReceiveResponse( hRequest, NULL );

  if ( !bResults )
  {
    if ( hRequest ) WinHttpCloseHandle( hRequest );
    if ( hConnect ) WinHttpCloseHandle( hConnect );
    if ( hSession ) WinHttpCloseHandle( hSession );
    return "";
  }

  pszOutBuffer = nullptr;

  CStreamWriterMemory data;

  do
  {
    dwSize = 0;
    if ( !WinHttpQueryDataAvailable( hRequest, &dwSize ) )
    {
      if ( hRequest ) WinHttpCloseHandle( hRequest );
      if ( hConnect ) WinHttpCloseHandle( hConnect );
      if ( hSession ) WinHttpCloseHandle( hSession );
      return "";
    }

    pszOutBuffer = new char[ dwSize + 1 ];

    if ( !WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded ) )
    {
      if ( hRequest ) WinHttpCloseHandle( hRequest );
      if ( hConnect ) WinHttpCloseHandle( hConnect );
      if ( hSession ) WinHttpCloseHandle( hSession );
      return "";
    }

    data.Write( pszOutBuffer, dwSize );

    SAFEDELETEA( pszOutBuffer );
  } while ( dwSize > 0 );


  if ( hRequest ) WinHttpCloseHandle( hRequest );
  if ( hConnect ) WinHttpCloseHandle( hConnect );
  if ( hSession ) WinHttpCloseHandle( hSession );

  return CString( (TS8*)data.GetData(), data.GetLength() );
}

CString FetchHTTPS( LPCWSTR url, LPCWSTR path )
{
  CString s1 = CString( url );
  CString s2 = CString( path );

  LOG_NFO( "[GW2TacO] Fetching URL: %s/%s", s1.GetPointer(), s2.GetPointer() );

  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;
  LPSTR pszOutBuffer;

  BOOL  bResults = FALSE;
  HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;

  hSession = WinHttpOpen( L"WinHTTPS Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

  if ( hSession )
    hConnect = WinHttpConnect( hSession, url, INTERNET_DEFAULT_PORT, 0 );

  if ( hConnect )
    hRequest = WinHttpOpenRequest( hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, NULL, WINHTTP_FLAG_SECURE );

  if ( hRequest )
    bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );

  if ( bResults )
    bResults = WinHttpReceiveResponse( hRequest, NULL );

  if ( !bResults )
    return "";

  pszOutBuffer = nullptr;

  CStreamWriterMemory data;

  do
  {
    dwSize = 0;
    if ( !WinHttpQueryDataAvailable( hRequest, &dwSize ) )
      return "";

    pszOutBuffer = new char[ dwSize + 1 ];

    if ( !WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded ) )
      return "";

    data.Write( pszOutBuffer, dwSize );

    SAFEDELETEA( pszOutBuffer );
  } while ( dwSize > 0 );


  if ( hRequest ) WinHttpCloseHandle( hRequest );
  if ( hConnect ) WinHttpCloseHandle( hConnect );
  if ( hSession ) WinHttpCloseHandle( hSession );

  return CString( (TS8*)data.GetData(), data.GetLength() );
}

#include <Urlmon.h>   // URLOpenBlockingStreamW()
#pragma comment( lib, "Urlmon.lib" )

bool HooksInitialized = false;
TBOOL IsTacOUptoDate = true;
int NewTacOVersion = RELEASECOUNT;

volatile int mainLoopCounter = 0;
volatile int lastCnt = 0;
int lastMainLoopTime = 0;
#include <thread>

#include <ShellScalingAPI.h>
//#pragma comment(lib,"Shcore.lib")

#include "Bedrock/UtilLib/jsonxx.h"

using namespace jsonxx;

#include "WvW.h"

#include <tlhelp32.h>

void GetFileName( CHAR pfname[ MAX_PATH ] )
{
  DWORD dwOwnPID = GetProcessId( GetCurrentProcess() );

  HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  PROCESSENTRY32* processInfo = new PROCESSENTRY32;
  processInfo->dwSize = sizeof( PROCESSENTRY32 );
  while ( Process32Next( hSnapShot, processInfo ) != FALSE )
  {
    if ( processInfo->th32ProcessID == dwOwnPID )
    {
      memcpy( pfname, processInfo->szExeFile, MAX_PATH );
      break;
    }
  }
  CloseHandle( hSnapShot );
  delete processInfo;
}

BOOL AppIsAllreadyRunning()
{
  CHAR pfname[ MAX_PATH ];
  memset( pfname, 0, MAX_PATH );
  GetFileName( pfname );

  BOOL bRunning = FALSE;

  DWORD dwOwnPID = GetProcessId( GetCurrentProcess() );

  HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  PROCESSENTRY32* processInfo = new PROCESSENTRY32;
  processInfo->dwSize = sizeof( PROCESSENTRY32 );
  int index = 0;
  while ( Process32Next( hSnapShot, processInfo ) != FALSE )
  {
    if ( !strcmp( processInfo->szExeFile, pfname ) )
    {
      if ( processInfo->th32ProcessID != dwOwnPID )
      {
        bRunning = TRUE;
        break;
      }
    }
  }
  CloseHandle( hSnapShot );
  delete processInfo;
  return bRunning;
}

TBOOL keyboardHookActive = false;
TBOOL mouseHookActive = false;

bool SetupTacoProtocolHandling()
{
  TCHAR szFileName[ MAX_PATH + 1 ];
  GetModuleFileName( NULL, szFileName, MAX_PATH + 1 );

  HKEY key;

  if ( RegCreateKeyEx( HKEY_CLASSES_ROOT, "gw2taco", 0, nullptr, REG_OPTION_NON_VOLATILE, 0, nullptr, &key, nullptr ) != ERROR_SUCCESS )
  {
    if ( RegOpenKey( HKEY_CLASSES_ROOT, "gw2taco", &key ) != ERROR_SUCCESS )
      return false;
  }
  else
  {
    RegCloseKey( key );
    if ( RegOpenKey( HKEY_CLASSES_ROOT, "gw2taco", &key ) != ERROR_SUCCESS )
      return false;
  }

  const char* urldesc = "URL:gw2taco protocol";

  if ( RegSetKeyValue( key, nullptr, nullptr, REG_SZ, urldesc, strlen( urldesc ) ) != ERROR_SUCCESS )
    return false;

  if ( RegSetKeyValue( key, nullptr, "URL Protocol", REG_SZ, nullptr, 0 ) )
    return false;

  if ( RegSetKeyValue( key, "DefaultIcon", nullptr, REG_SZ, szFileName, strlen( szFileName ) ) != ERROR_SUCCESS )
    return false;

  CString openMask = CString( "\"" ) + CString( szFileName ) + CString( "\" -fromurl %1" );

  if ( RegSetKeyValue( key, "shell\\open\\command", nullptr, REG_SZ, openMask.GetPointer(), openMask.Length() ) != ERROR_SUCCESS )
    return false;

  RegCloseKey( key );
  return true;
}

bool DownloadFile( const CString& url, CStreamWriterMemory& mem )
{
  LPSTREAM stream;

  HRESULT hr = URLOpenBlockingStream( nullptr, ( CString( "https://" ) + url ).GetPointer(), &stream, 0, nullptr );
  if ( FAILED( hr ) )
    hr = URLOpenBlockingStream( nullptr, ( CString( "http://" ) + url ).GetPointer(), &stream, 0, nullptr );

  if ( FAILED( hr ) )
    return false;

  char buffer[ 4096 ];
  do
  {
    DWORD bytesRead = 0;
    hr = stream->Read( buffer, sizeof( buffer ), &bytesRead );
    mem.Write( buffer, bytesRead );
  } while ( SUCCEEDED( hr ) && hr != S_FALSE );

  stream->Release();

  if ( FAILED( hr ) )
    return false;

  return true;
}

#define MINIZ_HEADER_FILE_ONLY
#include "Bedrock/UtilLib/miniz.c"

CArrayThreadSafe< CString > loadList;

void FetchMarkerPackOnline( CString& ourl )
{
  TS32 pos = ourl.Find( "gw2taco://markerpack/" );
  if ( pos < 0 )
  {
    LOG_ERR( "[GW2TacO] Trying to access malformed package url %s", ourl.GetPointer() );
    return;
  }

  LOG_NFO( "[GW2TacO] Trying to fetch marker pack %s", ourl.Substring( pos ).GetPointer() );

  CString url = ourl.Substring( pos + 21 );
  TU8* urlPtr = new TU8[ url.Length() + 1 ];
  memset( urlPtr, 0, url.Length() + 1 );
  memcpy( urlPtr, url.GetPointer(), url.Length() );

  DWORD downloadThreadID = 0;

  auto downloadThread = CreateThread( NULL, 0, []( LPVOID data )
                                      {
                                        CString url( (TS8*)data );
                                        delete[] data;

                                        CStreamWriterMemory mem;
                                        if ( !DownloadFile( url, mem ) )
                                        {
                                          LOG_ERR( "[GW2TacO] Failed to download package %s", url.GetPointer() );
                                          return (DWORD)0;
                                        }

                                        mz_zip_archive zip;
                                        memset( &zip, 0, sizeof( zip ) );
                                        if ( !mz_zip_reader_init_mem( &zip, mem.GetData(), mem.GetLength(), 0 ) )
                                        {
                                          LOG_ERR( "[GW2TacO] Package %s doesn't seem to be a well formed zip file", url.GetPointer() );
                                          return (DWORD)0;
                                        }

                                        mz_zip_reader_end( &zip );

                                        TS32 cnt = 0;
                                        for ( TU32 x = 0; x < url.Length(); x++ )
                                          if ( url[ x ] == '\\' || url[ x ] == '/' )
                                            cnt = x;

                                        CString fileName = url.Substring( cnt + 1 );
                                        if ( !fileName.Length() )
                                        {
                                          LOG_ERR( "[GW2TacO] Package %s has a malformed name", url.GetPointer() );
                                          return (DWORD)0;
                                        }

                                        if ( fileName.Find( ".zip" ) == fileName.Length() - 4 )
                                          fileName = fileName.Substring( 0, fileName.Length() - 4 );

                                        if ( fileName.Find( ".taco" ) == fileName.Length() - 5 )
                                          fileName = fileName.Substring( 0, fileName.Length() - 5 );

                                        for ( TU32 x = 0; x < fileName.Length(); x++ )
                                          if ( !isalnum( fileName[ x ] ) )
                                            fileName[ x ] = '_';

                                        fileName = CString( "POIs/" ) + fileName + ".taco";

                                        CStreamWriterFile out;
                                        if ( !out.Open( fileName.GetPointer() ) )
                                        {
                                          LOG_ERR( "[GW2TacO] Failed to open file for writing: %s", fileName.GetPointer() );
                                          return (DWORD)0;
                                        }

                                        if ( !out.Write( mem.GetData(), mem.GetLength() ) )
                                        {
                                          LOG_ERR( "[GW2TacO] Failed to write out data to file: %s", fileName.GetPointer() );
                                          remove( fileName.GetPointer() );
                                          return (DWORD)0;
                                        }

                                        loadList.Add( fileName );

                                        return (DWORD)0;
                                      }, urlPtr, 0, &downloadThreadID );
}

void ImportMarkerPack( CWBApplication* App, const CString& zipFile );

#include <imm.h>
#pragma comment(lib,"Imm32.lib")

void FlushZipDict();

TU32 lastSlowEventTime = 0;
int gw2WindowCount = 0;

BOOL __stdcall gw2WindowCountFunc( HWND   hwnd, LPARAM lParam )
{
  TCHAR name[ 400 ];
  memset( name, 0, 400 );
  GetWindowText( hwnd, name, 199 );
  if ( !strcmp( name, "Guild Wars 2" ) )
  {
    memset( name, 0, 400 );
    GetClassName( hwnd, name, 199 );
    if ( !strcmp( name, "ArenaNet_Dx_Window_Class" ) || !strcmp( name, "ArenaNet_Gr_Window_Class" ) )
    {
      gw2WindowCount++;
    }
  }
  return true;
}

BOOL __stdcall gw2WindowFromPIDFunction( HWND hWnd, LPARAM a2 )
{
  DWORD dwProcessId; // [esp+4h] [ebp-198h]
  CHAR ClassName[ 400 ]; // [esp+8h] [ebp-194h]

  memset( &ClassName, 0, 400 );
  GetClassNameA( hWnd, ClassName, 199 );
  if ( !strcmp( ClassName, "ArenaNet_Dx_Window_Class" ) || !strcmp( ClassName, "ArenaNet_Gr_Window_Class" ) )
  {
    dwProcessId = 0;
    GetWindowThreadProcessId( hWnd, &dwProcessId );
    if ( a2 == dwProcessId )
      gw2WindowFromPid = hWnd;
  }
  return 1;
}

INT WINAPI WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow )
{
  ImmDisableIME( -1 );
  lastSlowEventTime = globalTimer.GetTime();

  // init crash tracking

  extern CString TacOBuild;
  InitializeCrashTracker( CString( "GW2 TacO " ) + TacOBuild, CrashOverride );
  FORCEDDEBUGLOG( "Crash tracker initialized." );

  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wide = converter.from_bytes( TacOBuild.GetPointer() );

  bugSplat = new MiniDmpSender( L"GW2TacO", L"GW2TacO", wide.data(), NULL, MDSF_NONINTERACTIVE | MDSF_USEGUARDMEMORY | MDSF_LOGFILE | MDSF_PREVENTHIJACKING | MDSF_CUSTOMEXCEPTIONFILTER );
  SetGlobalCRTExceptionBehavior();
  SetPerThreadCRTExceptionBehavior();  // This call needed in each thread of your app
  bugSplat->setGuardByteBufferSize( 20 * 1024 * 1024 );

  Logger.AddOutput( new CLoggerOutput_File( _T( "GW2TacO.log" ) ) );
  ringbufferLog = new CLoggerOutput_RingBuffer();
  Logger.AddOutput( ringbufferLog );
  Logger.SetVerbosity( LOG_DEBUG );
  FORCEDDEBUGLOG( "Logger set up." );

  Logger.Log( LOG_INFO, false, false, "" );
  Logger.Log( LOG_INFO, false, false, "----------------------------------------------" );
  CString cmdLine( GetCommandLineA() );
  LOG_NFO( "[GW2TacO] CommandLine: %s", cmdLine.GetPointer() );

  if ( cmdLine.Find( "-fromurl" ) >= 0 )
  {
    TCHAR szFileName[ MAX_PATH + 1 ];
    GetModuleFileName( NULL, szFileName, MAX_PATH + 1 );
    CString s( szFileName );
    for ( TS32 x = s.Length() - 1; x >= 0; x-- )
      if ( s[ x ] == '\\' || s[ x ] == '/' )
      {
        s[ x ] = 0;
        break;
      }
    SetCurrentDirectory( s.GetPointer() );

    auto TacoWindow = FindWindow( "CoRE2", "Guild Wars 2 Tactical Overlay" );
    LOG_NFO( "[GW2TacO] TacO window id: %d", TacoWindow );
    if ( TacoWindow )
    {
      COPYDATASTRUCT MyCDS;
      MyCDS.dwData = 0;
      MyCDS.cbData = cmdLine.Length();
      MyCDS.lpData = cmdLine.GetPointer();

      SendMessage( TacoWindow,
                   WM_COPYDATA,
                   (WPARAM)( HWND )nullptr,
                   (LPARAM)(LPVOID)&MyCDS );

      LOG_NFO( "[GW2TacO] WM_COPYDATA sent. Result code: %d", GetLastError() );
      return 0;
    }

    FetchMarkerPackOnline( cmdLine );
  }

  if ( cmdLine.Find( "-forcenewinstance" ) < 0 )
  {
    if ( AppIsAllreadyRunning() )
      return 0;
  }

  auto mumblePos = cmdLine.Find( "-mumble" );
  if ( mumblePos >= 0 )
  {
    auto sub = cmdLine.Substring( mumblePos );
    auto cmds = sub.ExplodeByWhiteSpace();
    if ( cmds.NumItems() > 1 )
      mumbleLink.mumblePath = cmds[ 1 ];
  }

  //SetProcessDpiAwareness( PROCESS_PER_MONITOR_DPI_AWARE );

  /*
    std::thread t([]()
    {
      while ( 1 )
      {
        lastCnt = mainLoopCounter;
        Sleep( 2000 );
        if ( mainLoopCounter > 0 && lastCnt == mainLoopCounter )
        {
          int x = GetTime();
          int y = sin( 0 );
          mainLoopCounter = x / y;
        }
      }
    } );
  */

  FORCEDOPENDEBUGFILE();
  FORCEDDEBUGLOG( "Winmain started." );

  if ( !SetupTacoProtocolHandling() )
    LOG_ERR( "[GW2TacO] Failed to register gw2taco:// protocol with windows." );


  //Logger.AddOutput(new CLoggerOutput_StdOut());

  typedef HRESULT( WINAPI* SetProcessDpiAwareness )( _In_ PROCESS_DPI_AWARENESS value );
  typedef BOOL( *SetProcessDPIAwareFunc )( );

  LoadConfig();

  if ( cmdLine.Find( "-forcedpiaware" ) >= 0 || ( HasConfigValue( "ForceDPIAware" ) && GetConfigValue( "ForceDPIAware" ) ) )
  {
    bool dpiSet = false;

    HMODULE hShCore = LoadLibrary( _T( "Shcore.dll" ) );
    if ( hShCore )
    {
      SetProcessDpiAwareness setDPIAwareness = (SetProcessDpiAwareness)GetProcAddress( hShCore, "SetProcessDpiAwareness" );
      if ( setDPIAwareness )
      {
        setDPIAwareness( PROCESS_PER_MONITOR_DPI_AWARE );
        dpiSet = true;
        LOG_NFO( "[GW2TacO] DPI Awareness set through SetProcessDpiAwareness" );
      }
      FreeLibrary( hShCore );
    }

    if ( !dpiSet )
    {
      HMODULE hUser32 = LoadLibrary( _T( "user32.dll" ) );
      SetProcessDPIAwareFunc setDPIAware = (SetProcessDPIAwareFunc)GetProcAddress( hUser32, "SetProcessDPIAware" );
      if ( setDPIAware )
      {
        setDPIAware();
        LOG_NFO( "[GW2TacO] DPI Awareness set through SetProcessDpiAware" );
        dpiSet = true;
      }
      FreeLibrary( hUser32 );
    }

    if ( !dpiSet )
      LOG_ERR( "[GW2TacO] DPI Awareness NOT set" );
  }

  FORCEDDEBUGLOG( "Config Loaded." );

  localization = new Localization();
  localization->Import();

  LOG_NFO( "[GW2TacO] build ID: %s", ( CString( "GW2 TacO " ) + TacOBuild ).GetPointer() );

  bool hasDComp = 0;
  HMODULE dComp = LoadLibraryA( "dcomp.dll" );
  if ( dComp )
  {
    hasDComp = 1;
    FreeLibrary( dComp );
  }

  App = new COverlayApp();

  FORCEDDEBUGLOG( "App instance created." );

  int width = 1;
  int height = 1;

  CCoreWindowParameters p = CCoreWindowParameters( GetModuleHandle( NULL ), false, width, height, _T( "Guild Wars 2 Tactical Overlay" ), LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON2 ) ) );
  p.OverrideWindowStyle = WS_POPUP;
  p.OverrideWindowStyleEx = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW;// | WS_EX_TOPMOST;// | WS_EX_TOOLWINDOW;
  if ( dComp )
    p.OverrideWindowStyleEx = WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOREDIRECTIONBITMAP;

  FORCEDDEBUGLOG( "About to initialize the app." );
  if ( !App->Initialize( p ) )
  {
    MessageBox( NULL, "Failed to initialize GW2 TacO!\nIf you want this error fixed,\nplease send the generated GW2TacO.log and a dxdiag log to boyc@scene.hu", "Fail.", MB_ICONERROR );
    SAFEDELETE( App );
    return false;
  }

  if ( !ChangeWindowMessageFilterEx( (HWND)App->GetHandle(), WM_COPYDATA, MSGFLT_ALLOW, NULL ) )
    LOG_ERR( "[GW2TacO] Failed to change message filters for WM_COPYDATA - gw2taco:// protocol messages will NOT be processed!" );

  FORCEDDEBUGLOG( "App initialized." );

  DWORD hookThreadID = 0;

  App->SetScreenshotName( _T( "GW2TacO" ) );
  App->SetClearColor( CColor( 0, 0, 0, 0 ) );

  FORCEDDEBUGLOG( "screenshot name and clear color set" );

  ImportPOIS( App );
  FORCEDDEBUGLOG( "markers imported" );

  mumbleLink.Update();

  if ( !InitGUI( App ) )
  {
    LOG_ERR( "[GW2TacO] Missing file during init, exiting!" );
    return -1;
  }
  FORCEDDEBUGLOG( "gui initialized" );


  extern WBATLASHANDLE DefaultIconHandle;
  if ( DefaultIconHandle == -1 )
  {
    auto skinItem = App->GetSkin()->GetElementID( CString( "defaulticon" ) );
    DefaultIconHandle = App->GetSkin()->GetElement( skinItem )->GetHandle();
  }

  ImportPOIActivationData();
  FORCEDDEBUGLOG( "activation data imported" );
  ImportLocationalTimers();
  FORCEDDEBUGLOG( "locational timers imported" );

  LoadWvWObjectives();

  OpenWindows( App );

  FORCEDDEBUGLOG( "taco windows opened" );

  HWND handle = (HWND)App->GetHandle();

  if ( !HasConfigValue( "CheckForUpdates" ) )
    SetConfigValue( "CheckForUpdates", 1 );

  if ( !HasConfigValue( "HideOnLoadingScreens" ) )
    SetConfigValue( "HideOnLoadingScreens", 1 );

  if ( !HasConfigValue( "KeybindsEnabled" ) )
    SetConfigValue( "KeybindsEnabled", 1 );

  if ( !HasConfigValue( "EnableCrashMenu" ) )
    SetConfigValue( "EnableCrashMenu", 0 );

  if ( !HasConfigValue( "SendCrashDump" ) )
    SetConfigValue( "SendCrashDump", 1 );

  SetConfigValue( "LogTrails", 0 );

  //CString apidata = FetchHTTP( L"api.guildwars2.com", L"/v2/continents?ids=all" );

  if ( GetConfigValue( _T( "CheckForUpdates" ) ) )
  {
    DWORD UpdateCheckThreadID = 0;
    auto hookThread = CreateThread(
      NULL,                   // default security attributes
      0,                      // use default stack size  
      []( LPVOID data )
      {
        CString s = FetchHTTP( L"www.gw2taco.com", L"/2000/01/buildid.html" );
        TS32 idpos = s.Find( "[buildid:" );
        if ( idpos >= 0 )
        {
          CString sub = s.Substring( idpos );
          TS32 release = 0;
          TS32 build = 0;
          if ( sub.Scan( "[buildid:%d.%dr]", &release, &build ) == 2 )
          {
            extern TS32 TacORelease;
            extern TS32 TacOBuildCount;
            if ( release > TacORelease || build > TacOBuildCount )
            {
              NewTacOVersion = release;
              IsTacOUptoDate = false;
            }
          }
        }

        return (DWORD)0;
      },
      0,          // argument to thread function 
        0,                      // use default creation flags 
        &UpdateCheckThreadID );

  }

  //MARGINS marg;

  //marg.cxLeftWidth = 0;
  //marg.cyTopHeight = 0;
  //marg.cxRightWidth = width;
  //marg.cyBottomHeight = height;
  //DwmExtendFrameIntoClientArea(handle, &marg);

  //RECT WindowRect;
  //GetWindowRect(handle, &WindowRect);
  //WindowRect.right = WindowRect.left + width;
  //WindowRect.bottom = WindowRect.top + height;
  //AdjustWindowRect(&WindowRect, CS_HREDRAW | CS_VREDRAW, FALSE);
  //SetWindowPos(handle, 0, WindowRect.top, WindowRect.left, width, height, 0);

  //SetWindowLong(handle, GWL_STYLE, CS_HREDRAW | CS_VREDRAW);

  //SetWindowLong(handle, GWL_EXSTYLE, WS_EX_COMPOSITED);
  //SetLayeredWindowAttributes(handle, RGB(0, 0, 0), 0, ULW_COLORKEY);
  SetLayeredWindowAttributes( handle, 0, 255, LWA_ALPHA );

  ShowWindow( handle, nCmdShow );

  FORCEDDEBUGLOG( "set some window attributes" );

  TBOOL FoundGW2Window = false;

  if ( !HasConfigValue( "Vsync" ) )
    SetConfigValue( "Vsync", 1 );

  if ( !HasConfigValue( "SmoothCharacterPos" ) )
    SetConfigValue( "SmoothCharacterPos", 1 );

  App->SetVSync( GetConfigValue( "Vsync" ) );
  CRect pos;

  DWORD GW2Pid = 0;

  FORCEDDEBUGLOG( "starting main loop" );

  GW2TacO* taco = (GW2TacO*)App->GetRoot()->FindChildByID( "tacoroot", "GW2TacO" );

  if ( !taco )
    return 0;

  taco->InitScriptEngines();

  auto lastRenderTime = globalTimer.GetTime();
  if ( !HasConfigValue( "FrameThrottling" ) )
    SetConfigValue( "FrameThrottling", 1 );

  bool frameThrottling = GetConfigValue( "FrameThrottling" ) != 0;

  TS32 hideOnLoadingScreens = GetConfigValue( "HideOnLoadingScreens" );

  bool hadRetrace = false;

  while ( App->HandleMessages() )
  {
#ifdef _DEBUG
    if ( GetAsyncKeyState( VK_F11 ) )
      break;
#endif

    if ( globalTimer.GetTime() - lastSlowEventTime > 1000 )
    {
      if ( GetConfigValue( "CloseWithGW2" ) )
        if ( !gw2Window && FoundGW2Window )
          if ( !IsProcessRunning( GW2Pid ) )
            App->SetDone( true );
    }

    if ( loadList.NumItems() )
    {
      CString file = loadList[ 0 ];
      loadList.DeleteByIndex( 0 );
      ImportMarkerPack( App, file );
      //FlushZipDict();
    }

    FORCEDDEBUGLOG( "messages handled" );

    //WaitForSingleObject( mumbleFrameTrigger, 1000 );

    for ( int x = 0; x < ( frameThrottling ? 32 : 1 ); x++ )
    {
      if ( mumbleLink.Update() )
        break;
      if ( frameThrottling )
        Sleep( 1 );
    }

    bool shortTick = ( GetTime() - mumbleLink.LastFrameTime ) < 333;

    if ( !hideOnLoadingScreens )
      shortTick = true;

    if ( !FoundGW2Window )
    {
      //if (mumbleLink.mumblePath != "MumbleLink")
      {
        if ( !mumbleLink.IsValid() && GetTime() > 60000 )
        {
          LOG_ERR( "[GW2TacO] Closing TacO because GW2 with mumble link '%s' was not found in under a minute", mumbleLink.mumblePath.GetPointer() );
          App->SetDone( true );
        }
      }
    }

    if ( App->DeviceOK() )
    {
      FORCEDDEBUGLOG( "device is ok, drawing" );
      extern bool frameTriggered;

      auto currTime = globalTimer.GetTime();

      if ( currTime - lastSlowEventTime > 1000 )
      {
        hideOnLoadingScreens = GetConfigValue( "HideOnLoadingScreens" );
        lastSlowEventTime = globalTimer.GetTime();
        gw2WindowCount = 0;
        gw2Window = nullptr;
        gw2WindowFromPid = nullptr;
        EnumWindows( gw2WindowFromPIDFunction, mumbleLink.lastGW2ProcessID );
        gw2Window = gw2WindowFromPid;

/*
        if (!gw2Window)
          gw2Window = FindWindow("ArenaNet_Dx_Window_Class", nullptr);

        if ( !gw2Window )
          gw2Window = FindWindow( "ArenaNet_Gr_Window_Class", nullptr );
*/
      }

      if ( !mumbleLink.IsValid() || !gw2Window )
      {
        Sleep( 1000 );
        continue;
      }

      //if ( !frameThrottling || frameTriggered || lastRenderTime + 200 < currTime )
      {
        if ( gw2Window )
        {
          if ( !FoundGW2Window )
          {
            FORCEDDEBUGLOG( "GW2 window found for the first time" );
            GetWindowThreadProcessId( gw2Window, &GW2Pid );

            DWORD currentProcessIntegrity = GetProcessIntegrityLevel( GetCurrentProcess() );
            DWORD gw2ProcessIntegrity = GetProcessIntegrityLevel( OpenProcess( PROCESS_QUERY_INFORMATION, TRUE, GW2Pid ) );

            LOG_DBG( "[GW2TacO] Taco integrity: %x, GW2 integrity: %x", currentProcessIntegrity, gw2ProcessIntegrity );

            if ( gw2ProcessIntegrity > currentProcessIntegrity || gw2ProcessIntegrity == -1 )
              MessageBox( NULL, "GW2 seems to have more elevated rights than GW2 TacO.\nThis will probably result in TacO not being interactive when GW2 is in focus.\nIf this is an issue for you, restart TacO in Administrator mode.", "Warning", MB_ICONWARNING );

            //::SetWindowLong( handle, GWL_HWNDPARENT, (LONG)gw2Window );
            //::SetParent( handle, gw2Window );
          }
          FoundGW2Window = true;
          //auto style = GetWindowLong( handle, GWL_EXSTYLE );
          ////login window: 0x94000000
          ////fullscreen windowed: 0x94000000
          ////fullscreen: 0x02080020

          //int x = 0;

          RECT GW2ClientRect;
          POINT p = { 0, 0 };
          GetClientRect( gw2Window, &GW2ClientRect );
          ClientToScreen( gw2Window, &p );

          if ( GW2ClientRect.right - GW2ClientRect.left != pos.Width() || GW2ClientRect.bottom - GW2ClientRect.top != pos.Height() || p.x != pos.x1 || p.y != pos.y1 )
          {
            FORCEDDEBUGLOG( "moving TacO to be over GW2" );
            LOG_ERR( "[GW2TacO] gw2 window size change: %d %d %d %d (%d %d)", GW2ClientRect.left, GW2ClientRect.top, GW2ClientRect.right, GW2ClientRect.bottom, GW2ClientRect.right - GW2ClientRect.left, GW2ClientRect.bottom - GW2ClientRect.top );
            bool NeedsResize = GW2ClientRect.right - GW2ClientRect.left != pos.Width() || GW2ClientRect.bottom - GW2ClientRect.top != pos.Height();
            pos = CRect( GW2ClientRect.left + p.x, GW2ClientRect.top + p.y, GW2ClientRect.left + p.x + GW2ClientRect.right - GW2ClientRect.left, GW2ClientRect.top + p.y + GW2ClientRect.bottom - GW2ClientRect.top );

            ::SetWindowPos( handle, 0, pos.x1, pos.y1, pos.Width(), pos.Height(), SWP_NOREPOSITION );
            //::SetWindowPos( gw2Window, handle, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
            //::SetParent( handle, gw2Window );
            //::SetWindowLong( handle, GWL_STYLE, WS_CHILD );

            if ( NeedsResize )
            {
              App->HandleResize();
              MARGINS marg = { -1, -1, -1, -1 };
              DwmExtendFrameIntoClientArea( handle, &marg );
            }
          }

          auto foregroundWindow = GetForegroundWindow();

          if ( foregroundWindow == gw2Window && App->GetFocusItem() && App->GetFocusItem()->InstanceOf( "textbox" ) )
          {
            SetForegroundWindow( (HWND)App->GetHandle() );
            SetFocus( (HWND)App->GetHandle() );
            ::SetWindowPos( (HWND)App->GetHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
          }

          if ( foregroundWindow == (HWND)App->GetHandle() && !( App->GetFocusItem() && App->GetFocusItem()->InstanceOf( "textbox" ) ) )
          {
            SetForegroundWindow( gw2Window );
            SetFocus( gw2Window );
          }

          bool EditedButNotSelected = ( foregroundWindow != gw2Window && foregroundWindow != (HWND)App->GetHandle() && App->GetFocusItem() && App->GetFocusItem()->InstanceOf( "textbox" ) );
          if ( EditedButNotSelected )
            App->GetRoot()->SetFocus();

          if ( gw2Window && ( !( App->GetFocusItem() && App->GetFocusItem()->InstanceOf( "textbox" ) || EditedButNotSelected ) ) )
          {
            HWND wnd = ::GetNextWindow( gw2Window, GW_HWNDPREV );
            if ( wnd != handle )
            {
              if ( wnd )
                ::SetWindowPos( handle, wnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
              else
                ::SetWindowPos( handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
            }

            App->GetRoot()->Hide( !shortTick || ( ( GW2ClientRect.right - GW2ClientRect.left == 1120 ) && ( GW2ClientRect.bottom - GW2ClientRect.top == 976 ) && ( p.x != 0 || p.y != 0 ) ) );
          }
        }
        else
          App->GetRoot()->Hide( !shortTick );

        //App->GetRoot()->Hide( GetForegroundWindow() != gw2Window && GetForegroundWindow() != (HWND)App->GetHandle() );

        //HANDLE h = GetForegroundWindow();
        //if ( h != (HWND)App->GetHandle() && h != gw2Window )
        //  ::SetWindowPos( (HWND)App->GetHandle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        //else
        //  ::SetWindowPos( (HWND)App->GetHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );

        //FORCEDDEBUGLOG( "hid the TacO gui: %d", (int)( GetForegroundWindow() != gw2Window && GetForegroundWindow() != (HWND)App->GetHandle() ) );

        taco->TickScriptEngine();
        AutoSaveConfig();

        {
          //CLightweightCriticalSection cs( &renderCritSec );
          App->Display();
        }
        //App->GetDevice()->WaitRetrace();
        frameTriggered = false;
        lastRenderTime = currTime;
        hadRetrace = false;
      }
      //DwmFlush();

      if ( !HooksInitialized )
      {
        FORCEDDEBUGLOG( "hooks not initialized, doing that" );
        if ( !IsDebuggerPresent() )
        {

          FORCEDDEBUGLOG( "creating thread" );
          auto hookThread = CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            []( LPVOID data )
            {
              auto keyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, KeyboardHook, NULL, 0 );
              auto mouseHook = SetWindowsHookEx( WH_MOUSE_LL, MouseHook, NULL, 0 );
              MSG msg;

              if ( !keyboardHook || !mouseHook )
              {
                FORCEDDEBUGLOG( "failed to set mouse or keyboard hook" );
              }
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
            0,          // argument to thread function 
              0,                      // use default creation flags 
              &hookThreadID );
        }
        HooksInitialized = true;
      }
    }
    else
      LOG_ERR( "[GW2TacO] Device fail" );

    mainLoopCounter++;
    lastMainLoopTime = GetTime();

    //if ( GetAsyncKeyState( VK_HOME ) )
    //  Sleep( 5000 );
  }

  FlushZipDict();

  FORCEDDEBUGLOG( "closing down taco" );

  if ( hookThreadID )
    PostThreadMessage( hookThreadID, WM_QUIT, 0, 0 );

  ShowWindow( handle, SW_HIDE );

  //ExportPOIS();
  //ExportPOIActivationData();
  SaveConfig();
  FORCEDDEBUGLOG( "config saved" );

  while ( keyboardHookActive );
  while ( mouseHookActive );

  extern std::thread wvwPollThread;
  extern std::thread wvwUpdatThread;

  if ( wvwPollThread.joinable() )
  {
    wvwPollThread.join();
    Sleep( 1000 );
  }
  if ( wvwUpdatThread.joinable() )
    wvwUpdatThread.join();

  extern std::unordered_map<int, CDictionaryEnumerable<GUID, GW2Trail*>> trailSet;
  
  for ( auto& trails : trailSet )
  {
    for ( int x = 0; x < trails.second.NumItems(); x++ )
      delete trails.second.GetByIndex( x );
  }

  //extern CArray<CString> stringArray;
  //LOG_DBG( "string array count: %d", stringArray.NumItems() );

  //cleanup
  SAFEDELETE( App );

  FORCEDDEBUGLOG( "app deleted, returning from main()" );
  SAFEDELETE( localization );

  SAFEDELETE( ringbufferLog );
  SAFEDELETE( bugSplat );

  return true;
}