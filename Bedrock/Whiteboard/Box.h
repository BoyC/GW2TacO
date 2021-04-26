#pragma once
#include "Application.h"

enum WBBOXAXIS
{
  WB_HORIZONTAL = 0,
  WB_VERTICAL,
};

enum WBBOXARRANGEMENT
{
  WB_ARRANGE_NONE = 0,
  WB_ARRANGE_HORIZONTAL,
  WB_ARRANGE_VERTICAL
};

enum WBBOXSIZING
{
  WB_SIZING_KEEP = 0,
  WB_SIZING_FILL,
};

class CWBBox : public CWBItem
{
protected:

  virtual void AddChild( CWBItem *Item );
  virtual TBOOL MessageProc( CWBMessage &Message );
  virtual void RearrangeChildren();

  virtual void OnDraw( CWBDrawAPI *API );
  void RearrangeHorizontal();
  void RearrangeVertical();
  void UpdateScrollbarData();

  TS32 Spacing;
  WBBOXARRANGEMENT Arrangement;
  WBALIGNMENT AlignmentX, AlignmentY;
  WBBOXSIZING SizingX, SizingY;
  bool ClickThrough = false;

public:

  CWBBox();
  CWBBox( CWBItem *Parent, const CRect &Pos );
  virtual ~CWBBox();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position );

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "box" ), CWBItem );

  virtual void SetArrangement( WBBOXARRANGEMENT a );
  WBBOXARRANGEMENT GetArrangement();
  virtual void SetSpacing( TS32 s );
  virtual void SetAlignment( WBBOXAXIS axis, WBALIGNMENT align );
  virtual void SetSizing( WBBOXAXIS axis, WBBOXSIZING siz );
  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray & pseudo );
  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );

};