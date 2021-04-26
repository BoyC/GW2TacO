#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class TS3Control : public CWBItem
{
  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI *API );

public:

  TS3Control( CWBItem *Parent, CRect Position );
  virtual ~TS3Control();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "ts3control" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
};


