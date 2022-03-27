#include "DungeonProgress.h"
#include "GW2API.h"
#include "OverlayConfig.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "Language.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"

CDictionary<CString, TS32> dungeonToAchievementMap;

using namespace jsonxx;

void DungeonProgress::OnDraw( CWBDrawAPI* API )
{
  CWBFont* f = GetFont( GetState() );
  TS32 size = f->GetLineHeight();

  GW2::APIKeyManager::Status status = GW2::apiKeyManager.DisplayStatusText( API, f );
  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if ( key && key->valid && ( GetTime() - lastFetchTime > 150000 || !lastFetchTime ) && !beingFetched && !fetchThread.joinable() )
  {
    beingFetched = true;
    fetchThread = std::thread( [this, key]()
                               {
                                 SetPerThreadCRTExceptionBehavior();

                                 if ( !hasFullDungeonInfo )
                                 {
                                   Object json;

                                   CString globalRaidInfo = CString( "{\"dungeons\":" ) + key->QueryAPI( "v2/dungeons" ) + "}";
                                   json.parse( globalRaidInfo.GetPointer() );

                                   if ( json.has<Array>( "dungeons" ) )
                                   {
                                     auto dungeonData = json.get<Array>( "dungeons" ).values();

                                     for ( unsigned int x = 0; x < dungeonData.size(); x++ )
                                     {
                                       if ( !dungeonData[ x ]->is<String>() )
                                         continue;

                                       Dungeon d;
                                       d.name = CString( dungeonData[ x ]->get<String>().data() );

                                       CString raidInfo = key->QueryAPI( ( CString( "v2/dungeons/" ) + d.name ).GetPointer() );
                                       Object dungeonJson;
                                       dungeonJson.parse( raidInfo.GetPointer() );

                                       if ( dungeonJson.has<Array>( "paths" ) )
                                       {
                                         auto wings = dungeonJson.get<Array>( "paths" ).values();
                                         for ( unsigned y = 0; y < wings.size(); y++ )
                                         {
                                           auto dungeonPath = wings[ y ]->get<Object>();
                                           DungeonPath p;
                                           if ( dungeonPath.has<String>( "id" ) )
                                             p.name = CString( dungeonPath.get<String>( "id" ).data() );

                                           if ( dungeonPath.has<String>( "type" ) )
                                             p.type = CString( dungeonPath.get<String>( "type" ).data() );

                                           d.paths += p;
                                         }
                                       }

                                       if ( d.name.Length() )
                                       {
                                         d.shortName = CString::Format( "%c", toupper( d.name[ 0 ] ) );

                                         for ( unsigned int y = 0; y < d.name.Length() - 1; y++ )
                                         {
                                           if ( d.name[ y ] == '_' )
                                           {
                                             if ( d.name[ y + 1 ] == 'o' && y < d.name.Length() - 2 && d.name[ y + 2 ] == 'f' )
                                               d.shortName += "o";
                                             else
                                               if ( d.name[ y + 1 ] == 't' && y < d.name.Length() - 3 && d.name[ y + 2 ] == 'h' && d.name[ y + 3 ] == 'e' )
                                                 d.shortName += "t";
                                               else
                                                 if ( d.name[ y + 1 ] == 'a' && y < d.name.Length() - 4 && d.name[ y + 2 ] == 'r' && d.name[ y + 3 ] == 'a' && d.name[ y + 4 ] == 'h' )
                                                   d.shortName = "Arah";
                                                 else
                                                   d.shortName += CString::Format( "%c", (char)toupper( d.name[ y + 1 ] ) );
                                           }
                                         }
                                       }

                                       dungeons += d;
                                     }
                                   }

                                   hasFullDungeonInfo = true;
                                 }

                                 CString lastDungeonStatus = CString( "{\"dungeons\":" ) + key->QueryAPI( "v2/account/dungeons" ) + "}";
                                 CString dungeonFrequenterStatus = CString( "{\"dungeons\":" ) + key->QueryAPI( "v2/account/achievements?ids=2963" ) + "}";
                                 Object json;
                                 Object json2;
                                 json.parse( lastDungeonStatus.GetPointer() );
                                 json2.parse( dungeonFrequenterStatus.GetPointer() );

                                 if ( json.has<Array>( "dungeons" ) )
                                 {
                                   auto dungeonData = json.get<Array>( "dungeons" ).values();

                                   for ( unsigned int x = 0; x < dungeonData.size(); x++ )
                                   {
                                     if ( !dungeonData[ x ]->is<String>() )
                                       continue;

                                     CString eventName = CString( dungeonData[ x ]->get<String>().data() );
                                     for ( TS32 a = 0; a < dungeons.NumItems(); a++ )
                                       for ( TS32 b = 0; b < dungeons[ a ].paths.NumItems(); b++ )
                                       {
                                         if ( dungeons[ a ].paths[ b ].name == eventName )
                                           dungeons[ a ].paths[ b ].finished = true;
                                       }
                                   }
                                 }

                                 if ( json2.has<Array>( "dungeons" ) )
                                 {
                                   auto dungeonData = json2.get<Array>( "dungeons" ).values();

                                   for ( TS32 a = 0; a < dungeons.NumItems(); a++ )
                                     for ( TS32 b = 0; b < dungeons[ a ].paths.NumItems(); b++ )
                                       dungeons[ a ].paths[ b ].frequenter = false;

                                   if ( dungeonData.size() >= 1 && dungeonData[ 0 ]->is<Object>() )
                                   {
                                     Object obj = dungeonData[ 0 ]->get<Object>();
                                     if ( obj.has<Array>( "bits" ) )
                                     {
                                       auto bits = obj.get<Array>( "bits" ).values();
                                       if ( bits.size() > 0 )
                                       {
                                         for ( unsigned int x = 0; x < bits.size(); x++ )
                                         {
                                           if ( bits[ x ]->is<Number>() )
                                           {
                                             TS32 frequentedID = (TS32)( bits[ x ]->get<Number>() );
                                             for ( TS32 a = 0; a < dungeons.NumItems(); a++ )
                                               for ( TS32 b = 0; b < dungeons[ a ].paths.NumItems(); b++ )
                                               {
                                                 if ( dungeonToAchievementMap.HasKey( dungeons[ a ].paths[ b ].name ) )
                                                 {
                                                   if ( dungeonToAchievementMap[ dungeons[ a ].paths[ b ].name ] == frequentedID )
                                                     dungeons[ a ].paths[ b ].frequenter = true;
                                                 }
                                               }
                                           }
                                         }
                                       }
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

  if ( hasFullDungeonInfo )
  {
    TS32 posy = 1;

    TS32 textwidth = 0;
    for ( int x = 0; x < dungeons.NumItems(); x++ )
      textwidth = max( textwidth, f->GetWidth( dungeons[ x ].shortName, false ) );

    for ( int x = 0; x < dungeons.NumItems(); x++ )
    {
      auto& d = dungeons[ x ];

      f->Write( API, d.shortName + ":", CPoint( 0, posy + 1 ), 0xffffffff );
      TS32 posx = textwidth + f->GetLineHeight() / 2;
      for ( int y = 0; y < d.paths.NumItems(); y++ )
      {
        auto& p = d.paths[ y ];

        CRect r = CRect( posx, posy, posx + f->GetLineHeight() * 2, posy + f->GetLineHeight() - 1 );
        CRect cr = API->GetCropRect();
        API->SetCropRect( ClientToScreen( r ) );
        posx += f->GetLineHeight() * 2 + 1;
        if ( y == 0 )
          posx += f->GetLineHeight() / 2;
        API->DrawRect( r, p.finished ? 0x8033cc11 : 0x80cc3322 );
        CString s = y == 0 ? "S" : CString::Format( "P%d", y );

        if ( d.shortName == "TA" )
        {
          switch ( y )
          {
          case 1:
            s = "Up";
            break;
          case 2:
            s = "Fwd";
            break;
          case 3:
            s = "Ae";
            break;
          }
        }

        CPoint tp = f->GetTextPosition( s, r + CRect( -3, 0, 0, 0 ), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE );
        tp.y = posy + 1;
        f->Write( API, s, tp, 0xffffffff );
        API->DrawRectBorder( r, p.frequenter ? 0xffffcc00 : 0x80000000 );
        API->SetCropRect( cr );
      }
      posy += f->GetLineHeight();
    }
  }
  else
    f->Write( API, DICT( "waitingforapi" ), CPoint( 0, 0 ), 0xffffffff );

  DrawBorder( API );
}

DungeonProgress::DungeonProgress( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{
  dungeonToAchievementMap[ "ac_story" ] = 4;
  dungeonToAchievementMap[ "hodgins" ] = 5;
  dungeonToAchievementMap[ "detha" ] = 6;
  dungeonToAchievementMap[ "tzark" ] = 7;
  dungeonToAchievementMap[ "cm_story" ] = 12;
  dungeonToAchievementMap[ "asura" ] = 13;
  dungeonToAchievementMap[ "seraph" ] = 14;
  dungeonToAchievementMap[ "butler" ] = 15;
  dungeonToAchievementMap[ "ta_story" ] = 20;
  dungeonToAchievementMap[ "leurent" ] = 21;
  dungeonToAchievementMap[ "vevina" ] = 22;
  dungeonToAchievementMap[ "aetherpath" ] = 23;
  dungeonToAchievementMap[ "se_story" ] = 16;
  dungeonToAchievementMap[ "fergg" ] = 17;
  dungeonToAchievementMap[ "rasalov" ] = 18;
  dungeonToAchievementMap[ "koptev" ] = 19;
  dungeonToAchievementMap[ "cof_story" ] = 28;
  dungeonToAchievementMap[ "ferrah" ] = 29;
  dungeonToAchievementMap[ "magg" ] = 30;
  dungeonToAchievementMap[ "rhiannon" ] = 31;
  dungeonToAchievementMap[ "hotw_story" ] = 24;
  dungeonToAchievementMap[ "butcher" ] = 25;
  dungeonToAchievementMap[ "plunderer" ] = 26;
  dungeonToAchievementMap[ "zealot" ] = 27;
  dungeonToAchievementMap[ "coe_story" ] = 0;
  dungeonToAchievementMap[ "submarine" ] = 1;
  dungeonToAchievementMap[ "teleporter" ] = 2;
  dungeonToAchievementMap[ "front_door" ] = 3;
  dungeonToAchievementMap[ "jotun" ] = 8;
  dungeonToAchievementMap[ "mursaat" ] = 9;
  dungeonToAchievementMap[ "forgotten" ] = 10;
  dungeonToAchievementMap[ "seer" ] = 11;
}

DungeonProgress::~DungeonProgress()
{
  if ( fetchThread.joinable() )
    fetchThread.join();
}

CWBItem* DungeonProgress::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new DungeonProgress( Root, Pos );
}

TBOOL DungeonProgress::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

