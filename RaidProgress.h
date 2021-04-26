#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include <thread>

class RaidEvent
{
public:
  CString name;
  CString type;
  bool finished = false;
};

class Wing
{
public:
  CString name;
  CArray<RaidEvent> events;
};

class Raid
{
public:
  CString name;
  CString shortName;
  CString configName;
  CArray<Wing> wings;
};

class RaidProgress : public CWBItem
{
  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI *API );

  bool beingFetched = false;
  TS32 lastFetchTime = 0;

  bool hasFullRaidInfo = false;

  std::thread fetchThread;

  CArray<Raid> raids;

public:

  RaidProgress( CWBItem *Parent, CRect Position );
  virtual ~RaidProgress();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "raidprogress" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
  CArray<Raid>& GetRaids();
};


