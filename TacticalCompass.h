#pragma once
#include "Bedrock/WhiteBoard/whiteboard.h"

class GW2TacticalCompass : public CWBItem
{
  virtual void OnDraw( CWBDrawAPI* API );
  void DrawTacticalCompass( CWBDrawAPI* API );

public:

  GW2TacticalCompass( CWBItem* Parent, CRect Position );
  virtual ~GW2TacticalCompass();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "gw2rangecircles" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );
};
