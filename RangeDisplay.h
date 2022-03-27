#pragma once
#include "Bedrock/WhiteBoard/whiteboard.h"

class GW2RangeDisplay : public CWBItem
{
  virtual void OnDraw( CWBDrawAPI* API );
  void DrawRangeCircle( CWBDrawAPI* API, float range, float alpha );

public:

  GW2RangeDisplay( CWBItem* Parent, CRect Position );
  virtual ~GW2RangeDisplay();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "gw2rangecircles" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );
};
