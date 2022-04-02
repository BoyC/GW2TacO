#include "BasePCH.h"
#include "DropDown.h"

void CWBDropDown::OnDraw( CWBDrawAPI *API )
{
  WBITEMSTATE i = GetState();
  if ( i == WB_STATE_ACTIVE ) i = WB_STATE_HOVER;
  if ( i == WB_STATE_HOVER && !MouseOver() ) i = WB_STATE_NORMAL;
  if ( App->FindItemByGuid( ContextGuid ) ) i = WB_STATE_ACTIVE; //active if context menu is open

  DrawBackground( API, i );
  API->SetCropToClient( this );

  CWBSelectableItem *item = GetCursorItem();

  if ( item && item->IsSelected() )
  {
    //draw text

    CWBFont *Font = GetFont( i );
    WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
    CColor TextColor = CSSProperties.DisplayDescriptor.GetColor( i, WB_ITEM_FONTCOLOR );

    CRect r = GetClientRect();
    CPoint p = Font->GetCenter( item->GetText(), r, TextTransform );

    Font->Write( API, item->GetText(), p, TextColor, TextTransform, true );

  }

  DrawBorder( API );
}

CWBDropDown::CWBDropDown() : CWBItemSelector()
{
}

CWBDropDown::CWBDropDown( CWBItem *Parent, const CRect &Pos ) : CWBItemSelector()
{
  Initialize( Parent, Pos );
}

CWBDropDown::~CWBDropDown()
{

}

TBOOL CWBDropDown::Initialize( CWBItem *Parent, const CRect &Position )
{
  if ( !CWBItemSelector::Initialize( Parent, Position ) ) return false;

  ContextGuid = 0;

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff2d2d30 ) );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff3e3e40 ) );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff007acc ) );

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_DISABLED, WB_ITEM_FONTCOLOR, CColor::FromARGB( 0xff656565 ) );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_DISABLED_ACTIVE, WB_ITEM_FONTCOLOR, CColor::FromARGB( 0xff656565 ) );


  ApplyStyleDeclarations( _T( "padding:2px;" ) );
  return true;
}

TBOOL CWBDropDown::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
    if ( App->GetMouseItem() == this )
    {
      //open contextmenu

      if ( !List.NumItems() ) return true;
      CWBContextMenu *ctx = OpenContextMenu( ClientToScreen( GetWindowRect().BottomLeft() ) );
      ContextGuid = ctx->GetGuid();

      for ( TS32 x = 0; x < List.NumItems(); x++ )
        ctx->AddItem( List[ x ].GetText(), List[ x ].GetID() );

    }
    return true;

  case WBM_MOUSEWHEEL:
  {
    if ( App->GetMouseItem() == this )
    {
      if ( !List.NumItems() ) return true;
      if ( CursorPosition < 0 && Message.Data >= 0 ) return true;
      TS32 cp = min( List.NumItems() - 1, max( 0, CursorPosition - Message.Data ) );
      SelectItemByIndex( cp );
      return true;
    }
  }
  return true;

  case WBM_CONTEXTMESSAGE:
  {
    if ( Message.GetTarget() == GetGuid() )
    {
      SelectItem( Message.Data );
      return true;
    }
  }
  break;

  case WBM_KEYDOWN:
    if ( !InFocus() ) break;

    switch ( Message.Key )
    {
    case VK_UP:
    {
      if ( CursorPosition > 0 ) SelectItem( List[ CursorPosition - 1 ].GetID() );
      return true;
    }
    break;
    case VK_DOWN:
    {
      if ( CursorPosition < List.NumItems() - 1 ) SelectItem( List[ CursorPosition + 1 ].GetID() );
      return true;
    }
    break;
    }
    break;
  }

  return CWBItemSelector::MessageProc( Message );
}

CWBItem * CWBDropDown::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CWBDropDown * list = new CWBDropDown( Root, Pos );
  return list;
}
