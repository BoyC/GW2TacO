#include "BasePCH.h"
#include "ItemSelector.h"

//globally unique item IDs for automatically generated item IDs
TS32 wbItemIDCounter = 0;

//////////////////////////////////////////////////////////////////////////
// CWBSelectableItem

CWBSelectableItem::CWBSelectableItem()
{
  Selected = false;
  ID = 0;
}

CWBSelectableItem::CWBSelectableItem( const CString &text, SELECTABLEID id, TBOOL selected )
{
  Selected = selected;
  Text = text;
  ID = id;
}

CString &CWBSelectableItem::GetText()
{
  return Text;
}

SELECTABLEID CWBSelectableItem::GetID()
{
  return ID;
}

TBOOL CWBSelectableItem::IsSelected()
{
  return Selected;
}

TBOOL CWBSelectableItem::IsColorSet()
{
  return ColorSet;
}

CColor CWBSelectableItem::GetColor()
{
  return Color;
}

void CWBSelectableItem::SetColor( CColor col )
{
  ColorSet = true;
  Color = col;
}

void CWBSelectableItem::SetText( CString &text )
{
  Text = text;
}

void CWBSelectableItem::SetID( SELECTABLEID id )
{
  ID = id;
}

void CWBSelectableItem::Select( TBOOL selected )
{
  Selected = selected;
}

//////////////////////////////////////////////////////////////////////////
// CWBItemSelector

CWBItemSelector::CWBItemSelector() : CWBItem()
{
}

CWBItemSelector::CWBItemSelector( CWBItem *Parent, const CRect &Pos ) : CWBItem()
{
  Initialize( Parent, Pos );
}

CWBItemSelector::~CWBItemSelector()
{

}

TBOOL CWBItemSelector::Initialize( CWBItem *Parent, const CRect &Position )
{
  CursorPosition = -1;
  AnchorPosition = -1;

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;
  return true;
}

TBOOL CWBItemSelector::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_SELECTITEM:
    App->SendMessage( CWBMessage( App, WBM_ITEMSELECTED, GetGuid(), Message.Data ) );
    return true;
  default:
    break;
  }

  return CWBItem::MessageProc( Message );
}

SELECTABLEID CWBItemSelector::AddItem( const CString &Text )
{
  List += CWBSelectableItem( Text, wbItemIDCounter++, false );
  return List.Last().GetID();
}

SELECTABLEID CWBItemSelector::AddItem( const TCHAR *Text )
{
  return AddItem( CString( Text ) );
}

SELECTABLEID CWBItemSelector::AddItem( const CString &Text, SELECTABLEID ID )
{
  List += CWBSelectableItem( Text, wbItemIDCounter++, false );
  List.Last().SetID( ID );
  return List.Last().GetID();
}

SELECTABLEID CWBItemSelector::AddItem( const TCHAR *Text, SELECTABLEID ID )
{
  return AddItem( CString( Text ), ID );
}

TBOOL CWBItemSelector::DeleteItem( SELECTABLEID ID )
{
  for ( TS32 x = 0; x < List.NumItems(); x++ )
  {
    if ( List[ x ].GetID() == ID )
    {
      CursorPosition = -2;

      List.DeleteByIndex( x );
      if ( List.NumItems() )
        SelectItemByIndex( max( 0, min( List.NumItems() - 1, x ) ) );
      else
        SelectItemByIndex( -1 );

      //SetVScrollbarParameters(0, List.NumItems()*FixItemHeight, GetClientRect().Height());
      return true;
    }
  }

  return false;
}

void CWBItemSelector::Flush()
{
  List.Flush();
}

CWBSelectableItem * CWBItemSelector::GetItem( SELECTABLEID ID )
{
  for ( TS32 x = 0; x < List.NumItems(); x++ )
    if ( List[ x ].GetID() == ID ) return &List[ x ];
  return NULL;
}

//TBOOL CWBItemSelector::DeleteItem(SELECTABLEID ID)
//{
//}

TS32 CWBItemSelector::NumItems()
{
  return List.NumItems();
}

CWBSelectableItem &CWBItemSelector::GetItemByIndex( TS32 x )
{
  return List[ x ];
}

TS32 CWBItemSelector::GetItemIndex( SELECTABLEID ItemID )
{
  for ( TS32 x = 0; x < List.NumItems(); x++ )
    if ( List[ x ].GetID() == ItemID ) return x;
  return -1;
}

TS32 CWBItemSelector::NumSelected()
{
  TS32 Count = 0;
  for ( TS32 x = 0; x < List.NumItems(); x++ )
    if ( List[ x ].IsSelected() ) Count++;
  return Count;
}

TS32 CWBItemSelector::GetSelectedIndex( TS32 Idx )
{
  TS32 Count = 0;
  for ( TS32 x = 0; x < List.NumItems(); x++ )
    if ( List[ x ].IsSelected() )
    {
      if ( Count == Idx ) return x;
      Count++;
    }
  return -1;
}

CArray<TS32> CWBItemSelector::GetSelectedIndices()
{
  CArray<TS32> Dic;
  for ( TS32 x = 0; x < List.NumItems(); x++ )
    if ( List[ x ].IsSelected() ) Dic += x;
  return Dic;
}

TS32 CWBItemSelector::GetCursorPosition()
{
  return CursorPosition;
}

void CWBItemSelector::SelectItem( SELECTABLEID ItemID )
{
  TS32 idx = GetItemIndex( ItemID );
  if ( idx >= 0 ) SelectItemByIndex( idx );
}

void CWBItemSelector::SelectItemByIndex( TS32 Idx ) //single select implementation only
{
  //LOG_DBG("[itemselector] %s selecting idx %d", GetID().GetPointer(), Idx);

  //if (CursorPosition == Idx) return;

  //clear selection
  for ( TS32 x = 0; x < List.NumItems(); x++ )
    List[ x ].Select( false );

  CursorPosition = AnchorPosition = Idx;

  if ( CursorPosition >= 0 && CursorPosition < List.NumItems() )
  {
    List[ CursorPosition ].Select( true );
    App->SendMessage( CWBMessage( App, WBM_SELECTITEM, GetGuid(), List[ CursorPosition ].GetID() ) );
    //LOG_DBG("[itemselector] %s got item idx %d (%d) selected", GetID().GetPointer(), Idx, List[CursorPosition].GetID());
  }

  App->SendMessage( CWBMessage( App, WBM_SELECTIONCHANGE, GetGuid(), 0 ) );
}

SELECTABLEID CWBItemSelector::GetCursorItemID()
{
  if ( CursorPosition < 0 || CursorPosition >= List.NumItems() ) return -1;
  return List[ CursorPosition ].GetID();
}

CWBSelectableItem *CWBItemSelector::GetCursorItem()
{
  if ( CursorPosition < 0 || CursorPosition >= List.NumItems() ) return NULL;
  return &List[ CursorPosition ];
}

void CWBItemSelector::SetItemColor( SELECTABLEID ID, CColor color )
{
  auto *item = GetItem( ID );
  if ( !item )
    return;
  item->SetColor( color );
}

