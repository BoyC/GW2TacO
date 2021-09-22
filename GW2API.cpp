#include "GW2API.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"
#include "Language.h"

#include "Bedrock/UtilLib/jsonxx.h"
using namespace jsonxx;

CString FetchHTTPS( LPCWSTR url, LPCWSTR path );

CString FetchAPIData( char* path, const CString& apiKey )
{
  WCHAR wpath[ 4096 ];

  TBOOL hasquestionmark = false;
  char* str = path;
  while ( *str )
  {
    if ( *str == '?' )
    {
      hasquestionmark = true;
      break;
    }
    str++;
  }

  CString wstr = CString( path );
  if ( hasquestionmark )
    wstr += "&access_token=" + apiKey;
  else
    wstr += "?access_token=" + apiKey;

  memset( wpath, 0, sizeof( wpath ) );
  wstr.WriteAsWideChar( wpath, 4096 );
  return FetchHTTPS( L"api.guildwars2.com", wpath );
}

namespace GW2
{

  APIKeyManager apiKeyManager;

  APIKey::APIKey( const CString& key )
  {
    apiKey = key;
  }

  APIKey::~APIKey()
  {
    if ( fetcherThread.joinable() )
      fetcherThread.join();
  }

  void APIKey::FetchData()
  {
    if ( beingInitialized )
      return;

    keyName = "";
    accountName = "";
    charNames.FlushFast();
    caps.Flush();
    worldId = 0;

    valid = true;
    initialized = false;
    beingInitialized = true;

    fetcherThread = std::thread( [ this ]()
    {
      valid = true;

      CString keyData = QueryAPI( "/v2/tokeninfo" );

      Object json;
      json.parse( keyData.GetPointer() );

      if ( json.has<String>( "name" ) )
        keyName = CString( json.get<String>( "name" ).data() );
      else
        valid = false;

      if ( json.has<Array>( "permissions" ) )
      {
        auto& values = json.get<Array>( "permissions" ).values();
        for ( auto v : values )
        {
          if ( v->is<String>() )
            caps[ CString( v->get<String>().data() ) ] = true;
        }
      }
      else
        valid = false;

      if ( HasCaps( "account" ) )
      {
        CString accountData = QueryAPI( "/v2/account" );
        json.parse( accountData.GetPointer() );

        if ( json.has<String>( "name" ) )
          accountName = CString( json.get<String>( "name" ).data() );

        if ( json.has<Number>( "world" ) )
          worldId = (TS32)( json.get<Number>( "world" ) );
      }

      if ( HasCaps( "characters" ) )
      {
        CString characters = QueryAPI( "/v2/characters" );
        characters = CString::Format( "{\"characters\": %s }", characters.GetPointer() );
        json.parse( characters.GetPointer() );
        auto& values = json.get<Array>( "characters" ).values();
        for ( auto v : values )
        {
          if ( v->is<String>() )
            charNames += CString( v->get<String>().data() );
        }
      }
      else
      {
        LOG_ERR( "[TacO] API error: API key '%s - %s (%s)' doesn't have the 'characters' permission - account identification through Mumble Link will not be possible.", accountName.GetPointer(), keyName.GetPointer(), apiKey.GetPointer() );
      }

      initialized = true;
      beingInitialized = false;
    } );
  }

  TBOOL APIKey::HasCaps( const CString& cap )
  {
    if ( caps.HasKey( cap ) )
      return caps[ cap ];

    return false;
  }

  CString APIKey::QueryAPI( char* path )
  {
    LOG_NFO( "[GW2TacO] Querying the API: %s", path );

    return FetchAPIData( path, apiKey );
  }

  void APIKey::SetKey( const CString& key )
  {
    if ( fetcherThread.joinable() )
      fetcherThread.join();
    apiKey = key;
    caps.Flush();
    initialized = false;
    valid = true;
  }

  APIKey* APIKeyManager::GetIdentifiedAPIKey()
  {
    if (!mumbleLink.IsValid() || !mumbleLink.charName.Length() || !keys.NumItems())
      return nullptr;

    if (!initialized)
      Initialize();

    for (int x = 0; x < keys.NumItems(); x++)
    {
      APIKey* key = keys[x];
      if (!key->initialized)
        continue;

      if (key->fetcherThread.joinable())
        key->fetcherThread.join();

      if (key->charNames.Find(mumbleLink.charName) >= 0)
        return key;
    }

    return nullptr;
  }

  APIKeyManager::Status APIKeyManager::GetStatus()
  {
    if (!initialized)
      Initialize();

    if (!keys.NumItems())
      return Status::KeyNotSet;

    APIKey* key = GetIdentifiedAPIKey();
    if (!key) 
    {
      for (int x = 0; x < keys.NumItems(); x++)
      {
        if (!keys[x]->initialized)
          return Status::Loading;
      }

      if (!mumbleLink.charName.Length())
        return Status::WaitingForMumbleCharacterName;

      return Status::CouldNotIdentifyAccount;
    }

    if (!key->initialized) 
      return Status::Loading;
      
    return Status::OK;
  }

  APIKeyManager::Status APIKeyManager::DisplayStatusText(CWBDrawAPI* API, CWBFont* font)
  {
    APIKeyManager::Status status = GetStatus();

    switch (status) 
    {
    case Status::Loading:
      font->Write(API, DICT("waitingforapi"), CPoint(0, 0));
      break;
    case Status::KeyNotSet:
      font->Write(API, DICT("apikeynotset1"), CPoint(0, 0), CColor(0xff,0x40,0x40,0xff));
      font->Write(API, DICT("apikeynotset2"), CPoint(0, font->GetLineHeight()), CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::CouldNotIdentifyAccount:
      font->Write(API, DICT("couldntidentifyaccount1"), CPoint(0, 0), CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("couldntidentifyaccount2"), CPoint(0, font->GetLineHeight()), CColor(0xff, 0x40, 0x40, 0xff));
      break;
    case Status::WaitingForMumbleCharacterName:
      font->Write(API, DICT("waitingforcharactername1"), CPoint(0, 0));
      break;
    case Status::AllKeysInvalid:
      font->Write(API, DICT("apierror1"), CPoint(0, 0), CColor(0xff, 0x40, 0x40, 0xff));
      font->Write(API, DICT("apierror2"), CPoint(0, font->GetLineHeight()), CColor(0xff, 0x40, 0x40, 0xff));
      break;
    }
    return status;
  }

  void APIKeyManager::Initialize()
  {
    if (initialized)
      return;

    CString oldApiKey;

    if (HasConfigString("GW2APIKey"))
    {
      APIKey* key = new APIKey(GetConfigString("GW2APIKey"));
      RemoveConfigEntry("GW2APIKey");
      keys += key;
    }

    int x = 0;
    while (true) 
    {
      CString cfgName = CString::Format("GW2APIKey%d", x++);
      if (HasConfigString(cfgName.GetPointer()))
      {
        APIKey* key = new APIKey(GetConfigString(cfgName.GetPointer()));
        keys += key;
      }
      else
        break;
    }

    RebuildConfigValues();

    for (int x = 0; x < keys.NumItems(); x++)
      keys[x]->FetchData();

    initialized = true;
  }

  void APIKeyManager::RebuildConfigValues()
  {
    int x = 0;
    while (true)
    {
      CString cfgName = CString::Format("GW2APIKey%d", x++);
      if (HasConfigString(cfgName.GetPointer()))
        RemoveConfigEntry(cfgName.GetPointer());
      else
        break;
    }

    for (int x = 0; x < keys.NumItems(); x++)
    {
      CString cfgName = CString::Format("GW2APIKey%d", x);
      SetConfigString(cfgName.GetPointer(), keys[x]->apiKey);
    }
    
    SaveConfig();
  }

}
