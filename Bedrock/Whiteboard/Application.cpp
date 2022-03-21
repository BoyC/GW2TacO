#include "BasePCH.h"
#include "Application.h"
#include <locale.h>
#include "../UtilLib/PNGDecompressor.h"
#include "WhiteBoard.h"

CWBApplication::CWBApplication() : CCoreWindowHandlerWin()
{
  Root = NULL;
  MouseCaptureItem = NULL;
  MouseItem = NULL;
  DrawAPI = new CWBDrawAPI();
  Skin = new CWBSkin();
  Atlas = NULL;
  DefaultFont = NULL;
  Alt = Ctrl = Shift = Left = Middle = Right = false;
  Vsync = true;
  FrameTimes = new CRingBuffer<TS32>( 60 );
  LastFrameTime = 0;

  //initialize default factory calls

  RegisterUIFactoryCallback( _T( "window" ), CWBWindow::Factory );
  RegisterUIFactoryCallback( _T( "button" ), CWBButton::Factory );
  RegisterUIFactoryCallback( _T( "box" ), CWBBox::Factory );
  RegisterUIFactoryCallback( _T( "label" ), CWBLabel::Factory );
  RegisterUIFactoryCallback( _T( "textbox" ), CWBTextBox::Factory );
}

CWBApplication::~CWBApplication()
{
  Fonts.FreeAll();
  SAFEDELETE( Skin );
  SAFEDELETE( Atlas );
  SAFEDELETE( DrawAPI );
  SAFEDELETE( Root );
  SAFEDELETE( FrameTimes );
  LayoutRepository.FreeAll();
}

TBOOL CWBApplication::SendMessageToItem( CWBMessage &Message, CWBItem *Target )
{
  if ( Root && Message.GetTarget() == Root->GetGuid() ) Target = Root;

  if ( !Target )
  {
    //LOG(LOG_WARNING,_T("[gui] Message target item %d not found. Message type: %d"),Message.GetTarget(),Message.GetMessage());
    return false;
  }

  MessagePath.FlushFast();

  while ( Target )
  {
    MessagePath += Target;
    Target = Target->GetParent();
  }

  for ( TS32 x = MessagePath.NumItems() - 1; x >= 0; x-- )
    if ( MessagePath[ x ]->MessageProc( Message ) ) return true;

  return false;
}

void CWBApplication::ProcessMessage( CWBMessage &Message )
{
  if ( Message.GetTarget() )
  {
    CWBItem *Target = FindItemByGuid( Message.GetTarget() );
    SendMessageToItem( Message, Target );
    return;
  }

  //handle messages created by mouse events
  if ( Message.IsMouseMessage() )
  {
    if ( Message.GetMessage() == WBM_MOUSEMOVE )
    {
      MousePos = CPoint( Message.Position );
      UpdateMouseItem();
    }

    if ( Message.GetMessage() == WBM_LEFTBUTTONDOWN ) LeftDownPos = CPoint( Message.Position );
    if ( Message.GetMessage() == WBM_RIGHTBUTTONDOWN ) RightDownPos = CPoint( Message.Position );
    if ( Message.GetMessage() == WBM_MIDDLEBUTTONDOWN ) MidDownPos = CPoint( Message.Position );

    if ( MouseCaptureItem ) //mouse messages are captured by this item, send them directly there
    {
      MouseCaptureItem->MessageProc( Message );
      return;
    }

    CWBItem *mi = GetItemUnderMouse( MousePos, Message.GetMessage() );

    if ( mi )
    {
      //handle focus change
      if ( Message.GetMessage() == WBM_LEFTBUTTONDOWN ||
           Message.GetMessage() == WBM_MIDDLEBUTTONDOWN ||
           Message.GetMessage() == WBM_RIGHTBUTTONDOWN )
        mi->SetFocus();

      SendMessageToItem( Message, mi );
    }

    return;
  }

  //handle messages aimed at the item in focus

  //top to bottom version:
  //CWBItem *fi=GetRoot();
  //while (fi)
  //{
  //	if (fi->MessageProc(Message)) return;
  //	fi=fi->ChildInFocus;
  //}

  //bottom to top version:
  MessagePath.FlushFast();

  CWBItem *Target = GetRoot();
  while ( Target )
  {
    MessagePath += Target;
    Target = Target->GetChildInFocus();
  }

  for ( TS32 x = MessagePath.NumItems() - 1; x >= 0; x-- )
    if ( MessagePath[ x ]->MessageProc( Message ) ) return;

}

CWBItem *CWBApplication::GetItemUnderMouse( CPoint &Point, WBMESSAGE w )
{
  CRect r = Root->GetScreenRect();
  return Root->GetItemUnderMouse( Point, r, w );
}

void CWBApplication::HandleResize()
{
  CCoreWindowHandlerWin::HandleResize();

  if ( Root )
    Root->SetPosition( CRect( 0, 0, XRes, YRes ) );
}

LRESULT CWBApplication::InjectMessage( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  return WindowProc( uMsg, wParam, lParam );
}

void CWBApplication::UpdateControlKeyStates()
{
  Alt = ( GetKeyState( VK_MENU ) < 0 ) || ( GetAsyncKeyState( VK_MENU ) & 0x8000 ) || ( GetAsyncKeyState( VK_LMENU ) & 0x8000 ) || ( GetAsyncKeyState( VK_RMENU ) & 0x8000 );
  Ctrl = ( GetKeyState( VK_CONTROL ) < 0 ) || ( GetAsyncKeyState( VK_CONTROL ) & 0x8000 ) || ( GetAsyncKeyState( VK_LCONTROL ) & 0x8000 ) || ( GetAsyncKeyState( VK_RCONTROL ) & 0x8000 );
  Shift = ( GetKeyState( VK_SHIFT ) < 0 ) || ( GetAsyncKeyState( VK_SHIFT ) & 0x8000 ) || ( GetAsyncKeyState( VK_LSHIFT ) & 0x8000 ) || ( GetAsyncKeyState( VK_RSHIFT ) & 0x8000 );
}

TS32 CWBApplication::GetKeyboardState()
{
  return ( Alt*WB_KBSTATE_ALT ) | ( Ctrl*WB_KBSTATE_CTRL ) | ( Shift*WB_KBSTATE_SHIFT );
}

TS32 CWBApplication::GetInitialKeyboardDelay()
{
  TS32 setting = 0;
  if ( !SystemParametersInfo( SPI_GETKEYBOARDDELAY, 0, &setting, 0 ) ) return ( 1 + 1 ) * 250;  //1 by default
  return ( setting + 1 ) * 250;
}

TS32 CWBApplication::GetKeyboardRepeatTime()
{
  TS32 setting = 0;
  if ( !SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &setting, 0 ) ) return 400 - ( 31 * 12 ); //31 by default
  return 400 - ( setting * 12 );
}

LRESULT CWBApplication::WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
  //LOG(LOG_DEBUG,_T("[gui] WM_%d %d %d"),uMsg,wParam,lParam);

  switch ( uMsg )
  {
  case WM_ACTIVATE:
    if ( LOWORD( wParam ) )
      UpdateControlKeyStates();
    break;
  case WM_MOUSEMOVE:
  {
    POINT ap;
    GetCursorPos( &ap );
    ScreenToClient( hWnd, &ap );
    SendMessage( CWBMessage( this, WBM_MOUSEMOVE, 0, ap.x, ap.y ) );
  }
  break;
  case WM_LBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
  {
    Left = true;

    ::SetCapture( hWnd );
    POINT ap;
    GetCursorPos( &ap );
    ScreenToClient( hWnd, &ap );
    SendMessage( CWBMessage( this, WBM_LEFTBUTTONDOWN, 0, ap.x, ap.y ) );

    if ( uMsg == WM_LBUTTONDBLCLK )
      SendMessage( CWBMessage( this, WBM_LEFTBUTTONDBLCLK, 0, ap.x, ap.y ) );

    ClickRepeaterMode = WB_MCR_LEFT;
    NextRepeatedClickTime = globalTimer.GetTime() + GetInitialKeyboardDelay();
  }
  break;
  case WM_LBUTTONUP:
  {
    Left = false;
    ::ReleaseCapture();
    POINT ap;
    GetCursorPos( &ap );
    ScreenToClient( hWnd, &ap );
    SendMessage( CWBMessage( this, WBM_LEFTBUTTONUP, 0, ap.x, ap.y ) );
    ClickRepeaterMode = WB_MCR_OFF;
  }
  break;
  case WM_RBUTTONDOWN:
  case WM_RBUTTONDBLCLK:
  {
    Right = true;
    ::SetCapture( hWnd );
    POINT ap;
    GetCursorPos( &ap );
    ScreenToClient( hWnd, &ap );
    SendMessage( CWBMessage( this, WBM_RIGHTBUTTONDOWN, 0, ap.x, ap.y ) );

    if ( uMsg == WM_RBUTTONDBLCLK )
      SendMessage( CWBMessage( this, WBM_RIGHTBUTTONDBLCLK, 0, ap.x, ap.y ) );

    ClickRepeaterMode = WB_MCR_RIGHT;
    NextRepeatedClickTime = globalTimer.GetTime() + GetInitialKeyboardDelay();
  }
  break;
  case WM_RBUTTONUP:
  {
    Right = false;
    ::ReleaseCapture();
    POINT ap;
    GetCursorPos( &ap );
    ScreenToClient( hWnd, &ap );
    SendMessage( CWBMessage( this, WBM_RIGHTBUTTONUP, 0, ap.x, ap.y ) );
    ClickRepeaterMode = WB_MCR_OFF;
  }
  break;
  case WM_MBUTTONDOWN:
  case WM_MBUTTONDBLCLK:
  {
    Middle = true;
    ::SetCapture( hWnd );
    POINT ap;
    GetCursorPos( &ap );
    ScreenToClient( hWnd, &ap );
    SendMessage( CWBMessage( this, WBM_MIDDLEBUTTONDOWN, 0, ap.x, ap.y ) );

    if ( uMsg == WM_MBUTTONDBLCLK )
      SendMessage( CWBMessage( this, WBM_MIDDLEBUTTONDBLCLK, 0, ap.x, ap.y ) );

    ClickRepeaterMode = WB_MCR_MIDDLE;
    NextRepeatedClickTime = globalTimer.GetTime() + GetInitialKeyboardDelay();
  }
  break;
  case WM_MBUTTONUP:
  {
    Middle = false;
    ::ReleaseCapture();
    POINT ap;
    GetCursorPos( &ap );
    ScreenToClient( hWnd, &ap );
    SendMessage( CWBMessage( this, WBM_MIDDLEBUTTONUP, 0, ap.x, ap.y ) );
    ClickRepeaterMode = WB_MCR_OFF;
  }
  break;
  case WM_MOUSEWHEEL:
  {
    SendMessage( CWBMessage( this, WBM_MOUSEWHEEL, 0, GET_WHEEL_DELTA_WPARAM( wParam ) / WHEEL_DELTA ) );
  }
  break;
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN:
    //LOG_ERR( "[wndproc] WM_SYSKEYDOWN %d %d", wParam, lParam );

    if ( wParam == VK_CONTROL || wParam == VK_LCONTROL || wParam == VK_RCONTROL )
      Ctrl = true;
    if ( wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT )
      Shift = true;
    if ( wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU )
      Alt = true;

    //UpdateControlKeyStates();
    //LOG_ERR( "[wndproc] WM_KEYDOWN/WM_SYSKEYDOWN: %d %d %d %d %d %d", uMsg, wParam, lParam, Alt, Ctrl, Shift );
    SendMessage( CWBMessage( this, WBM_KEYDOWN, 0, wParam, GetKeyboardState() ) );
    break;
  case WM_SYSKEYUP:
  case WM_KEYUP:
    //LOG_DBG("[app] Keyup: %d",wParam);
    if ( wParam == VK_CONTROL || wParam == VK_LCONTROL || wParam == VK_RCONTROL )
      Ctrl = false;
    if ( wParam == VK_SHIFT || wParam == VK_LSHIFT || wParam == VK_RSHIFT )
      Shift = false;
    if ( wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU )
      Alt = false;

    //LOG_ERR( "[wndproc] WM_KEYUP/WM_SYSKEYUP: %d %d %d", uMsg, wParam, lParam );
    switch ( wParam )
    {
    case VK_SNAPSHOT:
      TakeScreenshot();
      break;
    }

    //UpdateControlKeyStates();
    SendMessage( CWBMessage( this, WBM_KEYUP, 0, wParam, GetKeyboardState() ) );
    break;
  case WM_SYSCHAR:
  case WM_CHAR:
    //LOG_DBG("[app] Char: %d",wParam);
    //LOG_ERR( "[wndproc] WM_CHAR/WM_SYSCHAR %d %d", uMsg, wParam, lParam );
    //UpdateControlKeyStates();
    SendMessage( CWBMessage( this, WBM_CHAR, 0, wParam, GetKeyboardState() ) );
    break;
  default:
    break;
  }

  return CCoreWindowHandlerWin::WindowProc( uMsg, wParam, lParam );
}

TBOOL CWBApplication::Initialize()
{
  Atlas = new CAtlas( 2048, 2048 );
  if ( !Atlas->InitializeTexture( Device ) )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Texture Atlas" ) );
    return false;
  }

  if ( !DrawAPI->Initialize( this, Device, Atlas ) ) return false;

  Root = new CWBRoot( NULL, CRect( 0, 0, XRes, YRes ) );
  Root->SetApplication( this );

  setlocale( LC_NUMERIC, "C" );

  return true;
}

TBOOL CWBApplication::Initialize( const CCoreWindowParameters &WindowParams )
{
  if ( !CCoreWindowHandlerWin::Initialize( WindowParams ) ) return false;
  return Initialize();
}

CWBItem *CWBApplication::GetRoot()
{
  return Root;
}

CWBItem *CWBApplication::GetFocusItem()
{
  CWBItem *fi = Root;
  while ( fi )
  {
    if ( !fi->ChildInFocus ) return fi;
    fi = fi->ChildInFocus;
  }
  return Root;
}

TBOOL CWBApplication::HandleMessages()
{
  //as this function is called once every frame, it's the best place to update the timer as well
  globalTimer.Update();

  //mouse click repeater
  if ( ClickRepeaterMode != WB_MCR_OFF )
  {
    if ( globalTimer.GetTime() > NextRepeatedClickTime )
    {
      POINT ap;
      GetCursorPos( &ap );
      ScreenToClient( hWnd, &ap );

      switch ( ClickRepeaterMode )
      {
      case WB_MCR_LEFT:
        SendMessage( CWBMessage( this, WBM_LEFTBUTTONREPEAT, 0, ap.x, ap.y ) );
        break;
      case WB_MCR_RIGHT:
        SendMessage( CWBMessage( this, WBM_RIGHTBUTTONREPEAT, 0, ap.x, ap.y ) );
        break;
      case WB_MCR_MIDDLE:
        SendMessage( CWBMessage( this, WBM_MIDDLEBUTTONREPEAT, 0, ap.x, ap.y ) );
        break;

      }
      NextRepeatedClickTime += GetKeyboardRepeatTime();
    }
  }

  if ( !CCoreWindowHandlerWin::HandleMessages() ) return false;

  //handle gui messages here

  for ( TS32 x = 0; x < MessageBuffer.NumItems(); x++ )
  {
    CWBMessage currentMessage;
    memcpy( &currentMessage, &MessageBuffer[ x ], sizeof( CWBMessage ) );
    ProcessMessage( currentMessage );
  }

  MessageBuffer.FlushFast();

  return true;
}

void CWBApplication::RegisterItem( CWBItem *Item )
{
  Items[ Item->GetGuid() ] = Item;

  if ( Item->GetScreenRect().Contains( MousePos ) )
    UpdateMouseItem();
}

void CWBApplication::UnRegisterItem( CWBItem *Item )
{
  Trash.Delete( Item );

  if ( MouseCaptureItem == Item ) ReleaseCapture();
  Items.Delete( Item->GetGuid() );

  if ( Item->GetScreenRect().Contains( MousePos ) )
    UpdateMouseItem();
}

void CWBApplication::SetDone( TBOOL d )
{
  Done = d;
}

void CWBApplication::Display()
{
  Display( DrawAPI );
}

void CWBApplication::Display( CWBDrawAPI *API )
{
  //LOG_DBG("Begin Frame");

  CleanTrash();

  FinalizeMouseCursor();
  SelectMouseCursor( CM_ARROW );

  DrawAPI->SetUIRenderState();

  Device->WaitRetrace();

  Device->Clear( true, true, ClearColor );
  Device->BeginScene();

  DrawAPI->SetOffset( Root->GetScreenRect().TopLeft() );
  DrawAPI->SetParentCropRect( Root->GetScreenRect() );
  DrawAPI->SetCropRect( Root->GetScreenRect() );

  Atlas->ClearImageUsageflags();

  Root->DrawTree( API );

  DrawAPI->RenderDisplayList();
  Device->EndScene();
  Device->Flip( Vsync );

  //LOG_DBG("End Frame");

  //this is a convenient place to store this:
  Logger.ResetEntryCounter();

  //update frame time
  TS32 frametime = globalTimer.GetTime();
  FrameTimes->Add( frametime - LastFrameTime );
  LastFrameTime = frametime;
}

CWBItem *CWBApplication::FindItemByGuid( WBGUID Guid, const TCHAR *type )
{
  if ( !Items.HasKey( Guid ) ) return NULL;
  CWBItem *i = Items[ Guid ];

  if ( type )
    return i->InstanceOf( type ) ? i : NULL;

  return i;
}

void CWBApplication::SendMessage( CWBMessage &Message )
{
  MessageBuffer += Message;
}

CWBItem *CWBApplication::SetCapture( CWBItem *Capturer )
{
  CWBItem *old = MouseCaptureItem;
  MouseCaptureItem = Capturer;
  return old;
}

TBOOL CWBApplication::ReleaseCapture()
{
  MouseCaptureItem = NULL;
  return true;
}

void CWBApplication::UpdateMouseItem()
{
  CWBItem *MouseItemOld = MouseItem;
  MouseItem = GetItemUnderMouse( MousePos, WBM_MOUSEMOVE );
  if ( MouseItem != MouseItemOld )
  {
    //call onmouseleave for the old items
    if ( MouseItem )
    {
      CWBItem *olditem = MouseItemOld;
      while ( olditem )
      {
        //find the old item in the tree of the new. if not found the mouse left the item
        if ( !MouseItem->FindItemInParentTree( olditem ) ) olditem->OnMouseLeave();
        olditem = olditem->Parent;
      }
    }

    //call onmouseenter for the new items
    if ( MouseItemOld )
    {
      CWBItem *newitem = MouseItem;
      while ( newitem )
      {
        //find the new item in the tree of the old. if not found the mouse entered the item
        if ( !MouseItemOld->FindItemInParentTree( newitem ) ) newitem->OnMouseEnter();
        newitem = newitem->Parent;
      }
    }
  }
}

void CWBApplication::CleanTrash()
{
  while ( Trash.NumItems() )
  {
    delete Trash[ Trash.NumItems() - 1 ];
    Trash.DeleteByIndex( Trash.NumItems() - 1 );
  }
}

CWBItem *CWBApplication::GetMouseItem()
{
  return MouseItem;
}

CWBItem *CWBApplication::GetMouseCaptureItem()
{
  return MouseCaptureItem;
}

TBOOL CWBApplication::CreateFont( CString FontName, CWBFontDescription *Font )
{
  if ( !Font ) return false;

  CWBFont *f = new CWBFont( Atlas );

  if ( !f->Initialize( Font ) )
  {
    SAFEDELETE( f );
    return false;
  }

  if ( Fonts.HasKey( FontName ) )
  {
    SAFEDELETE( Fonts[ FontName ] );
  }

  if ( !Fonts.NumItems() )
    DefaultFont = f;

  Fonts[ FontName ] = f;

  return true;
}

CWBFont *CWBApplication::GetFont( CString FontName )
{
  CWBFont *Font = Fonts.GetExisting( FontName );
  if ( Font ) return Font;
  LOG_WARN( "[gui] Font %s not found, falling back to default font", FontName.GetPointer() );
  return DefaultFont;
}

CWBFont *CWBApplication::GetDefaultFont()
{
  return DefaultFont;
}

TBOOL CWBApplication::SetDefaultFont( CString FontName )
{
  CWBFont *f = GetFont( FontName );
  if ( !f ) return false;
  DefaultFont = f;
  return true;
}

void CWBApplication::AddToTrash( CWBItem *item )
{
  Trash.AddUnique( item );
}

TBOOL CWBApplication::LoadXMLLayout( CString & XML )
{
  CXMLDocument *doc = new CXMLDocument();
  if ( !doc->LoadFromString( XML ) )
  {
    LOG_ERR( "[gui] Error loading XML Layout: parsing failed" );
    delete doc;
    return false;
  }

  CXMLNode root = doc->GetDocumentNode();

  if ( !root.GetChildCount( _T( "guidescriptor" ) ) )
  {
    LOG_ERR( "[gui] Error loading XML Layout: 'guidescriptor' member missing" );
    delete doc;
    return false;
  }

  root = root.GetChild( _T( "guidescriptor" ) );

  if ( !root.GetChildCount( _T( "gui" ) ) )
  {
    LOG_ERR( "[gui] Error loading XML Layout: 'gui' member missing" );
    delete doc;
    return false;
  }

  CXMLNode gui = root.GetChild( _T( "gui" ) );
  if ( !gui.HasAttribute( _T( "id" ) ) )
  {
    LOG_ERR( "[gui] Error loading XML Layout: 'id' member missing from 'gui'" );
    delete doc;
    return false;
  }

  CString id = gui.GetAttribute( _T( "id" ) );

  LayoutRepository.Free( id );
  LayoutRepository[ id ] = doc;

  //LOG_NFO("[gui] Successfully loaded XML layout '%s'",id.GetPointer());

  return true;
}

TBOOL CWBApplication::LoadXMLLayoutFromFile( CString FileName )
{
  CStreamReaderMemory f;
  if ( !f.Open( FileName.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading XML Layout: file '%s' not found", FileName.GetPointer() );
    return false;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );
  return LoadXMLLayout( s );
}

TBOOL CWBApplication::GenerateGUI( CWBItem *Root, CString &Layout )
{
  if ( !LayoutRepository.HasKey( Layout ) )
  {
    LOG_ERR( "[gui] Error generating UI: layout '%s' not loaded", Layout.GetPointer() );
    return false;
  }

  TBOOL b = GenerateGUIFromXML( Root, LayoutRepository[ Layout ] );
  if ( !b ) return false;

  StyleManager.ApplyStyles( Root );
  CWBMessage m;
  Root->BuildPositionMessage( Root->GetPosition(), m );
  m.Resized = true;
  Root->MessageProc( m );
  return true;
}

TBOOL CWBApplication::GenerateGUITemplate( CWBItem *Root, CString &Layout, CString &TemplateID )
{
  if ( !LayoutRepository.HasKey( Layout ) )
  {
    LOG_ERR( "[gui] Error generating UI template: layout '%s' not loaded", Layout.GetPointer() );
    return false;
  }

  TBOOL b = GenerateGUITemplateFromXML( Root, LayoutRepository[ Layout ], TemplateID );
  if ( !b ) return false;

  StyleManager.ApplyStyles( Root );
  CWBMessage m;
  Root->BuildPositionMessage( Root->GetPosition(), m );
  m.Resized = true;
  Root->MessageProc( m );

  return true;
}

TBOOL CWBApplication::GenerateGUITemplate( CWBItem *Root, TCHAR *Layout, TCHAR *TemplateID )
{
  return GenerateGUITemplate( Root, CString( Layout ), CString( TemplateID ) );
}

void CWBApplication::ApplyStyle( CWBItem *Target )
{
  StyleManager.ApplyStyles( Target );
  CWBMessage m;
  Target->BuildPositionMessage( Target->GetPosition(), m );
  m.Resized = true;
  Target->MessageProc( m );
}

void CWBApplication::ReApplyStyle()
{
  CWBItem *Root = GetRoot();
  StyleManager.ApplyStyles( Root );
  CWBMessage m;
  Root->BuildPositionMessage( Root->GetPosition(), m );
  m.Resized = true;
  Root->MessageProc( m );
}

TBOOL CWBApplication::GenerateGUI( CWBItem *Root, const TCHAR *layout )
{
  //LOG_NFO("[gui] Generating UI Layout '%s'",layout);
  return GenerateGUI( Root, CString( layout ) );
}

TBOOL CWBApplication::LoadCSS( CString & CSS, TBOOL ResetStyleManager )
{
  if ( ResetStyleManager )
    StyleManager.Reset();
  return StyleManager.ParseStyleData( CSS );
}

TBOOL CWBApplication::LoadCSSFromFile( CString FileName, TBOOL ResetStyleManager )
{
  CStreamReaderMemory f;
  if ( !f.Open( FileName.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading CSS: file '%s' not found", FileName.GetPointer() );
    return false;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );
  TBOOL b = LoadCSS( s, ResetStyleManager );
  //if (b)
  //	LOG_NFO("[gui] Successfully loaded CSS '%s'",FileName.GetPointer());

  return b;
}

CAtlas *CWBApplication::GetAtlas()
{
  return Atlas;
}

CWBSkin * CWBApplication::GetSkin()
{
  return Skin;
}

TBOOL CWBApplication::LoadSkin( CString &XML, CArray<int>& enabledGlyphs )
{
  CXMLDocument doc;
  if ( !doc.LoadFromString( XML ) ) return false;
  if ( !doc.GetDocumentNode().GetChildCount( _T( "whiteboardskin" ) ) ) return false;

  CXMLNode r = doc.GetDocumentNode().GetChild( _T( "whiteboardskin" ) );

  for ( TS32 x = 0; x < r.GetChildCount( _T( "image" ) ); x++ )
  {
    CXMLNode n = r.GetChild( _T( "image" ), x );
    CString img = n.GetAttributeAsString( _T( "image" ) );

    TU8 *Data = NULL;
    TS32 Size = 0;
    img.DecodeBase64( Data, Size );

    TU8 *Image;
    TS32 XRes, YRes;
    if ( DecompressPNG( Data, Size, Image, XRes, YRes ) )
    {
      ARGBtoABGR( Image, XRes, YRes );
      ClearZeroAlpha( Image, XRes, YRes );

      for ( TS32 y = 0; y < n.GetChildCount( _T( "element" ) ); y++ )
      {
        CXMLNode e = n.GetChild( _T( "element" ), y );

        CRect r2;
        e.GetAttributeAsInteger( _T( "x1" ), &r2.x1 );
        e.GetAttributeAsInteger( _T( "y1" ), &r2.y1 );
        e.GetAttributeAsInteger( _T( "x2" ), &r2.x2 );
        e.GetAttributeAsInteger( _T( "y2" ), &r2.y2 );

        CPoint b;
        e.GetAttributeAsInteger( _T( "x-behavior" ), &b.x );
        e.GetAttributeAsInteger( _T( "y-behavior" ), &b.y );

        WBATLASHANDLE h = Atlas->AddImage( Image, XRes, YRes, r2 );
        Skin->AddElement( e.GetAttributeAsString( _T( "name" ) ), h, (WBSKINELEMENTBEHAVIOR)b.x, (WBSKINELEMENTBEHAVIOR)b.y );
      }

      SAFEDELETEA( Image );
    }

    SAFEDELETEA( Data );
  }

  for ( TS32 x = 0; x < r.GetChildCount( _T( "mosaic" ) ); x++ )
  {
    CXMLNode m = r.GetChild( _T( "mosaic" ), x );

    CRect r2;
    m.GetAttributeAsInteger( _T( "overshootx1" ), &r2.x1 );
    m.GetAttributeAsInteger( _T( "overshooty1" ), &r2.y1 );
    m.GetAttributeAsInteger( _T( "overshootx2" ), &r2.x2 );
    m.GetAttributeAsInteger( _T( "overshooty2" ), &r2.y2 );

    Skin->AddMosaic( m.GetAttributeAsString( _T( "name" ) ), m.GetAttributeAsString( _T( "description" ) ), r2.x1, r2.y1, r2.x2, r2.y2 );
  }

  for ( TS32 x = 0; x < r.GetChildCount( _T( "font" ) ); x++ )
  {
    CXMLNode n = r.GetChild( _T( "font" ), x );

    CString Name = n.GetAttributeAsString( _T( "name" ) );

    CString img = n.GetAttributeAsString( _T( "image" ) );
    CString bin = n.GetAttributeAsString( _T( "binary" ) );

    TU8 *Dataimg = NULL;
    TU8 *Databin = NULL;
    TS32 Sizeimg = 0;
    TS32 Sizebin = 0;
    img.DecodeBase64( Dataimg, Sizeimg );
    bin.DecodeBase64( Databin, Sizebin );

    TS32 XRes, YRes;
    TU8 *Image;
    if ( DecompressPNG( Dataimg, Sizeimg, Image, XRes, YRes ) )
    {
      ARGBtoABGR( Image, XRes, YRes );
      CWBFontDescription *fd = new CWBFontDescription();
      if ( fd->LoadBMFontBinary( Databin, Sizebin, Image, XRes, YRes, enabledGlyphs ) )
      {
        TBOOL f = CreateFont( Name, fd );
      }
      else
        if ( fd->LoadBMFontText( Databin, Sizebin, Image, XRes, YRes, enabledGlyphs ) )
        {
          TBOOL f = CreateFont( Name, fd );
        }

      SAFEDELETE( fd );
      SAFEDELETE( Image );
    }

    SAFEDELETE( Dataimg );
    SAFEDELETE( Databin );
  }

  return true;
}

TBOOL CWBApplication::LoadSkinFromFile( CString FileName, CArray<int>& enabledGlyphs )
{
  CStreamReaderMemory f;
  if ( !f.Open( FileName.GetPointer() ) )
  {
    LOG_ERR( "[gui] Error loading Skin: file '%s' not found", FileName.GetPointer() );
    return false;
  }

  CString s( (char*)f.GetData(), (TS32)f.GetLength() );
  TBOOL b = LoadSkin( s, enabledGlyphs );
  if ( b )
    LOG_NFO( "[gui] Successfully loaded Skin '%s'", FileName.GetPointer() );

  return b;
}

TBOOL CWBApplication::GenerateGUIFromXML( CWBItem *Root, CXMLDocument *doc )
{
  CXMLNode root = doc->GetDocumentNode();

  if ( !root.GetChildCount( _T( "guidescriptor" ) ) )
    return false;

  root = root.GetChild( _T( "guidescriptor" ) );

  if ( !root.GetChildCount( _T( "gui" ) ) )
    return false;

  CXMLNode gui = root.GetChild( _T( "gui" ) );

  return ProcessGUIXML( Root, gui );
}

TBOOL CWBApplication::GenerateGUITemplateFromXML( CWBItem *Root, CXMLDocument *doc, CString &TemplateID )
{
  CXMLNode root = doc->GetDocumentNode();

  if ( !root.GetChildCount( _T( "guidescriptor" ) ) )
    return false;

  root = root.GetChild( _T( "guidescriptor" ) );

  for ( TS32 x = 0; x < root.GetChildCount( _T( "guitemplate" ) ); x++ )
  {
    CXMLNode t = root.GetChild( _T( "guitemplate" ), x );

    if ( t.HasAttribute( _T( "id" ) ) )
      if ( t.GetAttributeAsString( _T( "id" ) ) == TemplateID )
      {
        if ( t.HasAttribute( _T( "class" ) ) )
        {
          CStringArray a = t.GetAttribute( _T( "class" ) ).Explode( _T( " " ) );
          for ( int i = 0; i < a.NumItems(); i++ )
            if ( a[ i ].Length() > 1 )
              Root->AddClass( a[ i ] );
        }

        return ProcessGUIXML( Root, t );
      }
  }

  return false;
}


TBOOL CWBApplication::ProcessGUIXML( CWBItem *Root, CXMLNode & node )
{
  CRect Pos( 5, 5, 25, 25 );

  TBOOL b = true;
  for ( int i = 0; i < node.GetChildCount(); i++ )
  {
    b &= GenerateGUIFromXMLNode( Root, node.GetChild( i ), Pos );
    Pos = CRect( Pos.BottomLeft() + CPoint( 0, 2 ), Pos.BottomLeft() + CPoint( 20, 22 ) );
  }
  return b;
}

TBOOL CWBApplication::GenerateGUIFromXMLNode( CWBItem * Root, CXMLNode & node, CRect &Pos )
{
  CWBItem *NewItem = GenerateUIItem( Root, node, Pos );
  if ( !NewItem ) return false;

  if ( node.HasAttribute( _T( "pos" ) ) )
  {
    if ( node.GetAttribute( _T( "pos" ) ).GetSubstringCount( _T( "," ) ) == 3 )
    {
      node.GetAttribute( _T( "pos" ) ).Scan( _T( "%d,%d,%d,%d" ), &Pos.x1, &Pos.y1, &Pos.x2, &Pos.y2 );
    }
    else
    {
      TU32 x = 0, y = 0;
      node.GetAttribute( _T( "pos" ) ).Scan( _T( "%d,%d" ), &x, &y );
      Pos.MoveTo( x, y );
    }
    NewItem->SetPosition( Pos );
  }

  if ( node.HasAttribute( _T( "size" ) ) )
  {
    TU32 x = 0, y = 0;
    node.GetAttribute( _T( "size" ) ).Scan( _T( "%d,%d" ), &x, &y );
    Pos.SetSize( x, y );
    NewItem->SetPosition( Pos );
  }

  if ( node.HasAttribute( _T( "id" ) ) )
    NewItem->SetID( node.GetAttribute( _T( "id" ) ) );

  if ( node.HasAttribute( _T( "class" ) ) )
  {
    CStringArray a = node.GetAttribute( _T( "class" ) ).Explode( _T( " " ) );
    for ( int i = 0; i < a.NumItems(); i++ )
      if ( a[ i ].Length() > 1 )
        NewItem->AddClass( a[ i ] );
  }

  return ProcessGUIXML( NewItem, node );
}

CWBItem * CWBApplication::GenerateUIItem( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  if ( FactoryCallbacks.HasKey( node.GetNodeName() ) )
  {
    return FactoryCallbacks[ node.GetNodeName() ]( Root, node, Pos );
  }

  LOG( LOG_ERROR, _T( "[xml2gui] Unknown tag: '%s'" ), node.GetNodeName().GetPointer() );
  return NULL;
}

void CWBApplication::RegisterUIFactoryCallback( CString &ElementName, WBFACTORYCALLBACK FactoryCallback )
{
  FactoryCallbacks[ ElementName ] = FactoryCallback;
}

void CWBApplication::RegisterUIFactoryCallback( TCHAR *ElementName, WBFACTORYCALLBACK FactoryCallback )
{
  RegisterUIFactoryCallback( CString( ElementName ), FactoryCallback );
}

void CWBApplication::TakeScreenshot()
{
  CreateDirectory( _T( "Screenshots" ), NULL );

  TS32 maxcnt = 0;
  {
    CString s = ScreenShotName + _T( "_*.png" );
    CFileList fl( s, _T( "Screenshots" ) );

    for ( TS32 x = 0; x < fl.Files.NumItems(); x++ )
    {
      if ( fl.Files[ x ].FileName.Find( ScreenShotName ) == 0 )
      {
        CString s2 = ScreenShotName + _T( "_%d" );

        TS32 no = -1;
        TS32 i = fl.Files[ x ].FileName.Scan( s2.GetPointer(), &no );
        maxcnt = max( maxcnt, no );
      }
    }
  }

  DrawAPI->FlushDrawBuffer();

  CCoreBlendState *b = DrawAPI->GetDevice()->CreateBlendState();
  b->SetBlendEnable( 0, true );
  b->SetIndependentBlend( true );
  b->SetSrcBlend( 0, COREBLEND_ZERO );
  b->SetDestBlend( 0, COREBLEND_ONE );
  b->SetSrcBlendAlpha( 0, COREBLEND_ONE );
  b->SetDestBlendAlpha( 0, COREBLEND_ZERO );
  DrawAPI->GetDevice()->SetRenderState( b );

  DrawAPI->SetCropRect( CRect( 0, 0, XRes, YRes ) );
  DrawAPI->DrawRect( CRect( 0, 0, XRes, YRes ), 0xff000000 );
  DrawAPI->FlushDrawBuffer();

  CString fname = CString::Format( _T( "Screenshots\\%s_%04d.png" ), ScreenShotName.GetPointer(), maxcnt + 1 );
  DrawAPI->GetDevice()->TakeScreenShot( fname );

  DrawAPI->SetUIRenderState();
  delete b;
}

TF32 CWBApplication::GetFrameRate()
{
  TS32 FrameTimeAcc = 0;
  TS32 FrameCount = 0;
  for ( TS32 x = 0; x < 60; x++ )
  {
    if ( FrameTimes->NumItems() < x ) break;
    FrameTimeAcc += ( *FrameTimes )[ FrameTimes->NumItems() - 1 - x ];
    FrameCount++;
  }

  if ( !FrameCount ) return 0;
  if ( !FrameTimeAcc ) return 9999;
  return 1000.0f / ( FrameTimeAcc / (TF32)FrameCount );
}

