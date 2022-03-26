#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "Bedrock/Whiteboard/WhiteBoard.h"
#include <thread>
#include <vector>
#include <string>
#include <set>

namespace GW2
{

class Festival
{
public:
  std::string name;
  std::vector<int> dailyAchievements;
};

struct FestivalData
{
  char* name{};
  int category{};
  bool active{};
};

extern FestivalData festivals[6];

extern std::vector<Festival> festivalDailies;

class APIKey
{
public:

  CString apiKey;
  CDictionary< CString, TBOOL > caps;
  CString keyName;
  CString accountName;
  CStringArray charNames;
  int worldId = 0;

  APIKey() = default;
  APIKey( const CString& key );
  virtual ~APIKey();

  void FetchData();
  TBOOL HasCaps( const CString& cap );

  CString QueryAPI( char* path );

  bool initialized = false;
  bool valid = true;
  bool beingInitialized = false;

  std::thread fetcherThread;

  void SetKey( const CString& key );
};

class APIKeyManager
{
  bool initialized = false;

public:

  enum class Status
  {
    OK,
    Loading,
    KeyNotSet,
    CouldNotIdentifyAccount,
    WaitingForMumbleCharacterName,
    AllKeysInvalid
  };

  CArrayThreadSafe<APIKey*> keys;

  APIKey* GetIdentifiedAPIKey();
  Status GetStatus();
  Status DisplayStatusText( CWBDrawAPI* API, CWBFont* font );
  void Initialize();
  void RebuildConfigValues();
};

extern APIKeyManager apiKeyManager;

}

CString FetchAPIData( char* path, const CString& apiKey );

void CheckFestivalActive();

