#include "BasePCH.h"
#include "Window.h"

TU32 CWBWindow::GetBorderSelectionArea( CPoint &mousepos )
{
  if ( App->GetMouseItem() != this ) return 0;

  CRect r = GetScreenRect();

  if ( ( r + CRect( -BorderWidth, -BorderWidth, -BorderWidth, -BorderWidth ) ).Contains( mousepos ) ) return 0;

  TU32 result = 0;

  if ( r.x1 + CornerSelectionSize > mousepos.x ) result |= WB_DRAGMODE_LEFT;
  if ( r.x2 - CornerSelectionSize < mousepos.x ) result |= WB_DRAGMODE_RIGHT;
  if ( r.y1 + CornerSelectionSize > mousepos.y ) result |= WB_DRAGMODE_TOP;
  if ( r.y2 - CornerSelectionSize < mousepos.y ) result |= WB_DRAGMODE_BOTTOM;

  return result;
}

CRect CWBWindow::GetElementPos( WBWINDOWELEMENT Element )
{
  if ( Elements.HasKey( Element ) )
    return Elements[ Element ].PositionDescriptor.GetPosition( GetWindowRect().Size(), CSize( 0, 0 ), CRect( 0, 0, 10, 10 ) ) + GetWindowRect().TopLeft();

  switch ( Element )
  {
  case WB_WINELEMENT_CLOSE:
    return CRect( GetWindowRect().TopLeft(), GetWindowRect().TopLeft() + CPoint( 5, 5 ) );
    break;
  case WB_WINELEMENT_MINIMIZE:
    return CRect( GetWindowRect().TopLeft(), GetWindowRect().TopLeft() + CPoint( 5, 5 ) );
    break;
  case WB_WINELEMENT_INFO:
    return CRect( GetWindowRect().TopLeft(), GetWindowRect().TopLeft() + CPoint( 5, 5 ) );
    break;
  case WB_WINELEMENT_TITLE:
    return CRect( GetWindowRect().TopLeft(), GetWindowRect().TopRight() + CPoint( 0, 15 ) );
    break;
  default:
    return CRect();
    break;
  }
}

void CWBWindow::OnDraw( CWBDrawAPI *API )
{
  WBITEMSTATE i = GetState();
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  DrawBackground( API );

  //title text
  if ( ( Style & WB_WINDOW_TITLE ) )
  {
    CWBCSSPropertyBatch &TitleProps = Elements[ WB_WINELEMENT_TITLE ];

    CWBFont *Font = TitleProps.GetFont( App, i );
    if ( Font )
    {
      CRect titlepos = GetElementPos( WB_WINELEMENT_TITLE );
      titlepos = TitleProps.PositionDescriptor.GetPadding( titlepos.Size(), CRect( 0, 0, 0, 0 ) ) + titlepos.TopLeft();
      CRect r = API->GetCropRect();
      API->SetCropRect( ClientToScreen( titlepos ) );

      CPoint TitlePos = Font->GetTextPosition( WindowTitle, titlepos, TitleProps.TextAlignX, TitleProps.TextAlignY, (WBTEXTTRANSFORM)TitleProps.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM ) );
      Font->Write( API, WindowTitle, TitlePos, TitleProps.DisplayDescriptor.GetColor( i, WB_ITEM_FONTCOLOR ), TextTransform );
      API->SetCropRect( r );
    }
  }

  TBOOL MouseOverButton = false;

  //close button
  if ( Style & WB_WINDOW_CLOSEABLE )
  {
    CRect closebuttonpos = GetElementPos( WB_WINELEMENT_CLOSE );

    WBITEMSTATE buttonstate = WB_STATE_NORMAL;
    if ( App->GetMouseItem() == this )
      if ( closebuttonpos.Contains( ScreenToClient( App->GetMousePos() ) ) )
      {
        MouseOverButton = true;
        if ( App->GetMouseCaptureItem() == this )
          buttonstate = WB_STATE_ACTIVE;
        else
          buttonstate = WB_STATE_HOVER;
      }

    if ( Elements.HasKey( WB_WINELEMENT_CLOSE ) )
    {
      if ( Elements[ WB_WINELEMENT_CLOSE ].DisplayDescriptor.GetSkin( buttonstate, WB_ITEM_BACKGROUNDIMAGE ) != 0xffffffff )
      {
        int x = 0;
      }

      DrawBackgroundItem( API, Elements[ WB_WINELEMENT_CLOSE ].DisplayDescriptor, closebuttonpos, buttonstate );

      CWBFont *Font = GetFont( i );
      if ( Elements[ WB_WINELEMENT_CLOSE ].DisplayDescriptor.GetSkin( buttonstate, WB_ITEM_BACKGROUNDIMAGE ) == 0xffffffff )
        if ( Font ) Font->Write( API, _T( "X" ), Font->GetCenter( _T( "X" ), closebuttonpos ) );
    }
  }

  DrawBorder( API );

  if ( MouseOverButton || !( Style&WB_WINDOW_RESIZABLE ) ) return;
  if ( App->GetMouseCaptureItem() && App->GetMouseCaptureItem() != this ) return;
  if ( ScrollbarDragged() ) return;

  //change mouse cursor
  TS32 border = DragMode;
  if ( !border ) border = GetBorderSelectionArea( App->GetMousePos() );

  if ( ( border & WB_DRAGMODE_LEFT ) || ( border & WB_DRAGMODE_RIGHT ) )
    App->SelectMouseCursor( CM_SIZEWE );
  if ( ( border & WB_DRAGMODE_TOP ) || ( border & WB_DRAGMODE_BOTTOM ) )
    App->SelectMouseCursor( CM_SIZENS );
  if ( ( ( border & WB_DRAGMODE_LEFT ) && ( border & WB_DRAGMODE_TOP ) ) || ( ( border & WB_DRAGMODE_RIGHT ) && ( border & WB_DRAGMODE_BOTTOM ) ) ) App->SelectMouseCursor( CM_SIZENWSE );
  if ( ( ( border & WB_DRAGMODE_LEFT ) && ( border & WB_DRAGMODE_BOTTOM ) ) || ( ( border & WB_DRAGMODE_RIGHT ) && ( border & WB_DRAGMODE_TOP ) ) ) App->SelectMouseCursor( CM_SIZENESW );
}

CWBWindow::CWBWindow() : CWBItem()
{

}

CWBWindow::CWBWindow( CWBItem *Parent, const CRect &Pos, const TCHAR *txt, TU32 style ) : CWBItem()
{
  Initialize( Parent, Pos, txt, style );
}

CWBWindow::~CWBWindow()
{

}

TBOOL CWBWindow::Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *txt, TU32 style )
{
  WindowTitle = txt;

  DragMode = 0;

  Style = style;
  BorderWidth = 3;
  TitleBarHeight = style&WB_WINDOW_TITLE ? 12 : 0;
  CornerSelectionSize = 15;
  MinSize = CSize( CornerSelectionSize * 2 + 1, CornerSelectionSize * 2 + 1 );
  Elements[ WB_WINELEMENT_CLOSE ].PositionDescriptor.SetMetric( WB_MARGIN_TOP, WB_PIXELS, 3 );
  Elements[ WB_WINELEMENT_CLOSE ].PositionDescriptor.SetMetric( WB_MARGIN_RIGHT, WB_PIXELS, 3 );
  Elements[ WB_WINELEMENT_CLOSE ].PositionDescriptor.SetMetric( WB_WIDTH, WB_PIXELS, 11 );
  Elements[ WB_WINELEMENT_CLOSE ].PositionDescriptor.SetMetric( WB_HEIGHT, WB_PIXELS, 11 );
  Elements[ WB_WINELEMENT_CLOSE ].DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff2d2d30 ) );
  Elements[ WB_WINELEMENT_CLOSE ].DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1c97ea ) );
  Elements[ WB_WINELEMENT_CLOSE ].DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff3e3e40 ) );

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, 0xff2d2d30 );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, 0xff007acc );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BORDERCOLOR, 0xff424245 );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BORDERCOLOR, 0xff007acc );
  //DisplayDescriptor.SetValue(WB_STATE_NORMAL,WB_ITEM_CLIENTCOLOR,0xff252526);

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;
  SetBorderSizes( 1, 1, 1, 1 );
  SetClientPadding( BorderWidth - 1, TitleBarHeight + BorderWidth - 1, BorderWidth - 1, BorderWidth - 1 );
  return true;
}

TBOOL CWBWindow::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseItem() == this )
    {
      SetCapture();
      SavePosition();

      if ( Style & WB_WINDOW_CLOSEABLE )
      {
        if ( GetElementPos( WB_WINELEMENT_CLOSE ).Contains( ScreenToClient( Message.GetPosition() ) ) )
        {
          DragMode = WB_DRAGMODE_CLOSEBUTTON;
          return true;
        }
      }

      if ( Style & WB_WINDOW_RESIZABLE )
      {
        DragMode = GetBorderSelectionArea( Message.GetPosition() );
        if ( DragMode & WB_DRAGMASK ) return true;
      }

      if ( Style & WB_WINDOW_MOVEABLE )
      {
        if ( !GetClientRect().Contains( ScreenToClient( Message.GetPosition() ) ) )
        {
          DragMode = WB_DRAGMODE_MOVE;
          return true;
        }
      }

      DragMode = 0;
    }
    break;

  case WBM_MOUSEMOVE:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseCaptureItem() == this )
    {
      if ( DragMode == WB_DRAGMODE_MOVE && ( Style & WB_WINDOW_MOVEABLE ) )
      {
        SetPosition( GetSavedPosition() - App->GetLeftDownPos() + Message.GetPosition() );
        return true;
      }
      if ( ( DragMode & WB_DRAGMASK ) && ( Style & WB_WINDOW_RESIZABLE ) )
      {
        CRect r = GetSavedPosition();
        CPoint md = Message.GetPosition() - App->GetLeftDownPos();
        if ( DragMode & WB_DRAGMODE_LEFT ) r.x1 = min( r.x1 + md.x, r.x2 - MinSize.x );
        if ( DragMode & WB_DRAGMODE_TOP ) r.y1 = min( r.y1 + md.y, r.y2 - MinSize.y );
        if ( DragMode & WB_DRAGMODE_RIGHT ) r.x2 = max( r.x2 + md.x, r.x1 + MinSize.x );
        if ( DragMode & WB_DRAGMODE_BOTTOM ) r.y2 = max( r.y2 + md.y, r.y1 + MinSize.y );
        SetPosition( r );
        return true;
      }
    }
    break;

  case WBM_LEFTBUTTONUP:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseCaptureItem() == this )
    {
      if ( DragMode == WB_DRAGMODE_CLOSEBUTTON )
        if ( GetElementPos( WB_WINELEMENT_CLOSE ).Contains( ScreenToClient( Message.GetPosition() ) ) )
          App->SendMessage( CWBMessage( App, WBM_CLOSE, GetGuid() ) );

      TBOOL b = ReleaseCapture();
      if ( DragMode )
      {
        DragMode = 0;
        App->SendMessage( CWBMessage( App, WBM_WINDOWDRAGSTOPPED, GetGuid() ) );
        return b;
      }
    }
    break;

  case WBM_CLOSE:
    if ( Message.GetTarget() == GetGuid() )
      MarkForDeletion();
    break;

  }

  return CWBItem::MessageProc( Message );
}

TU32 CWBWindow::GetDragMode()
{
  return DragMode;
}

TBOOL CWBWindow::ApplyStyle( CString & prop, CString & value, CStringArray &pseudo )
{
  TBOOL ElementTarget = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "title" ) ||
         pseudo[ x ] == _T( "close" ) ||
         pseudo[ x ] == _T( "minimize" ) ||
         pseudo[ x ] == _T( "info" ) )
    {
      ElementTarget = true;
      break;
    }
  }

  //apply font styling to list item anyway
  if ( !ElementTarget ) InterpretFontString( Elements[ WB_WINELEMENT_TITLE ], prop, value, pseudo );
  if ( !ElementTarget ) return CWBItem::ApplyStyle( prop, value, pseudo );

  TBOOL Handled = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "title" ) )
    {
      Handled |= Elements[ WB_WINELEMENT_TITLE ].ApplyStyle( this, prop, value, pseudo );
      continue;
    }

    if ( pseudo[ x ] == _T( "close" ) )
    {
      Handled |= Elements[ WB_WINELEMENT_CLOSE ].ApplyStyle( this, prop, value, pseudo );
      continue;
    }

    if ( pseudo[ x ] == _T( "minimize" ) )
    {
      Handled |= Elements[ WB_WINELEMENT_MINIMIZE ].ApplyStyle( this, prop, value, pseudo );
      continue;
    }

    if ( pseudo[ x ] == _T( "info" ) )
    {
      Handled |= Elements[ WB_WINELEMENT_INFO ].ApplyStyle( this, prop, value, pseudo );
      continue;
    }
  }

  return Handled;
}

CWBItem * CWBWindow::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CWBWindow * window = new CWBWindow( Root, Pos );
  if ( node.HasAttribute( _T( "title" ) ) )
    window->SetTitle( node.GetAttribute( _T( "title" ) ) );
  return window;
}
