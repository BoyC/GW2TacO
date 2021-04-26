#pragma once

#include "DrawAPI.h"
#include "CSSItem.h"
#include "Skin.h"

typedef TU32 WBGUID;
class CWBDrawAPI;
class CWBApplication;
class CWBMessage;
enum WBMESSAGE;

#define WBMARGIN_KEEP (INT_MAX)
#define POSSIZETORECT(a,b) (CRect(a.x,a.y,a.x+b.x,a.y+b.y))

class CWBContextMenu;

enum WBALIGNMENT
{
  WB_ALIGN_TOP = 0,
  WB_ALIGN_LEFT = 0,
  WB_ALIGN_CENTER = 1,
  WB_ALIGN_MIDDLE = 1,
  WB_ALIGN_RIGHT = 2,
  WB_ALIGN_BOTTOM = 2,
};

enum WBITEMSTATE
{
  WB_STATE_NORMAL = 0,
  WB_STATE_ACTIVE = 1,
  WB_STATE_HOVER = 2,
  WB_STATE_DISABLED = 3,
  WB_STATE_DISABLED_ACTIVE = 4,

  WB_STATE_COUNT, //don't remove this, used as array size
};

enum WBITEMVISUALCOMPONENT
{
  WB_ITEM_BACKGROUNDCOLOR = 0,
  WB_ITEM_FOREGROUNDCOLOR,
  WB_ITEM_BORDERCOLOR,
  WB_ITEM_FONTCOLOR,
  WB_ITEM_BACKGROUNDIMAGE,
  WB_ITEM_SUBSKIN,
  //WB_ITEM_SELECTIONCOLOR,
  //WB_ITEM_SELECTIONFONTCOLOR,
  WB_ITEM_OPACITY,
  WB_ITEM_BACKGROUNDALIGNMENT_X,
  WB_ITEM_BACKGROUNDALIGNMENT_Y,
  WB_ITEM_TEXTTRANSFORM,

  //scrollbar stuff
  WB_ITEM_SCROLL_UP,
  WB_ITEM_SCROLL_DOWN,
  WB_ITEM_SCROLL_LEFT,
  WB_ITEM_SCROLL_RIGHT,
  WB_ITEM_SCROLL_HBAR,
  WB_ITEM_SCROLL_VBAR,
  WB_ITEM_SCROLL_HTHUMB,
  WB_ITEM_SCROLL_VTHUMB,

  WB_ITEM_COUNT, //don't remove this, used as array size
};

enum WBSCROLLDRAGMODE
{
  WB_SCROLLDRAG_NONE = 0,
  WB_SCROLLDRAG_BUTTON1,
  WB_SCROLLDRAG_UP,
  WB_SCROLLDRAG_THUMB,
  WB_SCROLLDRAG_DOWN,
  WB_SCROLLDRAG_BUTTON2,
};

class CWBScrollbarParams
{
public:
  TBOOL Enabled; //determines whether the scrollbar will be displayed at all
  TBOOL Dynamic; //if true the scrollbar disappears when not needed
  TBOOL Visible; //if true the client area has been adjusted so the scrollbar can fit

  //position data
  TS32 MinScroll, MaxScroll;
  TS32 ScrollPos;
  TS32 ViewSize;

  //dragging data
  WBSCROLLDRAGMODE Dragmode;
  TS32 DragStartPosition;

  CWBScrollbarParams()
  {
    DragStartPosition = 0;
    Dragmode = WB_SCROLLDRAG_NONE;
    Visible = Dynamic = Enabled = false;
    MinScroll = MaxScroll = ScrollPos = 0;
    ViewSize = 0;
  }
};

class CWBDisplayState
{
  TS32 Visuals[ WB_ITEM_COUNT ];
  TBOOL VisualSet[ WB_ITEM_COUNT ];
  //CDictionary<WBITEMVISUALCOMPONENT,TS32> Visuals;

public:

  CWBDisplayState();
  virtual ~CWBDisplayState();

  TBOOL IsSet( WBITEMVISUALCOMPONENT v );
  CColor GetColor( WBITEMVISUALCOMPONENT v );
  WBSKINELEMENTID GetSkin( WBITEMVISUALCOMPONENT v );
  void SetValue( WBITEMVISUALCOMPONENT v, TS32 value );
  TS32 GetValue( WBITEMVISUALCOMPONENT v );
};

class CWBDisplayProperties
{
  CWBDisplayState States[ WB_STATE_COUNT ];
  //CDictionary<WBITEMSTATE,CWBDisplayState> States;

public:

  CWBDisplayProperties();
  virtual ~CWBDisplayProperties();

  CColor GetColor( WBITEMSTATE s, WBITEMVISUALCOMPONENT v );
  WBSKINELEMENTID GetSkin( WBITEMSTATE s, WBITEMVISUALCOMPONENT v );
  void SetValue( WBITEMSTATE s, WBITEMVISUALCOMPONENT v, TS32 value );
  TS32 GetValue( WBITEMSTATE s, WBITEMVISUALCOMPONENT v );
};

class CWBCSSPropertyBatch
{
public:
  CRect BorderSizes;
  WBTEXTALIGNMENTX TextAlignX;
  WBTEXTALIGNMENTY TextAlignY;
  CWBDisplayProperties DisplayDescriptor;
  CWBPositionDescriptor PositionDescriptor;
  CDictionary<WBITEMSTATE, CString> Fonts;

  CWBCSSPropertyBatch();
  virtual CWBFont *GetFont( CWBApplication *App, WBITEMSTATE State );
  TBOOL ApplyStyle( CWBItem *Owner, CString & prop, CString & value, CStringArray &pseudo );
};

class CWBItem : public IWBCSS
{
  friend CWBApplication; //so we don't directly expose the message handling functions to the user

  const WBGUID Guid;

  CRect Position; //stored in parent space
  CRect ClientRect; //stored in window space
  CRect ScreenRect; //calculated automatically, stores the position in screen space
  CRect StoredPosition;
  CPoint ContentOffset; //describes how much the content is moved relative to the item. used for easily sliding content around by scrollbars

  CSize StoredContentSize;

  CWBItem *Parent;
  CArray<CWBItem*> Children;

  TS32 SortLayer;
  TS32 ZIndex;
  TF32 OpacityMultiplier = 1;

  TBOOL Hidden;
  TBOOL Disabled;
  TBOOL ForceMouseTransparent = false;
  TS32 Scrollbar_Size;
  TS32 Scrollbar_ButtonSize;
  TS32 Scrollbar_ThumbMinimalSize;
  CWBScrollbarParams HScrollbar, VScrollbar;

  //////////////////////////////////////////////////////////////////////////
  // private functions

  void UpdateScreenRect();

  virtual void OnMove( const CPoint &p );
  virtual void OnResize( const CSize &s );
  virtual void OnMouseEnter();
  virtual void OnMouseLeave();

  virtual void CalculateClientPosition();

  void DrawTree( CWBDrawAPI *API );

  virtual TBOOL Focusable() const;

  void *Data;

  virtual void AdjustClientAreaToFitScrollbars();
  virtual void ScrollbarHelperFunct( CWBScrollbarParams &s, TS32 &r, TBOOL ScrollbarNeeded );
  virtual void ScrollbardisplayHelperFunct( CWBScrollbarParams &s, TS32 &a1, TS32 &a2, TS32 &thumbsize, TS32 &thumbpos );
  virtual TBOOL GetHScrollbarRectangles( CRect &button1, CRect &Scrollup, CRect &Thumb, CRect &Scrolldown, CRect &button2 ); //returns the highlight areas of the scrollbar in client space
  virtual TBOOL GetVScrollbarRectangles( CRect &button1, CRect &Scrollup, CRect &Thumb, CRect &Scrolldown, CRect &button2 );
  virtual TBOOL ScrollbarRequired( CWBScrollbarParams &s );
  virtual TS32 CalculateScrollbarMovement( CWBScrollbarParams &s, TS32 scrollbarsize, TS32 delta );
  virtual void DrawScrollbarButton( CWBDrawAPI *API, CWBScrollbarParams &s, CRect &r, WBITEMVISUALCOMPONENT Button );
  virtual void DrawHScrollbar( CWBDrawAPI *API );
  virtual void DrawVScrollbar( CWBDrawAPI *API );
  virtual void HandleHScrollbarClick( WBSCROLLDRAGMODE m );
  virtual void HandleVScrollbarClick( WBSCROLLDRAGMODE m );
  virtual TBOOL AllowMouseHighlightWhileCaptureItem() { return false; }

  TBOOL ScanPXValue( CString &Value, TS32 &Result, CString &PropName );
  TBOOL ScanSkinValue( CString &Value, WBSKINELEMENTID &Result, CString &PropName );

  CWBItem *ChildSearcherFunct( CString &value, CString &type = CString( _T( "" ) ) );

  WBITEMSTATE GetScrollbarState( WBITEMVISUALCOMPONENT Component, CRect r );
  virtual void ChangeContentOffset( CPoint ContentOff );

  static const CString &GetClassName()
  {
    static const CString type = _T( "guiitem" );
    return type;
  }

protected:
  CWBApplication *App;
  CWBItem *ChildInFocus;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual void OnPostDraw( CWBDrawAPI *API );

  //////////////////////////////////////////////////////////////////////////
  // CSS modifiable properties

  CWBCSSPropertyBatch CSSProperties;

  virtual CWBItem *GetItemUnderMouse( CPoint &Point, CRect &CropRect, WBMESSAGE MessageType );
  virtual void SetChildAsTopmost( TS32 Index );
  virtual void SetChildAsBottommost( TS32 Index );
  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );

  CWBItem *SetCapture();
  TBOOL ReleaseCapture() const;
  virtual void AddChild( CWBItem *Item );
  virtual TS32 GetChildIndex( CWBItem *Item );

  virtual TBOOL ScrollbarDragged();

  virtual void DrawBackgroundItem( CWBDrawAPI *API, CWBDisplayProperties &Descriptor, CRect &Pos, WBITEMSTATE i, WBITEMVISUALCOMPONENT v = WB_ITEM_BACKGROUNDIMAGE );
  virtual void DrawBackground( CWBDrawAPI *API, WBITEMSTATE State );
  virtual void DrawBackground( CWBDrawAPI *API );
  virtual void DrawBorder( CWBDrawAPI *API );
  virtual void ApplyOpacity( CWBDrawAPI *API );

  virtual void DrawBackground( CWBDrawAPI *API, CRect& rect, WBITEMSTATE State, CWBCSSPropertyBatch& cssProps );
  virtual void DrawBorder( CWBDrawAPI *API, CRect& rect, CWBCSSPropertyBatch& cssProps );

  virtual CStringArray ExplodeValueWithoutSplittingParameters( CString String );
  virtual TBOOL ParseRGBA( CString description, CColor &output );

  static void PositionApplicator( CWBPositionDescriptor &pos, WBPOSITIONTYPE Type, CString &Value );
  static void VisualStyleApplicator( CWBDisplayProperties &desc, WBITEMVISUALCOMPONENT TargetComponent, TS32 Value, CStringArray &pseudo );
  static void FontStyleApplicator( CWBCSSPropertyBatch &desc, CStringArray &pseudo, CString &name );

  //auto resize stuff
  virtual CSize GetContentSize();
  virtual void ContentChanged();
  virtual void ChangeContentOffsetX( TS32 OffsetX );
  virtual void ChangeContentOffsetY( TS32 OffsetY );
  TBOOL ScrollbarsEnabled();

  virtual CPoint GetContentOffset() { return ContentOffset; }

public:

  CWBItem();
  CWBItem( CWBItem *Parent, const CRect &Position );
  virtual ~CWBItem();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position );
  virtual TBOOL MessageProc( CWBMessage &Message ); //return true if this item handled the message
  TBOOL FindItemInParentTree( CWBItem *Item );

  INLINE const WBGUID GetGuid() const { return Guid; }
  INLINE CWBApplication *GetApplication() const { return App; }
  INLINE CWBItem *GetParent() const { return Parent; }

  virtual void SetParent( CWBItem* i );

  virtual CRect GetClientRect() const; //returns value in client space
  virtual CRect GetWindowRect() const; //returns value in client space
  virtual CRect GetScreenRect() const; //returns value in screen space

  virtual CPoint ScreenToClient( const CPoint &p ) const;
  virtual CRect ScreenToClient( const CRect &p ) const;
  virtual CPoint ClientToScreen( const CPoint &p ) const;
  virtual CRect ClientToScreen( const CRect &p ) const;

  virtual void SetPosition( const CRect &Pos );
  virtual void ApplyRelativePosition();
  virtual void ApplyPosition( const CRect &Pos ); //only to be used by the parent item when moving the item around
  virtual void SetClientPadding( TS32 left, TS32 top, TS32 right, TS32 bottom );

  TBOOL IsWidthSet(); //tells if the width has been specified in the style of the item
  TBOOL IsHeightSet(); //tells if the height has been specified in the style of the item
  TS32 GetCalculatedWidth( CSize ParentSize ); //tells if the width has been specified in the style of the item
  TS32 GetCalculatedHeight( CSize ParentSize ); //tells if the height has been specified in the style of the item

  CRect GetPosition();

  TU32 NumChildren();
  CWBItem * GetChild( TU32 idx );

  TBOOL InFocus();
  TBOOL InLocalFocus();
  void SetFocus();
  void ClearFocus();
  TBOOL MouseOver();
  virtual CWBItem *GetChildInFocus();

  void SavePosition();
  CRect GetSavedPosition() const;
  void SetSavedPosition( CRect& savedPos );

  void Hide( TBOOL Hide );
  TBOOL IsHidden();
  void Enable( TBOOL Enabled );
  TBOOL IsEnabled();

  void SetData( void *data );
  void *GetData();

  void MarkForDeletion( bool removeFromParent = false );

  virtual CWBContextMenu *OpenContextMenu( CPoint Position );

  virtual const CString &GetType() const
  {
    static const CString type = _T( "guiitem" );
    return type;
  }

  virtual TBOOL InstanceOf( const CString &name ) const
  {
    return name == GetClassName();
  }

  virtual void EnableHScrollbar( TBOOL Enabled, TBOOL Dynamic );
  virtual void EnableVScrollbar( TBOOL Enabled, TBOOL Dynamic );
  virtual TBOOL IsHScrollbarEnabled();
  virtual TBOOL IsVScrollbarEnabled();
  virtual void SetHScrollbarParameters( TS32 MinScroll, TS32 MaxScroll, TS32 ViewSize );
  virtual void SetVScrollbarParameters( TS32 MinScroll, TS32 MaxScroll, TS32 ViewSize );
  virtual void GetHScrollbarParameters( TS32 &MinScroll, TS32 &MaxScroll, TS32 &ViewSize );
  virtual void GetVScrollbarParameters( TS32 &MinScroll, TS32 &MaxScroll, TS32 &ViewSize );
  virtual void SetHScrollbarPos( TS32 ScrollPos, TBOOL Clamp = false );
  virtual void SetVScrollbarPos( TS32 ScrollPos, TBOOL Clamp = false );
  virtual TS32 GetHScrollbarPos() { return HScrollbar.ScrollPos; };
  virtual TS32 GetVScrollbarPos() { return VScrollbar.ScrollPos; };
  virtual void SetTopmost();
  virtual void SetBottommost();

  virtual void SetBorderSizes( TS8 Left, TS8 Top, TS8 Right, TS8 Bottom );
  virtual void SetFont( WBITEMSTATE State, CString &Font );

  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );

  CWBItem *FindChildByID( CString &value, CString &type = CString( _T( "" ) ) );
  CWBItem *FindChildByID( TCHAR *value, const TCHAR *type = NULL );
  CWBItem *FindChildByID( CString &value, const TCHAR *type = NULL );

  template< typename t > t* FindChildByID( CString &value )
  {
    CWBItem* it = FindChildByID( value, t::GetClassName() );
    if ( !it )
      return nullptr;
    return (t)it;
  }

  template< typename t > t* FindChildByID( TCHAR *value )
  {
    CWBItem* it = FindChildByID( value, t::GetClassName().GetPointer() );
    if ( !it )
      return nullptr;
    return (t*)it;
  }

  template< typename... Args>
  CWBItem *FindChildByIDs( TCHAR *value, Args ... args )
  {
    int len = sizeof...( Args );
    const TCHAR* vals[] = { args... };

    if ( !len )
      return FindChildByID( value );

    for ( int x = 0; x < len; x++ )
    {
      auto child = FindChildByID( value, vals[ x ] );
      if ( child )
        return child;
    }

    return nullptr;
  }

  CWBItem *FindParentByID( CString &value, CString &type = CString( _T( "" ) ) );
  CWBItem *FindParentByID( TCHAR *value, TCHAR *type = NULL );
  CWBItem *FindParentByID( CString &value, TCHAR *type = NULL );
  virtual void CalculateWindowPosition( const CSize &s );

  void BuildPositionMessage( const CRect &Pos, CWBMessage &m );
  void ApplyStyleDeclarations( const CString &String );

  virtual WBITEMSTATE GetState();
  void SetDisplayProperty( WBITEMSTATE s, WBITEMVISUALCOMPONENT v, TS32 value );
  CWBDisplayProperties &GetDisplayDescriptor() { return CSSProperties.DisplayDescriptor; }
  CWBCSSPropertyBatch &GetCSSProperties() { return CSSProperties; }

  virtual CWBFont *GetFont( WBITEMSTATE State );

  CWBPositionDescriptor &GetPositionDescriptor();
  CSize GetClientWindowSizeDifference();

  virtual void DeleteChildren();
  virtual void SetChildInFocus( CWBItem *i );

  virtual TBOOL InterpretPositionString( CWBCSSPropertyBatch &pos, CString & prop, CString & value, CStringArray &pseudo );
  virtual TBOOL InterpretDisplayString( CWBCSSPropertyBatch &desc, CString & prop, CString & value, CStringArray &pseudo );
  virtual TBOOL InterpretFontString( CWBCSSPropertyBatch &desc, CString & prop, CString & value, CStringArray &pseudo );

  virtual void SetTreeOpacityMultiplier( TF32 OpacityMul ); //for fading out whole subtrees
  virtual TF32 GetTreeOpacityMultiplier();

  virtual void ReapplyStyles();
  virtual void SetForcedMouseTransparency( TBOOL transparent );
  TBOOL MarkedForDeletion();
};

//OOP kung-fu follows to provide InstanceOf() functionality for use with CSS
//the initial virtual InstanceOf() call ensures we start at the bottom of the class hierarchy
//on each level we check if the static name of the current class is the typename we're comparing against
//if not we traverse up the hierarchy by directly calling the InstanceOf() of the parent class

#define WB_DECLARE_GUIITEM_1PARENTS(TYPE,PARENTCLASS) \
	virtual const CString &GetType() const \
{\
	static const CString type = TYPE;\
	return type;\
}\
	\
friend CWBItem;\
private:\
	static const CString &GetClassName()\
{\
	static const CString type = TYPE;\
	return type;\
}\
public:\
	\
	virtual TBOOL InstanceOf(const CString &name) const\
{\
	if (name == GetClassName()) return true;\
	return PARENTCLASS::InstanceOf(name);\
}

#define WB_DECLARE_GUIITEM_2PARENTS(TYPE,PARENTCLASS1,PARENTCLASS2) \
	virtual const CString &GetType() const \
{\
	static const CString type = TYPE;\
	return type;\
}\
	\
friend CWBItem;\
private:\
	static const CString &GetClassName()\
{\
	static const CString type = TYPE;\
	return type;\
}\
public:\
	\
	virtual TBOOL InstanceOf(const CString &name) const\
{\
	if (name == GetClassName()) return true;\
	return PARENTCLASS1::InstanceOf(name) || PARENTCLASS2::InstanceOf(name);\
}

#define WB_DECLARE_GUIITEM_3PARENTS(TYPE,PARENTCLASS1,PARENTCLASS2,PARENTCLASS3) \
	virtual const CString &GetType() const \
{\
	static const CString type = TYPE;\
	return type;\
}\
	\
friend CWBItem;\
private:\
	static const CString &GetClassName()\
{\
	static const CString type = TYPE;\
	return type;\
}\
public:\
	\
	virtual TBOOL InstanceOf(const CString &name) const\
{\
	if (name == GetClassName()) return true;\
	return PARENTCLASS1::InstanceOf(name) || PARENTCLASS2::InstanceOf(name) || PARENTCLASS3::InstanceOf(name);\
}

#define EXPAND(x) x
#define WB_DECLARE_MACRO_SELECTOR(_1, _2, _3, NAME, ...) NAME
#define WB_DECLARE_GUIITEM(TYPE,...) EXPAND(EXPAND(WB_DECLARE_MACRO_SELECTOR(__VA_ARGS__, WB_DECLARE_GUIITEM_3PARENTS, WB_DECLARE_GUIITEM_2PARENTS, WB_DECLARE_GUIITEM_1PARENTS))(TYPE,__VA_ARGS__))