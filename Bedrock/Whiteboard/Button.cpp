#include "BasePCH.h"
#include "Button.h"

WBITEMSTATE CWBButton::GetState()
{
  WBITEMSTATE i = CWBItem::GetState();
  if ( Pushed )
  {
    i = IsEnabled() ? WB_STATE_ACTIVE : WB_STATE_DISABLED_ACTIVE;
  }
  else
  {
    if ( App->GetMouseCaptureItem() == this && MouseOver() ) i = WB_STATE_ACTIVE;
    else
    {
      if ( IsEnabled() )
        i = MouseOver() ? WB_STATE_HOVER : WB_STATE_NORMAL;
      else
        i = WB_STATE_DISABLED;
    }
  }

  if ( i == WB_STATE_HOVER && App->GetMouseCaptureItem() )
    if ( App->GetMouseCaptureItem() != this && App->GetMouseCaptureItem()->GetType() != _T( "contextmenu" ) ) i = WB_STATE_NORMAL;

  return i;
}

void CWBButton::OnDraw( CWBDrawAPI *API )
{
  DrawBackground( API );

  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  if ( Font )
  {
    CColor TextColor = CSSProperties.DisplayDescriptor.GetColor( i, WB_ITEM_FONTCOLOR );
    CPoint TextPos = Font->GetTextPosition( Text, GetClientRect(), CSSProperties.TextAlignX, CSSProperties.TextAlignY, TextTransform );
    Font->Write( API, Text, TextPos, TextColor, TextTransform );
  }

  DrawBorder( API );
}

CWBButton::CWBButton( CWBItem *Parent, const CRect &Pos, const TCHAR *Txt ) : CWBItem()
{
  Initialize( Parent, Pos, Txt );
}

CWBButton::~CWBButton()
{

}

TBOOL CWBButton::Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *Txt )
{
  Pushed = false;
  Text = Txt;

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff2d2d30 ) );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff3e3e40 ) );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff007acc ) );

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_DISABLED, WB_ITEM_FONTCOLOR, CColor::FromARGB( 0xff656565 ) );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_DISABLED_ACTIVE, WB_ITEM_FONTCOLOR, CColor::FromARGB( 0xff656565 ) );

  //rgbBackground = CColor::FromARGB( 0xff2d2d30 );
  //rgbBackgroundHover = CColor::FromARGB( 0xff3e3e40 );
  //rgbBackgroundActive = CColor::FromARGB( 0xff007acc );
  //TextAlignment = WBTA_CENTERX;

  ContentChanged();
  return true;
}

TBOOL CWBButton::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
  case WBM_RIGHTBUTTONDOWN:
  case WBM_MIDDLEBUTTONDOWN:
    if ( IsEnabled() )
      App->SetCapture( this );
    return true;

  case WBM_LEFTBUTTONUP:
    App->ReleaseCapture();
    if ( App->GetMouseItem() == this && IsEnabled() )
      App->SendMessage( CWBMessage( App, WBM_COMMAND, GetGuid() ) );
    return true;

  case WBM_RIGHTBUTTONUP:
    App->ReleaseCapture();
    return true;

  case WBM_MIDDLEBUTTONUP:
    App->ReleaseCapture();
    return true;

    // 	default:
    // 		return false;
  }

  return CWBItem::MessageProc( Message );
}

CSize CWBButton::GetContentSize()
{
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  if ( !Font ) return CSize( 0, 0 );
  return CSize( Font->GetWidth( Text, false, TextTransform ), Font->GetLineHeight() );
}

//void CWBButton::ResizeToContentSize(TBOOL Width, TBOOL Height, TS32 AddedWidth, TS32 AddedHeight)
//{
//	CSize s = GetContentSize();
//
//	TS32 width = GetPosition().Width();
//	TS32 height = GetPosition().Height();
//
//	if (Width) width = s.x + AddedWidth;
//	if (Height) height = s.y + AddedHeight;
//
//	CRect r = GetPosition();
//	SetPosition(CRect(r.TopLeft(), r.TopLeft() + CPoint(width, height)));
//}

void CWBButton::Push( TBOOL pushed )
{
  Pushed = pushed;
}

void CWBButton::SetText( CString val )
{
  Text = val;
  ContentChanged();
}

CString CWBButton::GetText() const
{
  return Text;
}

TBOOL CWBButton::IsPushed()
{
  return Pushed;
}

CWBItem * CWBButton::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CWBButton * button = new CWBButton( Root, Pos );
  if ( node.HasAttribute( _T( "text" ) ) )
    button->SetText( node.GetAttribute( _T( "text" ) ) );
  return button;
}
