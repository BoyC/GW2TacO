#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include <thread>

class DungeonPath
{
public:
  CString name;
  CString type;
  bool finished = false;
  bool frequenter = false;
};

class Dungeon
{
public:
  CString name;
  CString shortName;
  CArray<DungeonPath> paths;
};

class DungeonProgress : public CWBItem
{
  CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI *API );

  bool beingFetched = false;
  TS32 lastFetchTime = 0;

  bool hasFullDungeonInfo = false;

  std::thread fetchThread;

  CArray<Dungeon> dungeons;

public:

  DungeonProgress( CWBItem *Parent, CRect Position );
  virtual ~DungeonProgress();

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "dungeonprogress" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType );
};


