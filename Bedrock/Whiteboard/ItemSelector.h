#pragma once
#include "Application.h"

typedef TS32 SELECTABLEID;

class CWBSelectableItem
{
  CString Text;
  TBOOL Selected;
  SELECTABLEID ID;
  TBOOL ColorSet = false;
  CColor Color;

public:

  CWBSelectableItem();
  CWBSelectableItem( const CString &Text, SELECTABLEID ID, TBOOL Selected = false );

  virtual void SetText( CString &Text );
  virtual void SetID( SELECTABLEID ID );
  virtual void Select( TBOOL Selected );

  virtual CString &GetText();
  virtual SELECTABLEID GetID();
  virtual TBOOL IsSelected();
  virtual TBOOL IsColorSet();
  virtual CColor GetColor();
  virtual void SetColor( CColor col );

};

class CWBItemSelector : public CWBItem
{

protected:

  CArray<CWBSelectableItem> List;
  TBOOL MessageProc( CWBMessage &Message );

  TS32 CursorPosition; //position of the cursor - always selected for single-select, may or may not be selected for multi-select
  TS32 AnchorPosition; //reference point for multi-select: last item where selected got turned to true without multi-select

public:

  CWBItemSelector();
  CWBItemSelector( CWBItem *Parent, const CRect &Pos );
  virtual ~CWBItemSelector();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position );

  WB_DECLARE_GUIITEM( _T( "itemselector" ), CWBItem );

  virtual SELECTABLEID AddItem( const CString &Text );
  virtual SELECTABLEID AddItem( const TCHAR *Text );

  virtual SELECTABLEID AddItem( const CString &Text, SELECTABLEID ID );
  virtual SELECTABLEID AddItem( const TCHAR *Text, SELECTABLEID ID );

  virtual TBOOL DeleteItem( SELECTABLEID ID );

  virtual void Flush();

  virtual CWBSelectableItem *GetItem( SELECTABLEID ID );
  virtual TS32 NumItems();
  virtual CWBSelectableItem &GetItemByIndex( TS32 x );
  virtual TS32 GetItemIndex( SELECTABLEID ItemID );

  virtual TS32 NumSelected(); //Get the number of selected items
  virtual TS32 GetSelectedIndex( TS32 Idx ); //Get the index of the Nth selected item
  virtual CArray<TS32> GetSelectedIndices(); //Get an array with the indices of the selected items

  virtual void SelectItem( SELECTABLEID ItemID );
  virtual void SelectItemByIndex( TS32 Idx, bool dontSendMessage = false );

  virtual TS32 GetCursorPosition();
  virtual SELECTABLEID GetCursorItemID();
  virtual CWBSelectableItem *GetCursorItem();
  virtual void SetItemColor( SELECTABLEID ID, CColor color );
};