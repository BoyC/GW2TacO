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

  CArray<CWBContextItem*> Children;
  CWBContextItem* CopyOf = nullptr;

  void CopyChildrenFrom( CWBContextItem *itm );

public:

  CWBContextItem();
  virtual ~CWBContextItem();
  virtual CWBContextItem *AddItem( const TCHAR *Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true );
  virtual CWBContextItem *AddItem( const CString& Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true );
  virtual void AddSeparator();
  virtual void SetText( const CString& text );
  virtual void SetHighlight( TBOOL highlighted );
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
  void SpawnSubMenu( TS32 itemidx );
  CRect GetItemRect( TS32 idx );
  void MarkParentForDeletion();

  TBOOL MouseInContextHierarchy();
  CWBContextMenu *GetContextRoot();
  virtual TBOOL AllowMouseHighlightWhileCaptureItem() { return true; }

  CWBCSSPropertyBatch SeparatorElements;

  int spawnedFromIdx = -1;

public:

  CWBContextMenu();
  CWBContextMenu( CWBItem *Parent, const CRect &Pos, WBGUID Target );
  virtual ~CWBContextMenu();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, WBGUID Target );

  WB_DECLARE_GUIITEM( _T( "contextmenu" ), CWBItem );

  virtual CWBContextItem *AddItem( const TCHAR *Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true );
  virtual CWBContextItem *AddItem( const CString &Text, TS32 ID, TBOOL Highlighted = false, TBOOL closesContext = true );
  virtual void AddSeparator();

  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );
  virtual CWBContextItem *GetItem( TS32 ID );
};