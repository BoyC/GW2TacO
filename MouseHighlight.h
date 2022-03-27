#pragma once

#include "Bedrock/WhiteBoard/whiteboard.h"

enum class MouseColor
{
  red, lightred,
  black, gray,
  blue, lightblue,
  green, lightgreen,
  cyan, lightcyan,
  magenta, lightmagenta,
  brown, yellow,
  lightgray, white
};

class GW2MouseHighlight : public CWBItem
{

  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI* API );

public:

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );

  GW2MouseHighlight( CWBItem* Parent, CRect Position );
  virtual ~GW2MouseHighlight();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "mousehighlight" ), CWBItem );


  CPoint lastchangedpos;
  int numSameFrames = 0;

};

