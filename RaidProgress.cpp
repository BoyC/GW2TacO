#include "RaidProgress.h"
#include "GW2API.h"
#include "OverlayConfig.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "Language.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"

using namespace jsonxx;

void BeautifyString( CString& str )
{
  for ( TU32 x = 0; x < str.Length(); x++ )
  {
    if ( str[ x ] == '_' )
      str[ x ] = ' ';

    if ( x == 0 || str[ x - 1 ] == ' ' )
      str[ x ] = toupper( str[ x ] );

    if ( x > 0 && str[ x - 1 ] == ' ' )
    {
      if ( str[ x ] == 'O' && x < str.Length() - 1 && str[ x + 1 ] == 'f' )
        str[ x ] = 'o';
      if ( str[ x ] == 'T' && x < str.Length() - 2 && str[ x + 1 ] == 'h'  && str[ x + 2 ] == 'e' )
        str[ x ] = 't';
    }
  }
}

void RaidProgress::OnDraw( CWBDrawAPI *API )
{
  if ( !HasConfigValue( "CompactRaidWindow" ) )
    SetConfigValue( "CompactRaidWindow", 0 );

  TBOOL compact = GetConfigValue( "CompactRaidWindow" );

  CWBFont *f = GetFont( GetState() );
  TS32 size = f->GetLineHeight();

  GW2::APIKeyManager::Status status = GW2::apiKeyManager.DisplayStatusText(API, f);
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if ( key && key->valid && ( GetTime() - lastFetchTime > 150000 || !lastFetchTime ) && !beingFetched && !fetchThread.joinable() )
  {
    beingFetched = true;
    fetchThread = std::thread( [ this, key ]()
    {
      SetPerThreadCRTExceptionBehavior();
      if ( !hasFullRaidInfo )
      {
        Object json;

        CString globalRaidInfo = CString( "{\"raids\":" ) + key->QueryAPI( "v2/raids" ) + "}";
        json.parse( globalRaidInfo.GetPointer() );

        if ( json.has<Array>( "raids" ) )
        {
          auto raidData = json.get<Array>( "raids" ).values();

          for ( unsigned int x = 0; x < raidData.size(); x++ )
          {
            if ( !raidData[ x ]->is<String>() )
              continue;

            Raid r;
            r.name = CString( raidData[ x ]->get<String>().data() );
            if ( r.name.Length() )
            {
              r.shortName = CString::Format( "%c", toupper( r.name[ 0 ] ) );

              for ( unsigned int y = 0; y < r.name.Length() - 1; y++ )
              {
                if ( r.name[ y ] == '_' || r.name[ y ] == ' ' )
                {
                  if ( r.name[ y + 1 ] == 'o' && y < r.name.Length() - 2 && r.name[ y + 2 ] == 'f' )
                    r.shortName += "o";
                  else
                    if ( r.name[ y + 1 ] == 't' && y < r.name.Length() - 3 && r.name[ y + 2 ] == 'h' && r.name[ y + 3 ] == 'e' )
                      r.shortName += "t";
                    else
                      r.shortName += CString::Format( "%c", (char)toupper( r.name[ y + 1 ] ) );
                }
              }
              r.shortName += ":";
            }

            r.configName = CString( "showraid_" ) + r.name;
            for ( TU32 y = 0; y < r.configName.Length(); y++ )
              if ( !isalnum( r.configName[ y ] ) )
                r.configName[ y ] = '_';
              else
                r.configName[ y ] = tolower( r.configName[ y ] );

            CString raidInfo = key->QueryAPI( ( CString( "v2/raids/" ) + r.name ).GetPointer() );
            Object raidJson;
            raidJson.parse( raidInfo.GetPointer() );

            if ( raidJson.has<Array>( "wings" ) )
            {
              auto wings = raidJson.get<Array>( "wings" ).values();
              for ( unsigned y = 0; y < wings.size(); y++ )
              {
                auto wing = wings[ y ]->get<Object>();
                Wing w;
                if ( wing.has<String>( "id" ) )
                  w.name = CString( wing.get<String>( "id" ).data() );

                if ( wing.has<Array>( "events" ) )
                {
                  auto events = wing.get<Array>( "events" ).values();
                  for ( unsigned int z = 0; z < events.size(); z++ )
                  {
                    auto _event = events[ z ]->get<Object>();
                    RaidEvent e;
                    if ( _event.has<String>( "id" ) )
                      e.name = CString( _event.get<String>( "id" ).data() );
                    if ( _event.has<String>( "type" ) )
                      e.type = CString( _event.get<String>( "type" ).data() );

                    w.events += e;
                  }
                }
                r.wings += w;
              }
            }

            BeautifyString( r.name );
            raids += r;
          }
        }

        hasFullRaidInfo = true;
      }

      CString lastRaidStatus = CString( "{\"raids\":" ) + key->QueryAPI( "v2/account/raids" ) + "}";
      Object json;
      json.parse( lastRaidStatus.GetPointer() );

      if ( json.has<Array>( "raids" ) )
      {
        auto raidData = json.get<Array>( "raids" ).values();

        for ( unsigned int x = 0; x < raidData.size(); x++ )
        {
          if ( !raidData[ x ]->is<String>() )
            continue;

          CString eventName = CString( raidData[ x ]->get<String>().data() );
          for ( TS32 a = 0; a < raids.NumItems(); a++ )
            for ( TS32 b = 0; b < raids[ a ].wings.NumItems(); b++ )
              for ( TS32 c = 0; c < raids[ a ].wings[ b ].events.NumItems(); c++ )
              {
                if ( raids[ a ].wings[ b ].events[ c ].name == eventName )
                  raids[ a ].wings[ b ].events[ c ].finished = true;
              }
        }

      }


      beingFetched = false;
    } );
  }

  if ( !beingFetched && fetchThread.joinable() )
  {
    lastFetchTime = GetTime();
    fetchThread.join();
  }

  if ( hasFullRaidInfo )
  {
    TS32 posx = 0;
    if ( compact )
    {
      for ( TS32 x = 0; x < raids.NumItems(); x++ )
        posx = max( posx, f->GetWidth( raids[ x ].shortName ) );
    }
    posx += 3;
    TS32 oposx = posx;

    TS32 posy = 0;
    for ( int x = 0; x < raids.NumItems(); x++ )
    {
      auto& r = raids[ x ];

      if ( HasConfigValue( r.configName.GetPointer() ) && !GetConfigValue( r.configName.GetPointer() ) )
        continue;

      if ( !compact )
      {
        f->Write( API, DICT( r.configName, r.name ), CPoint( 0, posy + 1 ), 0xffffffff );
        posy += f->GetLineHeight();
      }
      else
      {
        f->Write( API, r.shortName, CPoint( 0, posy + 1 ), 0xffffffff );
      }
      for ( int y = 0; y < r.wings.NumItems(); y++ )
      {
        auto& w = r.wings[ y ];

        if ( !compact )
          posx = f->GetLineHeight() * 1;
        else
          posx = oposx;

        if ( !compact )
          f->Write( API, DICT( "raid_wing" ) + CString::Format( "%d:", y + 1 ), CPoint( posx, posy + 1 ), 0xffffffff );

        if ( !compact )
          posx = f->GetLineHeight() * 3;

        int cnt = 1;

        for ( int z = 0; z < w.events.NumItems(); z++ )
        {
          auto& e = w.events[ z ];

          CRect r = CRect( posx, posy, posx + f->GetLineHeight() * 2, posy + f->GetLineHeight() - 1 );
          CRect cr = API->GetCropRect();
          API->SetCropRect( ClientToScreen( r ) );
          posx += f->GetLineHeight() * 2 + 1;
          API->DrawRect( r, e.finished ? 0x8033cc11 : 0x80cc3322 );
          CString s = e.type[ 0 ] == 'B' ? ( DICT( "raid_boss" ) + CString::Format( "%d", cnt ) ) : DICT( "raid_event" );

          if ( e.type[ 0 ] == 'B' )
            cnt++;

          CPoint tp = f->GetTextPosition( s, r + CRect( -3, 0, 0, 0 ), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE );
          tp.y = posy + 1;
          f->Write( API, s, tp, 0xffffffff );
          API->DrawRectBorder( r, 0x80000000 );
          API->SetCropRect( cr );
        }

        posy += f->GetLineHeight();
      }
    }
  }
  else
    f->Write( API, DICT( "waitingforapi" ), CPoint( 0, 0 ), 0xffffffff );

  DrawBorder( API );
}

RaidProgress::RaidProgress( CWBItem *Parent, CRect Position ) : CWBItem( Parent, Position )
{
}

RaidProgress::~RaidProgress()
{
  if ( fetchThread.joinable() )
    fetchThread.join();
}

CWBItem * RaidProgress::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new RaidProgress( Root, Pos );
}

TBOOL RaidProgress::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

CArray<Raid>& RaidProgress::GetRaids()
{
  return raids;
}

