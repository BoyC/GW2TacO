#include "Achievements.h"
#include "GW2API.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"

#include "Bedrock/UtilLib/jsonxx.h"
using namespace jsonxx;

bool Achievements::beingFetched = false;
TS32 Achievements::lastFetchTime = 0;
std::thread Achievements::fetchThread;
bool Achievements::fetched = false;
CDictionary<TS32, Achievement> Achievements::achievements;
LIGHTWEIGHT_CRITICALSECTION Achievements::critSec;

void Achievements::FetchAchievements()
{
  if ( GW2::apiKeyManager.GetStatus() != GW2::APIKeyManager::Status::OK )
    return;

  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if ( key && key->valid && ( globalTimer.GetTime() - lastFetchTime > 150000 || !lastFetchTime ) && !beingFetched && !fetchThread.joinable() )
  {
    beingFetched = true;
    fetchThread = std::thread( [key]()
                               {
                                 SetPerThreadCRTExceptionBehavior();
                                 CheckFestivalActive();
                                 CString dungeonFrequenterStatus = CString( "{\"achievements\":" ) + key->QueryAPI( "v2/account/achievements" ) + "}";
                                 Object json;
                                 json.parse( dungeonFrequenterStatus.GetPointer() );

                                 if ( json.has<Array>( "achievements" ) )
                                 {
                                   auto achiData = json.get<Array>( "achievements" ).values();

                                   CDictionary<TS32, Achievement> incoming;

                                   for ( unsigned int x = 0; x < achiData.size(); x++ )
                                   {
                                     if ( !achiData[ x ]->is<Object>() )
                                       continue;
                                     auto& data = achiData[ x ]->get<Object>();

                                     if ( !data.has<Boolean>( "done" ) )
                                       continue;

                                     TBOOL done = data.get<Boolean>( "done" );

                                     if ( !data.has<Number>( "id" ) )
                                       continue;

                                     TS32 achiId = TS32( data.get<Number>( "id" ) );
                                     incoming[ achiId ].done = done;

                                     if ( !done && data.has<Array>( "bits" ) )
                                     {
                                       auto& bitArray = incoming[ achiId ].bits;
                                       auto bits = data.get<Array>( "bits" ).values();
                                       for ( unsigned int y = 0; y < bits.size(); y++ )
                                       {
                                         if ( !bits[ y ]->is<Number>() )
                                           continue;
                                         bitArray += TS32( bits[ y ]->get<Number>() );
                                       }
                                     }
                                     else
                                       if ( done )
                                         incoming[ achiId ].bits.FlushFast();
                                   }

                                   {
                                     CLightweightCriticalSection cs( &critSec );
                                     achievements = incoming;
                                   }
                                 }

                                 beingFetched = false;
                                 fetched = true;
                               } );
  }

  if ( !beingFetched && fetchThread.joinable() )
  {
    lastFetchTime = globalTimer.GetTime();
    fetchThread.join();
  }
}

void Achievements::WaitForFetch()
{
  if ( fetchThread.joinable() )
    fetchThread.join();
}
