#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class GW2MapTimer : public CWBItem
{
  struct Event
  {
    CString name;
    CString waypoint;
    CString worldBossId;
    int length;
    int start;
    CColor color;
  };

  struct Map
  {
    CString name;
    CString chestId;
    int Length;
    int Start;
    CString id;
    TBOOL display = true;
    CArray<Event> events;
  };

  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI* API );
  void SetLayout( CXMLNode& node );

  bool beingFetched = false;
  TS32 lastFetchTime = 0;

  bool hasWorldBossInfo = false;

  std::thread fetchThread;

  CArray<CString> worldBosses;
  CArray<CString> mapchests;

  LIGHTWEIGHT_CRITICALSECTION critSec;

public:

  CArray<Map> maps;

  GW2MapTimer( CWBItem* Parent, CRect Position );
  virtual ~GW2MapTimer();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "maptimer" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );
};


