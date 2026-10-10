#pragma once
#include "GuiItem.h"

#define WB_CONTEXT_SEPARATOR ((void*)(INT_MIN))

class CWBContextItem
{
  friend class CWBContextMenu;

  CString Text;
  TS32 ReturnID;
  TBOOL Separator;
  TBOOL Highlighted;
  TBOOL closesContext;
  TS32 Check = -1; // -1: no check box, 0: unchecked, 1: checked

  CArray<CWBContextItem*> Children;
  CWBContextItem* CopyOf = nullptr;

  void CopyChildrenFrom( CWBContextItem *itm );
  void ClearCopyLinks();

public:

  CWBContextItem();
  virtual ~CWBContextItem();
  virtual CWBContextItem *AddItem( const TCHAR *Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true, TS32 Check = -1 );
  virtual CWBContextItem *AddItem( const CString& Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true, TS32 Check = -1 );
  virtual void AddSeparator();
  virtual void SetText( const CString& text );
  virtual void SetHighlight( TBOOL highlighted );
  virtual void SetCheck( TS32 check );
};

class CWBContextMenu : public CWBItem
{
  TBOOL Pushed; //used to ignore mouse clicks originating from the opening item
  WBGUID Target;

  CWBContextMenu *SubMenu;
  CWBContextMenu *ParentMenu;
  CArray<CWBContextItem*> Items;

  TBOOL MessageProc( CWBMessage &Message );
  virtual void ResizeToContentSize();
  virtual void OnDraw( CWBDrawAPI *API );
  CRect GetItemRect( TS32 idx );
  TS32 GetEntryHeight( CWBFont *Font );
  TS32 GetCheckColumnWidth( CWBFont *Font );
  void MarkParentForDeletion();

  TBOOL MouseInContextHierarchy();
  CWBContextMenu *GetContextRoot();
  virtual TBOOL AllowMouseHighlightWhileCaptureItem() { return true; }

  CWBCSSPropertyBatch SeparatorElements;

  int spawnedFromIdx = -1;
  TS32 entryPadding = 0; // extra entry height, dropped when the menu would not fit on the screen with it

public:

  CWBContextMenu();
  CWBContextMenu( CWBItem *Parent, const CRect &Pos, WBGUID Target );
  virtual ~CWBContextMenu();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, WBGUID Target );

  WB_DECLARE_GUIITEM( _T( "contextmenu" ), CWBItem );

  virtual CWBContextItem *AddItem( const TCHAR *Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true, TS32 Check = -1 );
  virtual CWBContextItem *AddItem( const CString &Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true, TS32 Check = -1 );
  virtual void AddSeparator();
  void FlushItems();
  void SpawnSubMenu( TS32 itemidx );

  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );
  virtual CWBContextItem *GetItem( TS32 ID );

  CWBContextMenu* GetParentMenu();
};