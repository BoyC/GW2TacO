#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class ClickThroughButton : public CWBButton
{
  //virtual TBOOL MessageProc( CWBMessage &Message );

public:

  ClickThroughButton( CWBItem *Parent, const CRect &Pos, const TCHAR *txt = _T( "" ) );
  virtual ~ClickThroughButton();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, const TCHAR *txt = _T( "" ) );

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "clickthroughbutton" ), CWBItem );
};