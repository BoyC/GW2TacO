#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include <thread>

struct TransactionItem
{
  TS32 transactionID = 0;
  TS32 itemID = 0;
  TS32 price = 0;
  TS32 quantity = 0;
};

struct GW2ItemData
{
  TS32 itemID = 0;
  CString name;
  WBATLASHANDLE icon = 0;
  TS32 buyPrice = 0;
  TS32 sellPrice = 0;
};

extern CDictionary<TS32, GW2ItemData> itemDataCache;

class TPTracker : public CWBItem
{
  //CPoint lastpos;
  virtual void OnDraw( CWBDrawAPI* API );

  bool beingFetched = false;
  TS32 lastFetchTime = 0;

  //bool hasFullDungeonInfo = false;

  std::thread fetchThread;

  //CArray<Dungeon> dungeons;

  CArray<TransactionItem> buys;
  CArray<TransactionItem> sells;
  static TBOOL ParseTransaction( jsonxx::Object& object, TransactionItem& output );

  LIGHTWEIGHT_CRITICALSECTION dataWriteCritSec;

public:

  TPTracker( CWBItem* Parent, CRect Position );
  virtual ~TPTracker();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "tptracker" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );
};


