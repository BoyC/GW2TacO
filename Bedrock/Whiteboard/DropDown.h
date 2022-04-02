#pragma once
#include "Application.h"
#include "ItemSelector.h"

class CWBDropDown : public CWBItemSelector
{
protected:

  WBGUID ContextGuid;

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );

public:

  CWBDropDown();
  CWBDropDown( CWBItem *Parent, const CRect &Pos );
  virtual ~CWBDropDown();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position );
  //virtual TBOOL ApplyStyle(CString & prop, CString & value, CStringArray &pseudo);

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "dropdown" ), CWBItemSelector );
};