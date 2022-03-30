#include "GW2API.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"
#include "Language.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"

#include "Bedrock/UtilLib/jsonxx.h"
using namespace jsonxx;

#include <winhttp.h>
#pragma comment(lib,"winhttp.lib")

#include <Urlmon.h>   // URLOpenBlockingStreamW()
#pragma comment( lib, "Urlmon.lib" )

bool DownloadFile( const CString& url, CStreamWriterMemory& mem )
{
  LPSTREAM stream;

  HRESULT hr = URLOpenBlockingStream( nullptr, ( CString( "https://" ) + url ).GetPointer(), &stream, 0, nullptr );
  if ( FAILED( hr ) )
    hr = URLOpenBlockingStream( nullptr, ( CString( "http://" ) + url ).GetPointer(), &stream, 0, nullptr );

  if ( FAILED( hr ) )
    return false;

  char buffer[ 4096 ];
  do
  {
    DWORD bytesRead = 0;
    hr = stream->Read( buffer, sizeof( buffer ), &bytesRead );
    mem.Write( buffer, bytesRead );
  } while ( SUCCEEDED( hr ) && hr != S_FALSE );

  stream->Release();

  if ( FAILED( hr ) )
    return false;

  return true;
}

CString FetchHTTP( LPCWSTR url, LPCWSTR path )
{
  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;
  LPSTR pszOutBuffer;

  BOOL  bResults = FALSE;
  HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;

  hSession = WinHttpOpen( L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

  if ( hSession )
    hConnect = WinHttpConnect( hSession, url, INTERNET_DEFAULT_HTTP_PORT, 0 );

  if ( hConnect )
    hRequest = WinHttpOpenRequest( hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, NULL, NULL );

  if ( hRequest )
    bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );

  if ( bResults )
    bResults = WinHttpReceiveResponse( hRequest, NULL );

  if ( !bResults )
  {
    if ( hRequest ) WinHttpCloseHandle( hRequest );
    if ( hConnect ) WinHttpCloseHandle( hConnect );
    if ( hSession ) WinHttpCloseHandle( hSession );
    return "";
  }

  pszOutBuffer = nullptr;

  CStreamWriterMemory data;

  do
  {
    dwSize = 0;
    if ( !WinHttpQueryDataAvailable( hRequest, &dwSize ) )
    {
      if ( hRequest ) WinHttpCloseHandle( hRequest );
      if ( hConnect ) WinHttpCloseHandle( hConnect );
      if ( hSession ) WinHttpCloseHandle( hSession );
      return "";
    }

    pszOutBuffer = new char[ dwSize + 1 ];

    if ( !WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded ) )
    {
      if ( hRequest ) WinHttpCloseHandle( hRequest );
      if ( hConnect ) WinHttpCloseHandle( hConnect );
      if ( hSession ) WinHttpCloseHandle( hSession );
      return "";
    }

    data.Write( pszOutBuffer, dwSize );

    SAFEDELETEA( pszOutBuffer );
  } while ( dwSize > 0 );


  if ( hRequest ) WinHttpCloseHandle( hRequest );
  if ( hConnect ) WinHttpCloseHandle( hConnect );
  if ( hSession ) WinHttpCloseHandle( hSession );

  return CString( (TS8*)data.GetData(), data.GetLength() );
}

CString FetchHTTPS( LPCWSTR url, LPCWSTR path )
{
  CString s1 = CString( url );
  CString s2 = CString( path );

  LOG_NFO( "[GW2TacO] Fetching URL: %s/%s", s1.GetPointer(), s2.GetPointer() );

  DWORD dwSize = 0;
  DWORD dwDownloaded = 0;
  LPSTR pszOutBuffer;

  BOOL  bResults = FALSE;
  HINTERNET  hSession = NULL, hConnect = NULL, hRequest = NULL;

  hSession = WinHttpOpen( L"WinHTTPS Example/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0 );

  if ( hSession )
    hConnect = WinHttpConnect( hSession, url, INTERNET_DEFAULT_PORT, 0 );

  if ( hConnect )
    hRequest = WinHttpOpenRequest( hConnect, L"GET", path, NULL, WINHTTP_NO_REFERER, NULL, WINHTTP_FLAG_SECURE );

  if ( hRequest )
    bResults = WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0 );

  if ( bResults )
    bResults = WinHttpReceiveResponse( hRequest, NULL );

  if ( !bResults )
    return "";

  pszOutBuffer = nullptr;

  CStreamWriterMemory data;

  do
  {
    dwSize = 0;
    if ( !WinHttpQueryDataAvailable( hRequest, &dwSize ) )
      return "";

    pszOutBuffer = new char[ dwSize + 1 ];

    if ( !WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded ) )
      return "";

    data.Write( pszOutBuffer, dwSize );

    SAFEDELETEA( pszOutBuffer );
  } while ( dwSize > 0 );


  if ( hRequest ) WinHttpCloseHandle( hRequest );
  if ( hConnect ) WinHttpCloseHandle( hConnect );
  if ( hSession ) WinHttpCloseHandle( hSession );

  return CString( (TS8*)data.GetData(), data.GetLength() );
}

CString FetchWeb( const CString& url )
{
  bool https = url.Find( "https://" ) == 0;
  bool http = url.Find( "http://" ) == 0;
  if ( !https && !http )
    return "";

  int start = url.Find( "://" ) + 3;
  CString domain = url.Substring( start );

  int domainEnd = domain.Find( "/" );
  if ( domainEnd < 0 )
    return "";

  CString address = domain.Substring( domainEnd + 1 );
  domain = domain.Substring( 0, domainEnd );

  WCHAR wdomain[ 4096 ]{};
  WCHAR waddr[ 4096 ]{};
  address.WriteAsWideChar( waddr, 4096 );
  domain.WriteAsWideChar( wdomain, 4096 );

  if ( https )
    return FetchHTTPS( wdomain, waddr );

  return FetchHTTP( wdomain, waddr );
}

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

GW2::FestivalData GW2::festivals[ 6 ] = { { "halloween"             , 79,  false }, // halloween daily
                                          { "wintersday"            , 98,  false }, // wintersday daily
                                          { "superadventurefestival", 162, false }, // sab daily
                                          { "lunarnewyear"          , 201, false }, // daily lunar new year
                                          { "festivalofthefourwinds", 213, false }, // festival of the four winds daily
                                          { "dragonbash"            , 233, false }  // daily dragon bash
};


std::vector<GW2::Festival> festivalDailies;

void InitFestivalDailies()
{
  for ( auto festival : GW2::festivals )
  {
    GW2::Festival fest;
    fest.name = festival.name;

    CString id = "v2/achievements/categories?id=" + CString::Format( "%d", festival.category );
    WCHAR wpath[ 4096 ]{};
    id.WriteAsWideChar( wpath, 4096 );
    auto achievementCategoryData = FetchHTTPS( L"api.guildwars2.com", wpath );

    Object json;
    json.parse( achievementCategoryData.GetPointer() );
    if ( json.has<Array>( "achievements" ) )
    {
      auto& values = json.get<Array>( "achievements" ).values();
      for ( auto v : values )
      {
        if ( v->is<Number>() )
          fest.dailyAchievements.emplace_back( (int)v->get<Number>() );
      }
    }

    festivalDailies.emplace_back( fest );
  }
}

void CheckFestivalActive()
{
  if ( !festivalDailies.size() )
    InitFestivalDailies();

  auto dailies = FetchHTTPS( L"api.guildwars2.com", L"v2/achievements/daily" );
  std::set<int> dailyAchies;

  Object json;
  json.parse( dailies.GetPointer() );

  if ( json.has<Array>( "pve" ) )
  {
    auto& values = json.get<Array>( "pve" ).values();
    for ( auto v : values )
    {
      auto& obj = v->get<Object>();
      if ( obj.has<Number>( "id" ) )
        dailyAchies.insert( (int)obj.get<Number>( "id" ) );
    }
  }

  if ( json.has<Array>( "pvp" ) )
  {
    auto& values = json.get<Array>( "pvp" ).values();
    for ( auto v : values )
    {
      auto& obj = v->get<Object>();
      if ( obj.has<Number>( "id" ) )
        dailyAchies.insert( (int)obj.get<Number>( "id" ) );
    }
  }

  if ( json.has<Array>( "wvw" ) )
  {
    auto& values = json.get<Array>( "wvw" ).values();
    for ( auto v : values )
    {
      auto& obj = v->get<Object>();
      if ( obj.has<Number>( "id" ) )
        dailyAchies.insert( (int)obj.get<Number>( "id" ) );
    }
  }

  if ( json.has<Array>( "special" ) )
  {
    auto& values = json.get<Array>( "special" ).values();
    for ( auto v : values )
    {
      auto& obj = v->get<Object>();
      if ( obj.has<Number>( "id" ) )
        dailyAchies.insert( (int)obj.get<Number>( "id" ) );
    }
  }

  int cnt = 0;

  for ( auto& festival : festivalDailies )
  {
    GW2::festivals[ cnt ].active = false;
    for ( auto& achi : festival.dailyAchievements )
    {
      if ( dailyAchies.find( achi ) != dailyAchies.end() )
      {
        GW2::festivals[ cnt ].active = true;
        break;
      }
    }
    cnt++;
  }

  cnt = 0;

  for ( auto& festival : festivalDailies )
  {
    if ( !festival.dailyAchievements.empty() )
      GW2::festivals[ cnt ].active = true;
    cnt++;
  }

  if ( Config::GetValue( "ForceFestivals" ) )
    for ( auto& festival : GW2::festivals )
      festival.active = true;

  //GW2::festivals[ 1 ].active = true;
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

  fetcherThread = std::thread( [this]()
                               {
                                 SetPerThreadCRTExceptionBehavior();
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
                                   if ( !json.has<Array>( "characters" ) )
                                   {
                                     LOG_ERR( "[GW2TacO] Unexpected result from API characters endpoint: %s", characters.GetPointer() );
                                     LOG_ERR( "[GW2TacO] CHARACTERS WON'T BE RECOGNIZED FOR API KEY NAMED %s", keyName.GetPointer() );
                                   }
                                   else
                                   {
                                     auto& charArray = json.get<Array>( "characters" );
                                     auto& values = charArray.values();
                                     for ( auto v : values )
                                     {
                                       if ( v->is<String>() )
                                         charNames += CString( v->get<String>().data() );
                                     }
                                   }
                                 }
                                 else
                                 {
                                   LOG_ERR( "[GW2TacO] API error: API key '%s - %s (%s)' doesn't have the 'characters' permission - account identification through Mumble Link will not be possible.", accountName.GetPointer(), keyName.GetPointer(), apiKey.GetPointer() );
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
  if ( !mumbleLink.IsValid() || !mumbleLink.charName.Length() || !keys.NumItems() )
    return nullptr;

  if ( !initialized )
    Initialize();

  for ( int x = 0; x < keys.NumItems(); x++ )
  {
    APIKey* key = keys[ x ];
    if ( !key->initialized )
      continue;

    if ( key->fetcherThread.joinable() )
      key->fetcherThread.join();

    if ( key->charNames.Find( mumbleLink.charName ) >= 0 )
      return key;
  }

  return nullptr;
}

APIKeyManager::Status APIKeyManager::GetStatus()
{
  if ( !initialized )
    Initialize();

  if ( !keys.NumItems() )
    return Status::KeyNotSet;

  APIKey* key = GetIdentifiedAPIKey();
  if ( !key )
  {
    for ( int x = 0; x < keys.NumItems(); x++ )
    {
      if ( !keys[ x ]->initialized )
        return Status::Loading;
    }

    if ( !mumbleLink.charName.Length() )
      return Status::WaitingForMumbleCharacterName;

    return Status::CouldNotIdentifyAccount;
  }

  if ( !key->initialized )
    return Status::Loading;

  return Status::OK;
}

APIKeyManager::Status APIKeyManager::DisplayStatusText( CWBDrawAPI* API, CWBFont* font )
{
  APIKeyManager::Status status = GetStatus();

  switch ( status )
  {
  case Status::Loading:
    font->Write( API, DICT( "waitingforapi" ), CPoint( 0, 0 ) );
    break;
  case Status::KeyNotSet:
    font->Write( API, DICT( "apikeynotset1" ), CPoint( 0, 0 ), CColor( 0xff, 0x40, 0x40, 0xff ) );
    font->Write( API, DICT( "apikeynotset2" ), CPoint( 0, font->GetLineHeight() ), CColor( 0xff, 0x40, 0x40, 0xff ) );
    break;
  case Status::CouldNotIdentifyAccount:
    font->Write( API, DICT( "couldntidentifyaccount1" ), CPoint( 0, 0 ), CColor( 0xff, 0x40, 0x40, 0xff ) );
    font->Write( API, DICT( "couldntidentifyaccount2" ), CPoint( 0, font->GetLineHeight() ), CColor( 0xff, 0x40, 0x40, 0xff ) );
    break;
  case Status::WaitingForMumbleCharacterName:
    font->Write( API, DICT( "waitingforcharactername1" ), CPoint( 0, 0 ) );
    break;
  case Status::AllKeysInvalid:
    font->Write( API, DICT( "apierror1" ), CPoint( 0, 0 ), CColor( 0xff, 0x40, 0x40, 0xff ) );
    font->Write( API, DICT( "apierror2" ), CPoint( 0, font->GetLineHeight() ), CColor( 0xff, 0x40, 0x40, 0xff ) );
    break;
  }
  return status;
}

void APIKeyManager::Initialize()
{
  if ( initialized )
    return;

  CString oldApiKey;

  if ( Config::HasString( "GW2APIKey" ) )
  {
    APIKey* key = new APIKey( Config::GetString( "GW2APIKey" ) );
    Config::RemoveValue( "GW2APIKey" );
    keys += key;
  }

  int x = 0;
  while ( true )
  {
    CString cfgName = CString::Format( "GW2APIKey%d", x++ );
    if ( Config::HasString( cfgName.GetPointer() ) )
    {
      APIKey* key = new APIKey( Config::GetString( cfgName.GetPointer() ) );
      keys += key;
    }
    else
      break;
  }

  RebuildConfigValues();

  for ( int x = 0; x < keys.NumItems(); x++ )
    keys[ x ]->FetchData();

  initialized = true;
}

void APIKeyManager::RebuildConfigValues()
{
  int x = 0;
  while ( true )
  {
    CString cfgName = CString::Format( "GW2APIKey%d", x++ );
    if ( Config::HasString( cfgName.GetPointer() ) )
      Config::RemoveValue( cfgName.GetPointer() );
    else
      break;
  }

  for ( int x = 0; x < keys.NumItems(); x++ )
  {
    CString cfgName = CString::Format( "GW2APIKey%d", x );
    Config::SetString( cfgName.GetPointer(), keys[ x ]->apiKey );
  }

  Config::Save();
}

}
