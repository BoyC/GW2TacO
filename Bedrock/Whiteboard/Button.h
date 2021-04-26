#pragma once
#include "Application.h"

class CWBButton : public CWBItem
{
  CString Text;
  TBOOL Pushed;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CWBButton( CWBItem *Parent, const CRect &Pos, const TCHAR *txt = _T( "" ) );
  virtual ~CWBButton();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *txt = _T( "" ) );

  CString GetText() const;
  void SetText( CString val );

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "button" ), CWBItem );

  virtual CSize GetContentSize();
  //virtual void ResizeToContentSize(TBOOL Width, TBOOL Height, TS32 AddedWidth, TS32 AddedHeight);

  virtual void Push( TBOOL pushed );
  virtual TBOOL IsPushed();

  virtual WBITEMSTATE GetState();

};