#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class OverlayWindow : public CWBWindow
{

public:

  virtual TBOOL MessageProc( CWBMessage &Message );
  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );

  OverlayWindow( CWBItem *Parent, CRect Position );
  virtual ~OverlayWindow();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "OverlayWindow" ), CWBItem );
};