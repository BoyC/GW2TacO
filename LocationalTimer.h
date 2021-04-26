#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class LocationalTimer
{
public:

  struct TimerEvent
  {
    CString Text;
    TS32 Time;
    TS32 CountdownLength;
    TS32 OnScreenLength;
    CArray<CVector3> PolyPoints;
    CColor PolyColor = CColor( 255, 0, 0, 255 );
  };

  struct TimerPhase
  {
    CArray<TimerEvent> Events;
  };

  TS32 MapID = 0;
  CSphere EnterSphere = CSphere( CVector3( 0, 0, 0 ), 0 );
  CSphere ExitSphere = CSphere( CVector3( 0, 0, 0 ), 0 );
  CVector3 ResetPoint = CVector3( 0, 0, 0 );
  TS32 TimerLength = 0;
  TS32 StartDelay = 0;

  CArray<TimerEvent> Events;

  TBOOL IsRunning = false;
  TS32 StartTime = 0;

  LocationalTimer();
  virtual ~LocationalTimer();

  void Update();
  void ImportData( CXMLNode &node );

};

class TimerDisplay : public CWBItem
{
public:

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );

  TimerDisplay( CWBItem *Parent, CRect Position );
  virtual ~TimerDisplay();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "TimerDisplay" ), CWBItem );
};

extern CArray<LocationalTimer> LocationalTimers;
void ImportLocationalTimers();