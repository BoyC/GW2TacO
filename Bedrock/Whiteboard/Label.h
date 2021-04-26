#pragma once
#include "Application.h"

class CWBLabel : public CWBItem
{
  CString Text;
  virtual void OnDraw( CWBDrawAPI *API );

public:

  CWBLabel();
  CWBLabel( CWBItem *Parent, const CRect &Pos, const TCHAR *txt = _T( "" ) );
  virtual ~CWBLabel();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *txt = _T( "" ) );

  CString GetText() const { return Text; }
  void SetText( CString val );

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "label" ), CWBItem );

  TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType ) { return true; }
  virtual CSize GetContentSize();
};