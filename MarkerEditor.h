#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include "gw2tactical.h"

class GW2MarkerEditor : public CWBItem
{
  virtual TBOOL MessageProc( CWBMessage &Message );
  virtual void OnDraw( CWBDrawAPI *API );
  TBOOL Hidden = false;
  GUID CurrentPOI;

  CArray<GW2TacticalCategory*> CategoryList;
  TBOOL ChangeDefault = false;

public:

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );

  GW2MarkerEditor( CWBItem *Parent, CRect Position );
  virtual ~GW2MarkerEditor();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "markereditor" ), CWBItem );
};