#pragma once
#include "Application.h"

#define WB_WINDOW_CLOSEABLE				0x00000001
#define WB_WINDOW_MOVEABLE				0x00000002
#define WB_WINDOW_RESIZABLE				0x00000004
#define WB_WINDOW_ALWAYSONTOP			0x00000008
#define WB_WINDOW_TITLE					0x00000010

#define WB_WINDOW_DEFAULT (WB_WINDOW_CLOSEABLE | WB_WINDOW_RESIZABLE | WB_WINDOW_MOVEABLE | WB_WINDOW_TITLE)

#define WB_DRAGMODE_TOP					0x00000001
#define WB_DRAGMODE_BOTTOM				0x00000002
#define WB_DRAGMODE_LEFT				0x00000004
#define WB_DRAGMODE_RIGHT				0x00000008

#define WB_DRAGMASK						0x0000000F

#define WB_DRAGMODE_CLOSEBUTTON			0x00000010
#define WB_DRAGMODE_MOVE				0x00000020

enum WBWINDOWELEMENT
{
  WB_WINELEMENT_CLOSE = 0,
  WB_WINELEMENT_MINIMIZE,
  WB_WINELEMENT_INFO,
  WB_WINELEMENT_TITLE,
};

class CWBWindow : public CWBItem
{
  TS32 BorderWidth;
  TS32 TitleBarHeight;
  TS32 CornerSelectionSize;

  CSize MinSize;

  CString WindowTitle;

  CDictionary<WBWINDOWELEMENT, CWBCSSPropertyBatch> Elements;

protected:

  TU32 Style;
  TU32 DragMode;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CWBWindow();
  CWBWindow( CWBItem *Parent, const CRect &Pos, const TCHAR *txt = _T( "" ), TU32 style = WB_WINDOW_DEFAULT );
  virtual ~CWBWindow();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *txt = _T( "" ), TU32 style = WB_WINDOW_DEFAULT );
  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );

  CString GetTitle() const { return WindowTitle; }
  void SetTitle( CString val ) { WindowTitle = val; }

  TU32 GetDragMode();
  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );

  CRect GetElementPos( WBWINDOWELEMENT Element );
  TU32 GetBorderSelectionArea( CPoint &mousepos );

  WB_DECLARE_GUIITEM( _T( "window" ), CWBItem );
};