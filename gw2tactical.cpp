#include "gw2tactical.h"
#include "MumbleLink.h"
#include "Bedrock/UtilLib/PNGDecompressor.h"
#include "OverlayConfig.h"
#include <mmsystem.h>
#include "TrailLogger.h"
#include "Bedrock/UtilLib/miniz.c"
#include "WvW.h"
#include "Language.h"
#include "GW2API.h"
#include "OverlayApplication.h"

#include "Bedrock/UtilLib/jsonxx.h"
using namespace jsonxx;

WBATLASHANDLE DefaultIconHandle = -1;
WBATLASHANDLE forbiddenIconHandle = -1;
CSize forbiddenIconSize;
CDictionary<CString, WBATLASHANDLE> MapIcons;
GW2TacticalCategory CategoryRoot;
CDictionaryEnumerable<CString, GW2TacticalCategory*> CategoryMap;
TS32 useMetricDisplay = 0;

CString emptyString;
CArray<CString> stringArray;
float GetUIScale();

float globalOpacity = 1.0f;
float minimapOpacity = 1.0f;

TS32 AddStringToMap( const CString& string )
{
  if ( !string.Length() )
    return -1;
  return stringArray.AddUnique( string );
}

CString& GetStringFromMap( TS32 idx )
{
  if (idx<0 || idx>=stringArray.NumItems() )
    return emptyString;
  return stringArray[ idx ];
}

TS32 tacoStartTime = timeGetTime();

TS32 GetTime()
{
  return timeGetTime() - tacoStartTime;
}

void UpdateWvWStatus();

void FindClosestRouteMarkers( TBOOL force )
{
  for ( TS32 x = 0; x < Routes.NumItems(); x++ )
  {
    if ( !force && Routes[ x ].activeItem != -1 )
      continue;

    if ( Routes[ x ].MapID == mumbleLink.mapID && Routes[ x ].hasResetPos && ( Routes[ x ].resetPos - mumbleLink.charPosition ).Length() < Routes[ x ].resetRad )
      Routes[ x ].activeItem = 0;

    TF32 closestdist = 1000000000;
    TS32 closest = -1;

    for ( TS32 y = 0; y < Routes[ x ].route.NumItems(); y++ )
    {
      GUID g = Routes[ x ].route[ y ];
      if ( POIs.HasKey( g ) )
      {
        POI &p = POIs[ g ];
        if ( !p.mapID == mumbleLink.mapID )
          continue;

        TF32 dist = ( p.position - mumbleLink.charPosition ).Length();
        if ( dist < closestdist )
        {
          closestdist = dist;
          closest = y;
          Routes[ x ].activeItem = y;
        }
      }
    }
  }
}

GW2TacticalCategory *GetCategory( CString s )
{
  s.ToLower();
  if ( CategoryMap.HasKey( s ) )
    return CategoryMap[ s ];
  return nullptr;
}

CDictionary<CString, mz_zip_archive*> zipDict;

void FlushZipDict()
{
  for ( int x = 0; x < zipDict.NumItems(); x++ )
  {
    auto i = zipDict.GetByIndex( x );
    if ( i )
      mz_zip_reader_end( i );
  }
  zipDict.FreeAll();
}

mz_zip_archive* OpenZipFile( const CString& zipFile )
{
  if ( !zipDict.HasKey( zipFile ) )
  {
    mz_zip_archive* zip = new mz_zip_archive;
    memset( zip, 0, sizeof( mz_zip_archive ) );

    if ( !mz_zip_reader_init_file( zip, zipFile.GetPointer(), 0 ) )
    {
      LOG_ERR("[GW2TacO] Failed to open zip archive %s", zipFile.GetPointer());
      zipDict[ zipFile ] = nullptr;
      delete zip;
    }
    else
    {
      zipDict[ zipFile ] = zip;
    }
  }

  return zipDict[ zipFile ];
}

WBATLASHANDLE GetMapIcon( CWBApplication *App, CString &filename, const CString &zipFile, const CString& categoryZip )
{
  for ( TU32 x = 0; x < filename.Length(); x++ )
    if ( filename[ x ] == '\\' ) filename[ x ] = '/';

  CString s = ( zipFile.Length() ? ( zipFile + "\\" ) : "" ) + filename;
  s.ToLower();

  if ( MapIcons.HasKey( s ) )
    return MapIcons[ s ];

  if ( DefaultIconHandle == -1 )
  {
    DefaultIconHandle = App->GetSkin()->GetElement( App->GetSkin()->GetElementID( CString( _T( "defaulticon" ) ) ) )->GetHandle();
  }

  if ( forbiddenIconHandle == -1 )
  {
    forbiddenIconHandle = App->GetSkin()->GetElement( App->GetSkin()->GetElementID( CString( _T( "forbiddenicon" ) ) ) )->GetHandle();
    forbiddenIconSize = App->GetAtlas()->GetSize( forbiddenIconHandle );
  }

  if ( zipFile.Length() || categoryZip.Length() )
  {
    // we didn't find an entry from within the zip file, try to load it

    for (int x = 0; x < 2; x++)
    {
      if (!zipFile.Length() && x == 0)
        continue;

      if (!categoryZip.Length() && x == 1)
        continue;

      mz_zip_archive* zip = x == 0 ? OpenZipFile(zipFile) : OpenZipFile(categoryZip);

      if (zip)
      {
        int idx = mz_zip_reader_locate_file(zip, filename.GetPointer(), nullptr, 0);
        if (idx >= 0 && !mz_zip_reader_is_file_a_directory(zip, idx))
        {
          mz_zip_archive_file_stat stat;
          if (mz_zip_reader_file_stat(zip, idx, &stat) && stat.m_uncomp_size > 0)
          {
            TU8* data = new TU8[(TS32)stat.m_uncomp_size];

            if (mz_zip_reader_extract_to_mem(zip, idx, data, (TS32)stat.m_uncomp_size, 0))
            {
              TU8* imageData = nullptr;
              TS32 xres, yres;
              if (DecompressPNG(data, (TS32)stat.m_uncomp_size, imageData, xres, yres))
              {
                ARGBtoABGR(imageData, xres, yres);

                auto handle = App->GetAtlas()->AddImage(imageData, xres, yres, CRect(0, 0, xres, yres));
                //ExportPNG( imageData, xres, yres, false, CString::Format( "debugexport\\%d.png", cntr++ ) );

                SAFEDELETEA(imageData);
                MapIcons[s] = handle;

                delete[] data;
                return handle;
              }
              else
                LOG_ERR("[GWTacO] Failed to decompress png %s form archive %s", filename.GetPointer(), x == 0 ? zipFile.GetPointer() : categoryZip.GetPointer());
            }
            delete[] data;
          }
        }
      }
    }

    // zipfile load failed, fall back to regular load and add it as an alias
    WBATLASHANDLE handle = GetMapIcon( App, filename, "", "" );
    if ( handle == DefaultIconHandle )
      return handle;
    MapIcons[ s ] = handle;
    return handle;
  }

  CStreamReaderMemory f;
  if (!f.Open(s.GetPointer()) && !f.Open((CString("POIs\\") + s).GetPointer()))
  {
    LOG_ERR("[GWTacO] Failed to open image %s", s.GetPointer());
    return DefaultIconHandle;
  }

  TU8 *imageData = nullptr;
  TS32 xres, yres;
  if (!DecompressPNG(f.GetData(), (TS32)f.GetLength(), imageData, xres, yres))
  {
    LOG_ERR("[GWTacO] Failed to decompress png %s", s.GetPointer());
    return DefaultIconHandle;
  }

  ARGBtoABGR( imageData, xres, yres );

  auto handle = App->GetAtlas()->AddImage( imageData, xres, yres, CRect( 0, 0, xres, yres ) );

  SAFEDELETEA( imageData );
  MapIcons[ s ] = handle;
  //LOG_DBG( "Created icon with handle %d from %s", handle, s.GetPointer() );
  return handle;
}

CDictionaryEnumerable<GUID, POI> POIs;
CDictionaryEnumerable<POIActivationDataKey, POIActivationData> ActivationData;
extern CDictionaryEnumerable<GUID, GW2Trail*> trails;
CArray<POIRoute> Routes;

TU32 DictionaryHash( const GUID &i )
{
  TU8 *dta = (TU8*)( &i );
  TU32 Hash = 5381;
  for ( int x = 0; x < sizeof( GUID ); x++ )
    Hash = ( ( Hash << 5 ) + Hash ) + dta[ x ]; // hash * 33 + c
  return Hash;
}

TU32 DictionaryHash( const POIActivationDataKey &i )
{
  TU8 *dta = (TU8*)( &i );
  TU32 Hash = 5381;
  for ( int x = 0; x < sizeof( POIActivationDataKey ); x++ )
    Hash = ( ( Hash << 5 ) + Hash ) + dta[ x ]; // hash * 33 + c
  return Hash;
}

float distPointPlane( CVector3 vPoint, CPlane plane )
{
  return vPoint*plane.Normal + plane.D;
}

float distRayPlane( CVector3 vRayOrigin, CVector3 vnRayVector, CVector3 vnPlaneNormal, float planeD )
{
  float cosAlpha;
  float deltaD;

  cosAlpha = vnRayVector *  vnPlaneNormal;
  // parallel to the plane (alpha=90)
  if ( cosAlpha == 0 ) return -1.0f;
  deltaD = planeD - vRayOrigin*vnPlaneNormal;

  return ( deltaD / cosAlpha );
}

bool testfrustum( CVector3 c, CPlane planes[ 4 ], int skip )
{
  bool v = c.z > 0;
  for ( int x = 0; x < 4; x++ )
    if ( x != skip )
      v = v && ( distPointPlane( c, planes[ x ] ) < 0 );
  return v;
}

void SetRotate( CMatrix4x4 &m, float x, float y, float z, float phi )
{
  m.Rotation( CQuaternion::FromAxisAngle( CVector3( x, y, z ), phi ) );
}

CVector3 GW2TacticalDisplay::ProjectTacticalPos( CVector3 pos, TF32 fov, TF32 asp )
{
  CVector3 p = pos;
  TF32 length = p.Length();

  float yfov = fov / 2.0f;

  CVector3 fln, frn, fun, fdn;
  CMatrix4x4 rotm;

  float xfov = atan( asp*tan( yfov ) );

  rotm = CMatrix4x4::Rotation( CQuaternion::FromAxisAngle( CVector3( 0, 1, 0 ), -xfov ) );
  fln = CVector3( -1, 0, 0 ) * rotm;
  rotm = CMatrix4x4::Rotation( CQuaternion::FromAxisAngle( CVector3( 0, 1, 0 ), xfov ) );
  frn = CVector3( 1, 0, 0 ) * rotm;
  rotm = CMatrix4x4::Rotation( CQuaternion::FromAxisAngle( CVector3( 1, 0, 0 ), -yfov ) );
  fun = CVector3( 0, 1, 0 ) * rotm;
  rotm = CMatrix4x4::Rotation( CQuaternion::FromAxisAngle( CVector3( 1, 0, 0 ), yfov ) );
  fdn = CVector3( 0, -1, 0 ) * rotm;

  CPlane fplanes[ 4 ];
  fplanes[ 0 ] = CPlane( CVector3( 0, 0, 0 ), fln );
  fplanes[ 1 ] = CPlane( CVector3( 0, 0, 0 ), frn );
  fplanes[ 2 ] = CPlane( CVector3( 0, 0, 0 ), fun );
  fplanes[ 3 ] = CPlane( CVector3( 0, 0, 0 ), fdn );

  float ax = atan2( p.x, p.z );
  float ay = atan2( p.y, p.z );

  if ( !testfrustum( p, fplanes, -1 ) )
  {
    CVector3 o = p;
    CVector3 res[ 4 ];
    float di[ 4 ];

    CVector3 vn = ( p - CVector3( 0, 0, 1 ) ).Normalized();
    di[ 0 ] = distRayPlane( p, vn, fln, 0 );
    di[ 1 ] = distRayPlane( p, vn, frn, 0 );
    di[ 2 ] = distRayPlane( p, vn, fun, 0 );
    di[ 3 ] = distRayPlane( p, vn, fdn, 0 );

    bool ok[ 4 ];
    float m = 0;
    for ( int x = 0; x < 4; x++ )
    {
      res[ x ] = o + vn*di[ x ];
      ok[ x ] = testfrustum( res[ x ], fplanes, x );
      if ( ok[ x ] )
      {
        di[ x ] = ( o - res[ x ] ).Length();
        m = di[ x ];
        p = res[ x ];
      }
    }

    for ( int x = 0; x < 4; x++ )
      if ( ok[ x ] && di[ x ] < m )
      {
        p = res[ x ];
        m = di[ x ];
      }

  }

  return p.Normalized()*length;
}

float GetMapFade()
{
#define MAPFADELENGTH 250

  int lastMapTime = globalTimer.GetTime() - mumbleLink.lastMapChangeTime;
  if ( mumbleLink.isMapOpen && lastMapTime > MAPFADELENGTH )
    return 0.0f;

  float mapFade = 1.0f;

  if ( mumbleLink.isMapOpen )
  {
    lastMapTime = MAPFADELENGTH - lastMapTime;
    mapFade = min( 1.0f, lastMapTime / float( MAPFADELENGTH ) );
  }

  return mapFade;
}

void GW2TacticalDisplay::FetchAchievements()
{
  if (GW2::apiKeyManager.GetStatus() != GW2::APIKeyManager::Status::OK)
    return;

  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if ( key && key->valid && ( GetTime() - lastFetchTime > 150000 || !lastFetchTime ) && !beingFetched && !fetchThread.joinable() )
  {
    beingFetched = true;
    fetchThread = std::thread( [ this, key ]()
    {
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
          incoming[achiId].done = done;

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
          CLightweightCriticalSection cs( &dataWriteCritSec );
          achievements = incoming;
        }
      }

      beingFetched = false;
      achievementsFetched = true;
    } );
  }

  if ( !beingFetched && fetchThread.joinable() )
  {
    lastFetchTime = GetTime();
    fetchThread.join();
  }
}

void GW2TacticalDisplay::InsertPOI( POI& poi )
{
  if ( poi.mapID != mumbleLink.mapID )
    return;

  if ( poi.routeMember )
  {
    TBOOL discard = true;

    for ( TS32 y = 0; y < Routes.NumItems(); y++ )
    {
      if ( Routes[ y ].activeItem >= 0 )
      {
        if ( Routes[ y ].route[ Routes[ y ].activeItem ] == poi.guid )
        {
          discard = false;
          break;
        }
      }
    }

    if ( discard )
      return;
  }

  poi.cameraSpacePosition = CVector4( poi.position.x, poi.position.y + poi.typeData.height, poi.position.z, 1.0f )*cam;

  minimapPOIs.Add( &poi );

  if ( poi.typeData.fadeFar >= 0 && poi.typeData.fadeNear >= 0 )
  {
    TF32 dist = WorldToGameCoords( poi.cameraSpacePosition.Length() );
    if ( dist > poi.typeData.fadeFar )
      return;
  }

  mapPOIs.Add( &poi );
}

void GW2TacticalDisplay::DrawPOI( CWBDrawAPI *API, const tm& ptm, const time_t& currtime, POI& poi, bool drawDistance, CString& infoText)
{
  TBOOL drawCountdown = false;
  TS32 timeLeft;
  float alphaMultiplier = 1;

  if ( !poi.IsVisible( ptm, currtime, achievementsFetched, achievements, dataWriteCritSec ) )
    return;

  if ( poi.typeData.behavior == POIBehavior::WvWObjective )
  {
    time_t elapsedtime = currtime - poi.lastUpdateTime;
    if ( elapsedtime < 300 )
    {
      timeLeft = (TS32)( 300 - elapsedtime );
      drawCountdown = true;
    }
  }

  if ( poi.typeData.behavior == POIBehavior::ReappearAfterTimer )
  {
    time_t elapsedtime = currtime - poi.lastUpdateTime;
    if ( elapsedtime < poi.typeData.resetLength )
    {
      if ( poi.typeData.bits.hasCountdown )
      {
        timeLeft = (TS32)( poi.typeData.resetLength - elapsedtime );
        drawCountdown = true;
      }
      else
        return;
    }

    TF32 dist = (poi.position - mumbleLink.charPosition).Length();

    if (dist <= poi.typeData.infoRange)
    {
      if (poi.typeData.info >= 0)
        infoText += GetStringFromMap(poi.typeData.info) + "\n";
    }

    if ( !drawCountdown && ( poi.typeData.bits.autoTrigger || poi.typeData.bits.hasCountdown ) && ( dist <= poi.typeData.triggerRange ) )
    {
      //auto trigger
      POIActivationData d;
      time( &d.lastUpdateTime );
      poi.lastUpdateTime = d.lastUpdateTime;
      d.poiguid = poi.guid;

      int data = 0;
      if ( poi.typeData.behavior == POIBehavior::OncePerInstance )
        data = mumbleLink.mapInstance;
      if ( poi.typeData.behavior == POIBehavior::DailyPerChar )
        data = mumbleLink.charIDHash;
      if ( poi.typeData.behavior == POIBehavior::OncePerInstancePerChar )
        data = mumbleLink.charIDHash^mumbleLink.mapInstance;

      ActivationData[ POIActivationDataKey( poi.guid, data ) ] = d;
    }
  }

  if ( poi.routeMember && ( ( poi.position - mumbleLink.charPosition ).Length() <= poi.typeData.triggerRange ) )
  {
    for ( TS32 y = 0; y < Routes.NumItems(); y++ )
    {
      if ( Routes[ y ].activeItem < 0 )
        return;

      if ( Routes[ y ].route[ Routes[ y ].activeItem ] == poi.guid )
      {
        //progress route

        if ( Routes[ y ].backwards )
          Routes[ y ].activeItem -= 1;
        else
          Routes[ y ].activeItem += 1;

        Routes[ y ].activeItem = ( Routes[ y ].activeItem + Routes[ y ].route.NumItems() ) % Routes[ y ].route.NumItems();
      }
    }
  }

  // get alpha for map

  float mapFade = GetMapFade();

  if ( !poi.icon )
  {
    poi.icon = GetMapIcon(App, GetStringFromMap(poi.iconFile), GetStringFromMap(poi.zipFile), poi.category ? GetStringFromMap(poi.category->zipFile) : "");
    //LOG_DBG( "[GW2TacO] Icon is %s", GetStringFromMap( poi.iconFile ).GetPointer() );
  }

  WBATLASHANDLE icon = poi.icon;
  TF32 size = poi.typeData.size;
  TF32 Alpha = poi.typeData.alpha;

  auto camspace = poi.cameraSpacePosition;
  auto screenpos = camspace;

  CVector4 camspacex = camspace + CVector4( 0.5f, 0, 0, 0 )*size;

  if ( TacticalIconsOnEdge )
  {
    screenpos /= screenpos.w;
    CVector3 projpos = ProjectTacticalPos( screenpos, mumbleLink.fov, asp );
    screenpos.x = projpos.x;
    screenpos.y = projpos.y;
    screenpos.z = projpos.z;
    //screenpos.Normalize();
    //screenpos *= camspace.Length();
    camspace = screenpos;
    camspacex = camspace + CVector4( 0.5f, 0, 0, 0 )*size;
  }

  if ( !TacticalIconsOnEdge && camspace.z <= 0 ) return;

  TF32 dist = WorldToGameCoords( camspace.Length() );
  if ( poi.typeData.fadeNear >= 0 && poi.typeData.fadeFar >= 0 )
  {
    TF32 fadeAlpha = 1;

    if ( dist > poi.typeData.fadeFar )
      return;
    if ( dist > poi.typeData.fadeNear )
      fadeAlpha = 1 - ( dist - poi.typeData.fadeNear ) / ( poi.typeData.fadeFar - poi.typeData.fadeNear );

    Alpha *= fadeAlpha;
  }

  camspace = camspace*persp;
  camspace /= camspace.w;

  screenpos = screenpos*persp;
  screenpos /= screenpos.w;

  camspacex = camspacex*persp;
  camspacex /= camspacex.w;

  //CSize iconsize = poi.iconSize;

  int s = (int)min( poi.typeData.maxSize, max( poi.typeData.minSize, abs( ( camspacex - camspace ).x )*drawrect.Width() ) );

  //if (TacticalIconsOnEdge)
  //	s = (int)min(max(iconsize.x, iconsize.y) / 2.0f, s);

  if ( poi.typeData.behavior == POIBehavior::WvWObjective )
  {
    alphaMultiplier = max( 0, min( 1, powf( CVector2( screenpos ).Length(), 2 ) + 0.3f ) );
  }

  screenpos = screenpos*0.5 + CVector4( 0.5, 0.5, 0.5, 0.0 );

  CPoint p = CPoint( (int)( screenpos.x*drawrect.Width() ), (int)( ( 1 - screenpos.y )*drawrect.Height() ) );

  CRect rect = CRect( p - CPoint( s, s ), p + CPoint( s, s ) );

  if ( TacticalIconsOnEdge )
  {
    TS32 edge = poi.typeData.minSize;

    CPoint cp = rect.Center();
    if ( cp.x < edge )
      rect = rect + CPoint( edge - cp.x, 0 );
    if ( cp.x > drawrect.x2 - edge )
      rect = rect - CPoint( drawrect.x2 - cp.x + edge, 0 );

    if ( cp.y < edge )
      rect = rect + CPoint( 0, edge - cp.y );
    if ( cp.y > drawrect.y2 - edge )
      rect = rect - CPoint( 0, drawrect.y2 - cp.y + edge );

  }

  if ( !drawCountdown || poi.typeData.behavior == POIBehavior::WvWObjective )
  {
    CColor col = poi.typeData.color;
    if ( icon != DefaultIconHandle )
      col.A() = TU8( col.A() * Alpha * alphaMultiplier * mapFade * globalOpacity );
    else
      col.A() = TU8( col.A() * mapFade * globalOpacity);
    API->DrawAtlasElement( icon, rect, false, false, true, true, col );
  }

  if ( drawWvWNames && poi.typeData.behavior == POIBehavior::WvWObjective )
  {
    CWBFont *f = App->GetDefaultFont();
    extern CArray<WvWObjective> wvwObjectives;
    CString wvwObjectiveName;

    if ( poi.wvwObjectiveID < wvwObjectives.NumItems() )
      wvwObjectiveName = DICT( wvwObjectives[ poi.wvwObjectiveID ].nameToken, wvwObjectives[ poi.wvwObjectiveID ].name );

    if ( wvwObjectiveName.Length() )
    {
      p = f->GetTextPosition( wvwObjectiveName.GetPointer(), rect, WBTA_CENTERX, WBTA_TOP, WBTT_UPPERCASE, false ) - CPoint( 0, f->GetLineHeight() );
      for ( TS32 x = 0; x < 3; x++ )
        for ( TS32 y = 0; y < 3; y++ )
          f->Write( API, wvwObjectiveName, p + CPoint( x - 1, y - 1 ), CColor( 0, 0, 0, TU8( 255 * alphaMultiplier * globalOpacity * mapFade / 2.0f ) ), WBTT_UPPERCASE, false );
      f->Write( API, wvwObjectiveName, p, CColor( 255, 255, 0, TU8( 255 * alphaMultiplier * mapFade * globalOpacity) ), WBTT_UPPERCASE, false );
    }
  }

  if ( drawCountdown )
  {
    CWBFont *f = GetFont( GetState() );
    if ( !f )
      return;

    if ( poi.typeData.behavior == POIBehavior::WvWObjective )
      f = App->GetDefaultFont();

    CString txt;
    TS32 seconds = timeLeft % 60;
    TS32 minutes = ( timeLeft - seconds ) / 60;
    TS32 hours = ( timeLeft - seconds - minutes * 60 ) / 60;

    if ( hours )
      txt += CString::Format( "%.2d:", hours );

    if ( minutes )
      txt += CString::Format( "%.2d:", minutes );

    txt += CString::Format( "%.2d", seconds );

    TS32 offset = 0;
    if ( drawDistance )
      offset += f->GetLineHeight();

    CPoint p;
    if ( poi.typeData.behavior == POIBehavior::WvWObjective )
    {
      if ( forbiddenIconHandle != -1 )
      {
        CColor col = 0xffffffff;
        if ( icon != DefaultIconHandle )
          col.A() = TU8( col.A() * Alpha * alphaMultiplier * mapFade * globalOpacity);
        else
          col.A() = TU8( col.A() * mapFade * globalOpacity);
        API->DrawAtlasElement( forbiddenIconHandle, rect, false, false, true, true, col );
      }

      p = f->GetTextPosition( txt.GetPointer(), rect, WBTA_CENTERX, WBTA_BOTTOM, WBTT_NONE, false ) + CPoint( 0, f->GetLineHeight() + offset );
      for ( TS32 x = 0; x < 3; x++ )
        for ( TS32 y = 0; y < 3; y++ )
          f->Write( API, txt, p + CPoint( x - 1, y - 1 ), CColor( 0, 0, 0, TU8( 255 * alphaMultiplier * globalOpacity * mapFade / 2.0f ) ), WBTT_NONE, false );
      f->Write( API, txt, p, CColor( 255, 255, 0, TU8( 255 * alphaMultiplier * mapFade * globalOpacity) ), WBTT_NONE, false );
    }
    else
    {
      p = f->GetTextPosition( txt.GetPointer(), rect, WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, false );
      p.y += offset;
      f->Write( API, txt, p, CColor( 255, 255, 0, TU8( 255 * mapFade * globalOpacity ) ), WBTT_NONE, false );
    }
  }

  if ( drawDistance )
  {
    CWBFont *f = App->GetDefaultFont();
    if ( !f )
      return;

    if ( Alpha*alphaMultiplier > 0 )
    {
      TF32 charDist = WorldToGameCoords( ( poi.position - mumbleLink.charPosition ).Length() );

      CString txt;
      
      if ( !useMetricDisplay )
        txt = CString::Format( "%d", (TS32)charDist );
      else
      {
        charDist *= 0.0254f;
        txt = CString::Format( "%.1fm", charDist );
      }

      p = f->GetTextPosition( txt.GetPointer(), rect, WBTA_CENTERX, WBTA_BOTTOM, WBTT_NONE, false ) + CPoint( 0, f->GetLineHeight() );

      for ( TS32 x = 0; x < 3; x++ )
        for ( TS32 y = 0; y < 3; y++ )
          f->Write( API, txt, p + CPoint( x - 1, y - 1 ), CColor( 0, 0, 0, TU8( 255 * Alpha * alphaMultiplier * globalOpacity * mapFade / 2.0f ) ), WBTT_NONE, false );
      f->Write( API, txt, p, CColor( 255, 255, 255, TU8( 255 * Alpha * alphaMultiplier * mapFade * globalOpacity) ), WBTT_NONE, false );
    }
  }
}

TF32 uiScale = 1.0f;

void GW2TacticalDisplay::DrawPOIMinimap( CWBDrawAPI *API, const CRect& miniRect, CVector2& pos, const tm& ptm, const time_t& currtime, POI& poi, float alpha, float zoomLevel )
{
  if ( alpha <= 0 )
    return;
  if ( !poi.IsVisible( ptm, currtime, achievementsFetched, achievements, dataWriteCritSec ) )
    return;

  if ( !poi.typeData.bits.keepOnMapEdge && !miniRect.Contains( CPoint( TS32( pos.x ), TS32( pos.y ) ) ) )
    return;

  if ( poi.typeData.bits.keepOnMapEdge )
  {
    pos.x = min( miniRect.x2, max( miniRect.x1, pos.x ) );
    pos.y = min( miniRect.y2, max( miniRect.y1, pos.y ) );
  }

  if ( !poi.icon )
  {
    poi.icon = GetMapIcon(App, GetStringFromMap(poi.iconFile), GetStringFromMap(poi.zipFile), poi.category ? GetStringFromMap(poi.category->zipFile) : "");
    //LOG_DBG( "[GW2TacO] Icon is %s", GetStringFromMap( poi.iconFile ).GetPointer() );
  }

  TF32 poiSize = TF32( poi.typeData.miniMapSize );
  if ( poi.typeData.bits.scaleWithZoom )
    poiSize /= zoomLevel;
  poiSize *= uiScale;

  alpha *= 1.0f - max( 0.0f, min( 1.0f, ( zoomLevel - poi.typeData.miniMapFadeOutLevel ) / 2.0f ) );

  CVector2 startPoint = pos - CVector2( poiSize / 2.0f, poiSize / 2.0f );
  CPoint topLeft = CPoint( TS32( startPoint.x ), TS32( startPoint.y ) );

  CRect displayRect( topLeft, topLeft );
  displayRect.x2 = topLeft.x + TS32( poiSize );
  displayRect.y2 = topLeft.y + TS32( poiSize );

  CColor col = poi.typeData.color;
  col.A() = TU8(col.A() * alpha * minimapOpacity);


  API->DrawAtlasElement( poi.icon, displayRect, false, false, true, true, col );
}

void GW2TacticalDisplay::OnDraw( CWBDrawAPI *API )
{
  //default values
  if ( !HasConfigValue( "TacticalIconsOnEdge" ) )
    SetConfigValue( "TacticalIconsOnEdge", 1 );

  if ( !HasConfigValue( "TacticalLayerVisible" ) )
    SetConfigValue( "TacticalLayerVisible", 1 );

  if ( !HasConfigValue( "DrawWvWNames" ) )
    SetConfigValue( "DrawWvWNames", 1 );

  if ( !HasConfigValue( "TacticalDrawDistance" ) )
    SetConfigValue( "TacticalDrawDistance", 0 );

  if (!HasConfigValue("UseMetricDisplay"))
    SetConfigValue("UseMetricDisplay", 0);

  if ( !HasConfigValue( "OpacityIngame" ) )
    SetConfigValue( "OpacityIngame", 0 );

  if (!HasConfigValue("OpacityMap"))
    SetConfigValue("OpacityMap", 0);

  if (!HasConfigValue("TacticalInfoTextVisible"))
    SetConfigValue("TacticalInfoTextVisible", 1);

  int opac = GetConfigValue("OpacityIngame");
  if (opac == 0)
    globalOpacity = 1.0f;
  if (opac == 1)
    globalOpacity = 2 / 3.0f;
  if (opac == 2)
    globalOpacity = 1 / 3.0f;

  opac = GetConfigValue("OpacityMap");
  if (opac == 0)
    minimapOpacity = 1.0f;
  if (opac == 1)
    minimapOpacity = 2 / 3.0f;
  if (opac == 2)
    minimapOpacity = 1 / 3.0f;

  useMetricDisplay = GetConfigValue( "UseMetricDisplay" );

  if ( !GetConfigValue( "TacticalLayerVisible" ) )
    return;

  if ( !mumbleLink.IsValid() )
    return;

  uiScale = GetUIScale();

  if ( !HasConfigValue( "ShowMinimapMarkers" ) )
    SetConfigValue( "ShowMinimapMarkers", 1 );
  int showMinimapMarkers = GetConfigValue( "ShowMinimapMarkers" );

  if ( !HasConfigValue( "ShowBigmapMarkers" ) )
    SetConfigValue( "ShowBigmapMarkers", 1 );
  int showBigmapMarkers = GetConfigValue( "ShowBigmapMarkers" );

  if ( !HasConfigValue( "ShowInGameMarkers" ) )
    SetConfigValue( "ShowInGameMarkers", 1 );
  int showIngameMarkers = GetConfigValue( "ShowInGameMarkers" );

  FetchAchievements();
  UpdateWvWStatus();

  TacticalIconsOnEdge = GetConfigValue( "TacticalIconsOnEdge" );
  drawWvWNames = GetConfigValue( "DrawWvWNames" ) != 0;
  bool drawDistance = GetConfigValue( "TacticalDrawDistance" ) != 0;

  time_t rawtime;
  time( &rawtime );
  struct tm ptm;
  gmtime_s( &ptm, &rawtime );

  mapPOIs.FlushFast();
  minimapPOIs.FlushFast();

  drawrect = GetClientRect();

  cam.SetLookAtLH( mumbleLink.camPosition, mumbleLink.camPosition + mumbleLink.camDir, CVector3( 0, 1, 0 ) );
  persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (TF32)drawrect.Height(), 0.01f, 1000.0f );

  asp = drawrect.Width() / (TF32)drawrect.Height();

  for ( int x = 0; x < POIs.NumItems(); x++ )
  {
    auto &poi = POIs.GetByIndex( x );
    InsertPOI( poi );
  }

  extern bool wvwCanBeRendered;

  if ( wvwCanBeRendered )
    for ( int x = 0; x < wvwPOIs.NumItems(); x++ )
    {
      auto &poi = wvwPOIs.GetByIndex( x );
      InsertPOI( poi );
    }

  time_t currtime;
  time( &currtime );

  mapPOIs.Sort( []( POI **a, POI **b ) -> int { return ( *b )->cameraSpacePosition.z > ( *a )->cameraSpacePosition.z; } );

  for ( TS32 x = 0; x < Routes.NumItems(); x++ )
  {
    if ( Routes[ x ].hasResetPos && Routes[ x ].MapID == mumbleLink.mapID && ( Routes[ x ].resetPos - mumbleLink.charPosition ).Length() < Routes[ x ].resetRad )
      Routes[ x ].activeItem = 0;
  }

  CString infoText;

  if ( showIngameMarkers > 0 )
    for ( int x = 0; x < mapPOIs.NumItems(); x++ )
    {
      if ( !mapPOIs[ x ]->typeData.bits.inGameVisible && showIngameMarkers != 2 )
        continue;
      DrawPOI( API, ptm, currtime, *mapPOIs[ x ], drawDistance, infoText );
    }

  // punch hole in minimap

  CRect miniRect = GetMinimapRectangle();

  API->FlushDrawBuffer();
  API->GetDevice()->SetRenderState( ( (COverlayApp*)App )->holePunchBlendState );

  API->DrawRect( miniRect, CColor( 0, 0, 0, 0 ) );

  API->FlushDrawBuffer();
  API->SetUIRenderState();

  //API->DrawRect(miniRect, CColor(80, 80, 80, 192));

  // draw minimap trails

  GW2TrailDisplay* trails = (GW2TrailDisplay*)App->GetRoot()->FindChildByID( _T( "trail" ), _T( "gw2Trails" ) );
  if ( trails )
    trails->DrawProxy( API, true );

  // draw minimap

  float mapFade = GetMapFade();

  if ( mapFade > 0 && showMinimapMarkers > 0 )
  {
    CMatrix4x4 miniMapTrafo = mumbleLink.miniMap.BuildTransformationMatrix( miniRect, false );
    for ( int x = 0; x < minimapPOIs.NumItems(); x++ )
    {
      if ( !minimapPOIs[ x ]->typeData.bits.miniMapVisible && showMinimapMarkers != 2 )
        continue;
      if ( !minimapPOIs[ x ]->IsVisible( ptm, currtime, achievementsFetched, achievements, dataWriteCritSec ) )
        continue;

      CVector3 poiPos = minimapPOIs[ x ]->position * miniMapTrafo;
      DrawPOIMinimap( API, miniRect, CVector2( poiPos.x, poiPos.y ), ptm, currtime, *minimapPOIs[ x ], mapFade, mumbleLink.miniMap.mapScale );
    }
  }

  if ( mumbleLink.isMapOpen && mapFade < 1.0 && showBigmapMarkers > 0 )
  {
    miniRect = GetClientRect();
    CMatrix4x4 miniMapTrafo = mumbleLink.bigMap.BuildTransformationMatrix( miniRect, true );
    for ( int x = 0; x < minimapPOIs.NumItems(); x++ )
    {
      if ( !minimapPOIs[ x ]->typeData.bits.bigMapVisible && showBigmapMarkers != 2 )
        continue;
      if ( !minimapPOIs[ x ]->IsVisible( ptm, currtime, achievementsFetched, achievements, dataWriteCritSec ) )
        continue;

      CVector3 poiPos = minimapPOIs[ x ]->position * miniMapTrafo;
      DrawPOIMinimap( API, miniRect, CVector2( poiPos.x, poiPos.y ), ptm, currtime, *minimapPOIs[ x ], 1.0f - mapFade, mumbleLink.bigMap.mapScale );
    }
  }

  if (GetConfigValue("TacticalInfoTextVisible"))
  {
    auto font = GetApplication()->GetRoot()->GetFont(GetState());
    TS32 width = font->GetWidth(infoText);
    font->Write(API, infoText, CPoint(int((GetClientRect().Width() - width) / 2.0f), int(GetClientRect().Height() * 0.15f)));
  }

}

GW2TacticalDisplay::GW2TacticalDisplay( CWBItem *Parent, CRect Position ) : CWBItem( Parent, Position )
{

}

GW2TacticalDisplay::~GW2TacticalDisplay()
{

}

CWBItem *GW2TacticalDisplay::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new GW2TacticalDisplay( Root, Pos );
}

TBOOL GW2TacticalDisplay::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

void GW2TacticalDisplay::RemoveUserMarkersFromMap()
{
  if (!mumbleLink.IsValid())
    return;

  for (TS32 x = 0; x < POIs.NumItems(); x++)
  {
    if (POIs.GetByIndex(x).mapID == mumbleLink.mapID)
    {
      POIs.DeleteByIndex(x);
      x--;
    }

  }

  ExportPOIS();
}

TBOOL FindSavedCategory( GW2TacticalCategory *t )
{
  if ( t->KeepSaveState )
    return true;
  for ( TS32 x = 0; x < t->children.NumItems(); x++ )
    if ( FindSavedCategory( t->children[ x ] ) ) return true;
  return false;
}

void ExportSavedCategories( CXMLNode *n, GW2TacticalCategory *t )
{
  if ( !FindSavedCategory( t ) )
    return;
  auto nn = n->AddChild( "MarkerCategory" );
  nn.SetAttribute( "name", t->name.GetPointer() );
  if ( t->name != t->displayName )
    nn.SetAttribute( "DisplayName", t->displayName.GetPointer() );
  t->data.Write( &nn );
  for ( TS32 x = 0; x < t->children.NumItems(); x++ )
    ExportSavedCategories( &nn, t->children[ x ] );
}

void ExportPOI( CXMLNode *n, POI &p )
{
  CXMLNode* t = &n->AddChild( _T( "POI" ) );
  t->SetAttributeFromInteger( "MapID", p.mapID );
  t->SetAttributeFromFloat( "xpos", p.position.x );
  t->SetAttributeFromFloat( "ypos", p.position.y );
  t->SetAttributeFromFloat( "zpos", p.position.z );
  //if ( p.Name.Length() )
  //  t->SetAttribute( "text", p.Name.GetPointer() );
  if ( p.Type.Length() )
    t->SetAttribute( "type", p.Type.GetPointer() );
  t->SetAttribute( "GUID", CString::EncodeToBase64( ( TU8* )&( p.guid ), sizeof( GUID ) ).GetPointer() );
  p.typeData.Write( t );
}

void ExportTrail( CXMLNode *n, GW2Trail& p )
{
  CXMLNode* t = &n->AddChild( _T( "Trail" ) );
  if ( p.Type.Length() )
    t->SetAttribute( "type", p.Type.GetPointer() );
  t->SetAttribute( "GUID", CString::EncodeToBase64( ( TU8* )&( p.guid ), sizeof( GUID ) ).GetPointer() );
  p.typeData.Write( t );
}

void ExportPOIS()
{
  CXMLDocument d;
  CXMLNode& root = d.GetDocumentNode();
  root = root.AddChild( "OverlayData" );

  for ( TS32 x = 0; x < CategoryRoot.children.NumItems(); x++ )
    ExportSavedCategories( &root, CategoryRoot.children[ x ] );

  CXMLNode* n = &root.AddChild( "POIs" );

  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    auto &p = POIs.GetByIndex( x );
    if ( !p.External && !p.routeMember )
      ExportPOI( n, p );
  }

  for ( TS32 x = 0; x < trails.NumItems(); x++ )
  {
    auto &p = trails.GetByIndex( x );
    if ( !p->External )
      ExportTrail( n, *p );
  }

  for ( TS32 x = 0; x < Routes.NumItems(); x++ )
  {
    if ( Routes[ x ].external )
      continue;

    CXMLNode* t = &n->AddChild( _T( "Route" ) );
    t->SetAttribute( "Name", Routes[ x ].name.GetPointer() );
    t->SetAttributeFromInteger( "BackwardDirection", (TS32)Routes[ x ].backwards );

    for ( TS32 y = 0; y < Routes[ x ].route.NumItems(); y++ )
    {
      if ( POIs.HasKey( Routes[ x ].route[ y ] ) )
        ExportPOI( t, POIs[ Routes[ x ].route[ y ] ] );
    }
  }

  d.SaveToFile( "poidata.xml" );
}

GUID LoadGUID( CXMLNode &n )
{
  CString guidb64 = n.GetAttributeAsString( _T( "GUID" ) );

  TU8 *Data = NULL;
  TS32 Size = 0;
  guidb64.DecodeBase64( Data, Size );

  GUID guid;

  if ( Size == sizeof( GUID ) )
    memcpy( &guid, Data, sizeof( GUID ) );
  else
    CoCreateGuid( &guid );

  SAFEDELETEA( Data );

  return guid;
}

void RecursiveImportPOIType( CXMLNode &root, GW2TacticalCategory *Root, CString currentCategory, MarkerTypeData &defaults, TBOOL KeepSaveState, const CString& zipFile)
{
  for ( TS32 x = 0; x < root.GetChildCount( "MarkerCategory" ); x++ )
  {
    auto &n = root.GetChild( "MarkerCategory", x );
    if ( !n.HasAttribute( "name" ) )
      continue;

    CString name = n.GetAttribute( "name" );

    for ( TS32 x = 0; x < (TS32)name.Length(); x++ )
      if ( !isalnum( name.GetPointer()[ x ] ) && name.GetPointer()[ x ] != '.' )
        name.GetPointer()[ x ] = '_';

    CString displayName;
    CString newCatName = currentCategory;

    CStringArray nameExploded = name.Explode( "." );

    GW2TacticalCategory *c = nullptr;

    for ( TS32 y = 0; y < nameExploded.NumItems(); y++ )
    {
      GW2TacticalCategory *Root2 = Root;

      if ( newCatName.Length() )
        newCatName += ".";
      newCatName += nameExploded[ y ];
      newCatName.ToLower();

      c = GetCategory( newCatName );

      if ( !c )
      {
        c = new GW2TacticalCategory();
        c->name = nameExploded[ y ];
        c->data = defaults;
        CategoryMap[ newCatName ] = c;
        Root2->children += c;
        c->Parent = Root2;
        Root2 = c;
        displayName = c->name;

        c->name.ToLower();
      }
    }

    if ( !c )
      continue;

    if ( !c->displayName.Length() )
      c->displayName = displayName;

    if ( n.HasAttribute( "DisplayName" ) )
    {
      displayName = n.GetAttribute( "DisplayName" );
      c->displayName = displayName;
      localization->ProcessStringForUsedGlyphs( displayName );
    }

    if ( n.HasAttribute( "IsSeparator" ) )
    {
      int separator = 0;
      n.GetAttributeAsInteger( "IsSeparator", &separator );
      c->IsOnlySeparator = separator;
    }

    c->data.Read( n, KeepSaveState );
    c->zipFile = AddStringToMap(zipFile);
    c->KeepSaveState = KeepSaveState;

    RecursiveImportPOIType( n, c, newCatName, c->data, KeepSaveState, zipFile );
  }
}

void ImportPOITypes()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "categorydata.xml" ) ) return;

  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "OverlayData" );

  CategoryMap.Flush();
  CategoryRoot.children.FreeArray();
  RecursiveImportPOIType( root, &CategoryRoot, CString(), MarkerTypeData(), false, CString() );
}

void ImportPOI( CWBApplication *App, CXMLNode &t, POI &p, const CString& zipFile )
{
  if ( t.HasAttribute( "MapID" ) ) t.GetAttributeAsInteger( "MapID", &p.mapID );
  if ( t.HasAttribute( "xpos" ) ) t.GetAttributeAsFloat( "xpos", &p.position.x );
  if ( t.HasAttribute( "ypos" ) ) t.GetAttributeAsFloat( "ypos", &p.position.y );
  if ( t.HasAttribute( "zpos" ) ) t.GetAttributeAsFloat( "zpos", &p.position.z );
  if ( t.HasAttribute( "icon" ) ) t.GetAttributeAsInteger( "icon", &p.icon );
  //if ( t.HasAttribute( "text" ) ) p.Name = t.GetAttributeAsString( "text" );
  if ( t.HasAttribute( "type" ) ) p.Type = t.GetAttributeAsString( "type" );

  if ( !t.HasAttribute( "GUID" ) )
    CoCreateGuid( &p.guid );
  else
    p.guid = LoadGUID( t );

  p.zipFile = AddStringToMap( zipFile );

  auto *td = GetCategory( p.Type );
  if ( td )
    p.SetCategory( App, td );

  p.typeData.Read( t, true );

  p.iconFile = p.typeData.iconFile;
  //p.icon = GetMapIcon( App, p.typeData.iconFile, zipFile );
  //p.iconSize = App->GetAtlas()->GetSize( p.icon );
}

TBOOL ImportTrail( CWBApplication *App, CXMLNode &t, GW2Trail &p, const CString& zipFile )
{
  p.zipFile = zipFile;

  if ( t.HasAttribute( "type" ) ) p.Type = t.GetAttributeAsString( "type" );

  if ( !t.HasAttribute( "GUID" ) )
    CoCreateGuid( &p.guid );
  else
    p.guid = LoadGUID( t );

  auto *td = GetCategory( p.Type );
  if ( td )
    p.SetCategory( App, td );

  p.typeData.Read( t, true );

  return p.Import( GetStringFromMap( p.typeData.trailData ), zipFile );
}

void ImportPOIDocument( CWBApplication *App, CXMLDocument& d, TBOOL External, const CString& zipFile )
{
  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "OverlayData" );

  RecursiveImportPOIType( root, &CategoryRoot, CString(), MarkerTypeData(), !External, zipFile );

  if ( root.GetChildCount( "POIs" ) )
  {
    CXMLNode n = root.GetChild( "POIs" );

    //LOG_DBG("Reading %d pois", n.GetChildCount("POI"));

    if ( n.GetChildCount( "POI" ) > 0 )
    {
      CXMLNode t = n.GetChild( "POI", 0 );
      do
      {
        POI p;
        ImportPOI( App, t, p, zipFile );
        p.External = External;
        POIs[ p.guid ] = p;
      } while ( t.Next( t, "POI" ) );
    }

    for ( TS32 x = 0; x < n.GetChildCount( "Route" ); x++ )
    {
      CXMLNode rn = n.GetChild( "Route", x );
      POIRoute r;
      if ( rn.HasAttribute( "Name" ) ) r.name = rn.GetAttributeAsString( "Name" );
      TS32 b = false;
      if ( rn.HasAttribute( "BackwardDirection" ) ) rn.GetAttributeAsInteger( "BackwardDirection", &b );
      if ( rn.HasAttribute( "MapID" ) ) rn.GetAttributeAsInteger( "MapID", &r.MapID );
      r.backwards = b;
      r.external = External;
      if ( rn.HasAttribute( "resetposx" ) &&
           rn.HasAttribute( "resetposy" ) &&
           rn.HasAttribute( "resetposz" ) &&
           rn.HasAttribute( "resetrange" ) )
      {
        rn.GetAttributeAsFloat( "resetposx", &r.resetPos.x );
        rn.GetAttributeAsFloat( "resetposy", &r.resetPos.y );
        rn.GetAttributeAsFloat( "resetposz", &r.resetPos.z );
        rn.GetAttributeAsFloat( "resetrange", &r.resetRad );
        r.hasResetPos = true;
      }

      if ( rn.GetChildCount( "POI" ) > 0 )
      {
        CXMLNode t = rn.GetChild( "POI", 0 );
        do
        {
          POI p;
          ImportPOI( App, t, p, zipFile );
          p.External = External;
          p.routeMember = true;
          POIs[ p.guid ] = p;
          r.route += p.guid;
        } while ( t.Next( t, "POI" ) );
      }

      Routes.Add( r );
    }

    if ( n.GetChildCount( "Trail" ) > 0 )
    {
      CXMLNode t = n.GetChild( "Trail", 0 );
      do
      {
        GW2Trail* p = new GW2Trail();
        if ( ImportTrail( App, t, *p, zipFile ) )
        {
          p->External = External;
          trails[ p->guid ] = p;
        }
      } while ( t.Next( t, "Trail" ) );
    }
  }
}

void ImportPOIFile( CWBApplication *App, CString s, TBOOL External )
{
  CXMLDocument d;
  if ( !d.LoadFromFile( s.GetPointer() ) ) return;
  ImportPOIDocument( App, d, External, CString( "" ) );
}

void ImportPOIString( CWBApplication *App, const CString& data, const CString& zipFile )
{
  CXMLDocument d;
  if ( !d.LoadFromString( data ) ) return;
  ImportPOIDocument( App, d, true, zipFile );
}

void ImportMarkerPack( CWBApplication* App, const CString& zipFile )
{
  mz_zip_archive* zip = OpenZipFile( zipFile );
  if ( !zip )
    return;

  for ( TU32 x = 0; x < mz_zip_reader_get_num_files( zip ); x++ )
  {
    mz_zip_archive_file_stat stat;
    if ( !mz_zip_reader_file_stat( zip, x, &stat ) )
      continue;

    if ( mz_zip_reader_is_file_a_directory( zip, x ) )
      continue;

    if ( stat.m_uncomp_size <= 0 )
      continue;

    CString fileName( stat.m_filename );
    fileName.ToLower();
    if ( fileName.Find( ".xml" ) != fileName.Length() - 4 )
      continue;

    TU8* data = new TU8[ (TS32)stat.m_uncomp_size ];

    if ( !mz_zip_reader_extract_to_mem( zip, x, data, (TS32)stat.m_uncomp_size, 0 ) )
    {
      delete[] data;
      continue;
    }

    CString doc( (TS8*)data, (TU32)stat.m_uncomp_size );
    ImportPOIString( App, doc, zipFile );

    delete[] data;
  }
}

void ImportPOIS( CWBApplication *App )
{
  FlushZipDict();
  ImportPOITypes();

  POIs.Flush();
  Routes.Flush();

  {
    CFileList list;
    list.ExpandSearch( "*.xml", "POIs", false );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportPOIFile( App, list.Files[ x ].Path + list.Files[ x ].FileName, true );
  }

  {
    CFileList list;
    list.ExpandSearch( "*.zip", "POIs", false );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportMarkerPack( App, list.Files[ x ].Path + list.Files[ x ].FileName );
  }

  {
    CFileList list;
    list.ExpandSearch( "*.taco", "POIs", false );
    for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
      ImportMarkerPack( App, list.Files[ x ].Path + list.Files[ x ].FileName );
  }

  ImportPOIFile( App, "poidata.xml", false );

  LoadMarkerCategoryVisibilityInfo();

  //FlushZipDict();
}

void ImportPOIActivationData()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "activationdata.xml" ) ) return;

  if ( !d.GetDocumentNode().GetChildCount( "OverlayData" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "OverlayData" );

  if ( root.GetChildCount( "Activations" ) )
  {
    CXMLNode n = root.GetChild( "Activations" );

    for ( TS32 x = 0; x < n.GetChildCount( "POIActivation" ); x++ )
    {
      CXMLNode t = n.GetChild( "POIActivation", x );

      if ( !t.HasAttribute( "GUID" ) )
        continue;

      POIActivationData p;

      if ( t.HasAttribute( "lut1" ) ) t.GetAttributeAsInteger( "lut1", &( (TS32*)&p.lastUpdateTime )[ 0 ] );
      if ( t.HasAttribute( "lut2" ) ) t.GetAttributeAsInteger( "lut2", &( (TS32*)&p.lastUpdateTime )[ 1 ] );

      p.poiguid = LoadGUID( t );

      p.uniqueData = 0;
      if ( t.HasAttribute( "instance" ) ) t.GetAttributeAsInteger( "instance", &p.uniqueData );

      ActivationData[ POIActivationDataKey( p.poiguid, p.uniqueData ) ] = p;

      if ( POIs.HasKey( p.poiguid ) )
        POIs[ p.poiguid ].lastUpdateTime = p.lastUpdateTime;
    }
  }
}

void ExportPOIActivationData()
{
  CXMLDocument d;
  CXMLNode& root = d.GetDocumentNode();
  root = root.AddChild( "OverlayData" );

  CXMLNode* n = &root.AddChild( "Activations" );

  for ( TS32 x = 0; x < ActivationData.NumItems(); x++ )
  {
    auto& dat = ActivationData.GetByIndex( x );
    if ( POIs.HasKey( dat.poiguid ) )
    {
      auto& poi = POIs[ dat.poiguid ];
      if ( poi.typeData.behavior == POIBehavior::AlwaysVisible )
        continue;

      //if (poi.behavior == (TS32)POIBehavior::ReappearOnDailyReset)
      //{
      //	continue;
      //}
    }

    CXMLNode* t = &n->AddChild( _T( "POIActivation" ) );
    t->SetAttributeFromInteger( "lut1", ( (TS32*)&dat.lastUpdateTime )[ 0 ] );
    t->SetAttributeFromInteger( "lut2", ( (TS32*)&dat.lastUpdateTime )[ 1 ] );
    if ( dat.uniqueData )
      t->SetAttributeFromInteger( "instance", dat.uniqueData );
    t->SetAttribute( "GUID", CString::EncodeToBase64( ( TU8* )&( dat.poiguid ), sizeof( GUID ) ).GetPointer() );
  }

  d.SaveToFile( "activationdata.xml" );
}

CString DefaultMarkerCategory = "";

void AddPOI( CWBApplication *App )
{
  if ( !mumbleLink.IsValid() ) return;
  POI poi;
  poi.position = mumbleLink.charPosition;
  poi.mapID = mumbleLink.mapID;
  poi.icon = DefaultIconHandle;
  //poi.iconSize = DefaultIconSize;

  CoCreateGuid( &poi.guid );

  auto cat = GetCategory( DefaultMarkerCategory );

  if ( poi.mapID == -1 ) return;
  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    if ( POIs.GetByIndex( x ).mapID != poi.mapID ) continue;
    CVector3 v = POIs.GetByIndex( x ).position - poi.position;
    if ( v.Length() < POIs.GetByIndex( x ).typeData.triggerRange && cat == POIs.GetByIndex( x ).category ) return;
  }

  if ( cat )
    poi.SetCategory( App, cat );

  POIs[ poi.guid ] = poi;
  ExportPOIS();
}

void DeletePOI()
{
  if ( !mumbleLink.IsValid() ) return;
  POI poi;
  poi.position = CVector3( mumbleLink.charPosition );
  poi.mapID = mumbleLink.mapID;

  if ( poi.mapID == -1 ) return;
  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    if ( POIs.GetByIndex( x ).mapID != poi.mapID ) continue;
    CVector3 v = POIs.GetByIndex( x ).position - poi.position;
    if ( v.Length() < POIs.GetByIndex( x ).typeData.triggerRange )
    {
      POIs.DeleteByIndex( x );
      ExportPOIS();
      return;
    }
  }
}

void UpdatePOI()
{
  if ( !mumbleLink.IsValid() ) return;

  if ( mumbleLink.mapID == -1 ) return;

  TBOOL found = false;

  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    auto &cpoi = POIs.GetByIndex( x );

    if ( cpoi.mapID != mumbleLink.mapID ) continue;

    CVector3 v = cpoi.position - CVector3( mumbleLink.charPosition );
    if ( v.Length() < cpoi.typeData.triggerRange )
    {
      auto& str = GetStringFromMap( cpoi.typeData.toggleCategory );
      if ( str.Length() )
      {
        GW2TacticalCategory* cat = GetCategory( str );
        if ( cat )
        {
          cat->IsDisplayed = !cat->IsDisplayed;
          SetConfigValue( ( CString( "CategoryVisible_" ) + cat->GetFullTypeName() ).GetPointer(), cat->IsDisplayed );
        }
      }

      if ( !found && cpoi.typeData.behavior != POIBehavior::AlwaysVisible )
      {
        POIActivationData d;
        time( &d.lastUpdateTime );
        cpoi.lastUpdateTime = d.lastUpdateTime;
        d.poiguid = cpoi.guid;

        d.uniqueData = 0;
        if ( cpoi.typeData.behavior == POIBehavior::OncePerInstance )
          d.uniqueData = mumbleLink.mapInstance;
        if ( cpoi.typeData.behavior == POIBehavior::DailyPerChar )
          d.uniqueData = mumbleLink.charIDHash;
        if ( cpoi.typeData.behavior == POIBehavior::OncePerInstancePerChar )
          d.uniqueData = mumbleLink.charIDHash^mumbleLink.mapInstance;

        ActivationData[ POIActivationDataKey( cpoi.guid, d.uniqueData ) ] = d;
        ExportPOIActivationData();
        found = true;
      }
    }
  }
  //ExportPOIS();
}

MarkerTypeData::MarkerTypeData()
{
  memset( &bits, 0, sizeof( bits ) );
  bits.miniMapVisible = true;
  bits.bigMapVisible = true;
  bits.inGameVisible = true;
  bits.scaleWithZoom = true;
}

void MarkerTypeData::Read( CXMLNode &n, TBOOL StoreSaveState )
{
  TBOOL _iconFileSaved = n.HasAttribute( "iconFile" );
  TBOOL _sizeSaved = n.HasAttribute( "iconSize" );
  TBOOL _alphaSaved = n.HasAttribute( "alpha" );
  TBOOL _fadeNearSaved = n.HasAttribute( "fadeNear" );
  TBOOL _fadeFarSaved = n.HasAttribute( "fadeFar" );
  TBOOL _heightSaved = n.HasAttribute( "heightOffset" );
  TBOOL _behaviorSaved = n.HasAttribute( "behavior" );
  TBOOL _resetLengthSaved = n.HasAttribute( "resetLength" );
  //TBOOL _resetOffsetSaved = n.HasAttribute( "resetOffset" );
  TBOOL _autoTriggerSaved = n.HasAttribute( "autoTrigger" );
  TBOOL _hasCountdownSaved = n.HasAttribute( "hasCountdown" );
  TBOOL _triggerRangeSaved = n.HasAttribute( "triggerRange" );
  TBOOL _minSizeSaved = n.HasAttribute( "minSize" );
  TBOOL _maxSizeSaved = n.HasAttribute( "maxSize" );
  TBOOL _colorSaved = n.HasAttribute( "color" );
  TBOOL _trailDataSaved = n.HasAttribute( "trailData" );
  TBOOL _animSpeedSaved = n.HasAttribute( "animSpeed" );
  TBOOL _textureSaved = n.HasAttribute( "texture" );
  TBOOL _trailScaleSaved = n.HasAttribute( "trailScale" );
  TBOOL _toggleCategorySaved = n.HasAttribute( "toggleCategory" );
  TBOOL _achievementIdSaved = n.HasAttribute( "achievementId" );
  TBOOL _achievementBitSaved = n.HasAttribute( "achievementBit" );
  TBOOL _miniMapVisibleSaved = n.HasAttribute( "miniMapVisibility" );
  TBOOL _bigMapVisibleSaved = n.HasAttribute( "mapVisibility" );
  TBOOL _inGameVisibleSaved = n.HasAttribute( "inGameVisibility" );
  TBOOL _scaleWithZoomSaved = n.HasAttribute( "scaleOnMapWithZoom" );
  TBOOL _miniMapSizeSaved = n.HasAttribute( "mapDisplaySize" );
  TBOOL _miniMapFadeOutLevelSaved = n.HasAttribute( "mapFadeoutScaleLevel" );
  TBOOL _keepOnMapEdgeSaved = n.HasAttribute( "keepOnMapEdge" );
  TBOOL _infoSaved = n.HasAttribute("info");
  TBOOL _infoRangeSaved = n.HasAttribute("infoRange");

  if ( StoreSaveState )
  {
    bits.iconFileSaved = _iconFileSaved;
    bits.sizeSaved = _sizeSaved;
    bits.alphaSaved = _alphaSaved;
    bits.fadeNearSaved = _fadeNearSaved;
    bits.fadeFarSaved = _fadeFarSaved;
    bits.heightSaved = _heightSaved;
    bits.behaviorSaved = _behaviorSaved;
    bits.resetLengthSaved = _resetLengthSaved;
    //bits.resetOffsetSaved = _resetOffsetSaved;
    bits.autoTriggerSaved = _autoTriggerSaved;
    bits.hasCountdownSaved = _hasCountdownSaved;
    bits.triggerRangeSaved = _triggerRangeSaved;
    bits.minSizeSaved = _minSizeSaved;
    bits.maxSizeSaved = _maxSizeSaved;
    bits.colorSaved = _colorSaved;
    bits.trailDataSaved = _trailDataSaved;
    bits.animSpeedSaved = _animSpeedSaved;
    bits.textureSaved = _textureSaved;
    bits.trailScaleSaved = _trailScaleSaved;
    bits.toggleCategorySaved = _toggleCategorySaved;
    bits.achievementIdSaved = _achievementIdSaved;
    bits.achievementBitSaved = _achievementBitSaved;
    bits.miniMapVisibleSaved = _miniMapVisibleSaved;
    bits.bigMapVisibleSaved = _bigMapVisibleSaved;
    bits.inGameVisibleSaved = _inGameVisibleSaved;
    bits.scaleWithZoomSaved = _scaleWithZoomSaved;
    bits.miniMapSizeSaved = _miniMapSizeSaved;
    bits.miniMapFadeOutLevelSaved = _miniMapFadeOutLevelSaved;
    bits.keepOnMapEdgeSaved = _keepOnMapEdgeSaved;
    bits.infoSaved = _infoSaved;
    bits.infoRangeSaved = _infoRangeSaved;
  }

  if ( _iconFileSaved )
    iconFile = AddStringToMap( n.GetAttribute( "iconFile" ) );

  if ( _sizeSaved )
    n.GetAttributeAsFloat( "iconSize", &size );

  if ( _alphaSaved )
    n.GetAttributeAsFloat( "alpha", &alpha );
  if ( _fadeNearSaved )
    n.GetAttributeAsFloat( "fadeNear", &fadeNear );
  if ( _fadeFarSaved )
    n.GetAttributeAsFloat( "fadeFar", &fadeFar );
  if ( _heightSaved )
    n.GetAttributeAsFloat( "heightOffset", &height );
  if ( _behaviorSaved )
  {
    TS32 x;
    n.GetAttributeAsInteger( "behavior", &x );
    behavior = (POIBehavior)x;
  }
  if ( _resetLengthSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "resetLength", &val );
    resetLength = val;
  }
  //if ( _resetOffsetSaved )
  //  n.GetAttributeAsInteger( "resetOffset", &resetOffset );
  if ( _autoTriggerSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "autoTrigger", &val );
    bits.autoTrigger = val != 0;
  }
  if ( _hasCountdownSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "hasCountdown", &val );
    bits.hasCountdown = val != 0;
  }
  if ( _triggerRangeSaved )
    n.GetAttributeAsFloat( "triggerRange", &triggerRange );
  if ( _minSizeSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "minSize", &val );
    minSize = val;
  }
  if ( _maxSizeSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "maxSize", &val );
    maxSize = val;
  }
  if ( _colorSaved )
  {
    CString colorStr = n.GetAttributeAsString( "color" );
    TS32 colHex = 0xffffffff;
    colorStr.Scan( "%x", &colHex );
    color = CColor( colHex );
  }
  if ( _trailDataSaved )
    trailData = AddStringToMap( n.GetAttribute( "trailData" ) );
  if ( _animSpeedSaved )
    n.GetAttributeAsFloat( "animSpeed", &animSpeed );
  if ( _textureSaved )
    texture = AddStringToMap( n.GetAttribute( "texture" ) );
  if ( _trailScaleSaved )
    n.GetAttributeAsFloat( "trailScale", &trailScale );
  if ( _toggleCategorySaved )
    toggleCategory = AddStringToMap( n.GetAttribute( "toggleCategory" ) );
  if ( _achievementIdSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "achievementId", &val );
    achievementId = val;
  }
  if ( _achievementBitSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "achievementBit", &val );
    achievementBit = val;
  }
  if ( _miniMapVisibleSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "miniMapVisibility", &val );
    bits.miniMapVisible = val != 0;
  }
  if ( _bigMapVisibleSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "mapVisibility", &val );
    bits.bigMapVisible = val != 0;
  }
  if ( _inGameVisibleSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "inGameVisibility", &val );
    bits.inGameVisible = val != 0;
  }
  if ( _scaleWithZoomSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "scaleOnMapWithZoom", &val );
    bits.scaleWithZoom = val != 0;
  }
  if ( _miniMapFadeOutLevelSaved )
    n.GetAttributeAsFloat( "mapFadeoutScaleLevel", &miniMapFadeOutLevel );

  if ( _miniMapSizeSaved )
  {
    TS32 x;
    n.GetAttributeAsInteger( "mapDisplaySize", &x );
    miniMapSize = x;
  }
  if ( _keepOnMapEdgeSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "keepOnMapEdge", &val );
    bits.keepOnMapEdge = val != 0;
  }
  if (_infoSaved)
    info = AddStringToMap(n.GetAttributeAsString("info"));
  if (_infoRangeSaved)
    n.GetAttributeAsFloat("infoRange", &infoRange);
}

void MarkerTypeData::Write( CXMLNode *n )
{
  if ( bits.iconFileSaved )
    n->SetAttribute( "iconFile", GetStringFromMap( iconFile ).GetPointer() );
  if ( bits.sizeSaved )
    n->SetAttributeFromFloat( "iconSize", size );
  if ( bits.alphaSaved )
    n->SetAttributeFromFloat( "alpha", alpha );
  if ( bits.fadeNearSaved )
    n->SetAttributeFromFloat( "fadeNear", fadeNear );
  if ( bits.fadeFarSaved )
    n->SetAttributeFromFloat( "fadeFar", fadeFar );
  if ( bits.heightSaved )
    n->SetAttributeFromFloat( "heightOffset", height );
  if ( bits.behaviorSaved )
    n->SetAttributeFromInteger( "behavior", (TS32)behavior );
  if ( bits.resetLengthSaved )
    n->SetAttributeFromInteger( "resetLength", resetLength );
  //if ( bits.resetOffsetSaved )
  //  n->SetAttributeFromInteger( "resetOffset", resetOffset );
  if ( bits.autoTriggerSaved )
    n->SetAttributeFromInteger( "autoTrigger", bits.autoTrigger );
  if ( bits.hasCountdownSaved )
    n->SetAttributeFromInteger( "hasCountdown", bits.hasCountdown );
  if ( bits.triggerRangeSaved )
    n->SetAttributeFromFloat( "triggerRange", triggerRange );
  if ( bits.minSizeSaved )
    n->SetAttributeFromInteger( "minSize", minSize );
  if ( bits.maxSizeSaved )
    n->SetAttributeFromInteger( "maxSize", maxSize );
  if ( bits.colorSaved )
    n->SetAttribute( "color", CString::Format( "%x", color.argb() ).GetPointer() );
  if ( bits.trailDataSaved )
    n->SetAttribute( "trailData", GetStringFromMap( trailData ).GetPointer() );
  if ( bits.animSpeedSaved )
    n->SetAttributeFromFloat( "animSpeed", animSpeed );
  if ( bits.textureSaved )
    n->SetAttribute( "texture", GetStringFromMap( texture ).GetPointer() );
  if ( bits.trailScaleSaved )
    n->SetAttributeFromFloat( "trailScale", trailScale );
  if ( bits.toggleCategorySaved )
    n->SetAttribute( "toggleCategory", GetStringFromMap( toggleCategory ).GetPointer() );
  if ( bits.achievementIdSaved )
    n->SetAttributeFromInteger( "achievementId", achievementId );
  if ( bits.achievementBitSaved )
    n->SetAttributeFromInteger( "achievementBit", achievementBit );
  if ( bits.miniMapVisibleSaved )
    n->SetAttributeFromInteger( "miniMapVisibility", bits.miniMapVisible );
  if ( bits.bigMapVisibleSaved )
    n->SetAttributeFromInteger( "mapVisibility", bits.bigMapVisible );
  if ( bits.inGameVisibleSaved )
    n->SetAttributeFromInteger( "inGameVisibility", bits.inGameVisible );
  if ( bits.scaleWithZoomSaved )
    n->SetAttributeFromInteger( "scaleOnMapWithZoom", bits.scaleWithZoom );
  if ( bits.miniMapFadeOutLevelSaved )
    n->SetAttributeFromFloat( "mapFadeoutScaleLevel", miniMapFadeOutLevel );
  if ( bits.miniMapSizeSaved )
    n->SetAttributeFromInteger( "mapDisplaySize", miniMapSize );
  if ( bits.keepOnMapEdgeSaved )
    n->SetAttributeFromInteger( "keepOnMapEdge", bits.keepOnMapEdge );
  if (bits.infoSaved)
    n->SetAttribute("info", GetStringFromMap(info).GetPointer());
  if (bits.infoRangeSaved)
    n->SetAttributeFromFloat("infoRange", infoRange);
}

void AddTypeContextMenu( CWBContextItem *ctx, CArray<GW2TacticalCategory*> &CategoryList, GW2TacticalCategory *Parent, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL closeOnClick )
{
  for ( TS32 x = 0; x < CategoryMap.NumItems(); x++ )
  {
    auto dta = CategoryMap.GetByIndex( x );
    if ( dta->Parent == Parent )
    {
      CString txt;
      if ( AddVisibilityMarkers )
        txt += "[" + CString( dta->IsDisplayed ? "x" : " " ) + "] ";
      if ( dta->displayName.Length() )
        txt += dta->displayName;
      else
        txt += dta->name;

      if ( dta->IsOnlySeparator )
      {
        ctx->AddSeparator();
        if ( dta->displayName.Length() )
          txt = dta->displayName;
        else
          txt = dta->name;
        ctx->AddItem( txt.GetPointer(), CategoryList.NumItems() + BaseID, false, closeOnClick );
        CategoryList += dta;
        ctx->AddSeparator();
      }
      else
      {
        auto n = ctx->AddItem( txt.GetPointer(), CategoryList.NumItems() + BaseID, AddVisibilityMarkers && dta->IsDisplayed, closeOnClick );
        CategoryList += dta;
        AddTypeContextMenu( n, CategoryList, dta, AddVisibilityMarkers, BaseID, closeOnClick );
      }
    }
  }
}

void AddTypeContextMenu( CWBContextMenu *ctx, CArray<GW2TacticalCategory*> &CategoryList, GW2TacticalCategory *Parent, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL closeOnClick )
{
  for ( TS32 x = 0; x < CategoryMap.NumItems(); x++ )
  {
    auto dta = CategoryMap.GetByIndex( x );
    if ( dta->Parent == Parent )
    {
      CString txt;
      if ( AddVisibilityMarkers )
        txt += "[" + CString( dta->IsDisplayed ? "x" : " " ) + "] ";
      if ( dta->displayName.Length() )
        txt += dta->displayName;
      else
        txt += dta->name;

      if ( dta->IsOnlySeparator )
      {
        ctx->AddSeparator();
        if ( dta->displayName.Length() )
          txt = dta->displayName;
        else
          txt = dta->name;
        ctx->AddItem( txt.GetPointer(), CategoryList.NumItems() + BaseID, false, closeOnClick );
        CategoryList += dta;
        ctx->AddSeparator();
      }
      else
      {
        auto n = ctx->AddItem( txt.GetPointer(), CategoryList.NumItems() + BaseID, AddVisibilityMarkers && dta->IsDisplayed, closeOnClick );
        CategoryList += dta;
        AddTypeContextMenu( n, CategoryList, dta, AddVisibilityMarkers, BaseID, closeOnClick );
      }
    }
  }
}

void OpenTypeContextMenu( CWBContextItem *ctx, CArray<GW2TacticalCategory*> &CategoryList, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL closeOnClick )
{
  CategoryList.Flush();
  AddTypeContextMenu( ctx, CategoryList, &CategoryRoot, AddVisibilityMarkers, BaseID, closeOnClick );
}

void OpenTypeContextMenu( CWBContextMenu *ctx, CArray<GW2TacticalCategory*> &CategoryList, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL closeOnClick )
{
  CategoryList.Flush();
  AddTypeContextMenu( ctx, CategoryList, &CategoryRoot, AddVisibilityMarkers, BaseID, closeOnClick );
}

float WorldToGameCoords( float world )
{
  return world / 0.0254f;
}

float GameToWorldCoords( float game )
{
  return game * 0.0254f;
}

CString GW2TacticalCategory::GetFullTypeName()
{
  if ( cachedTypeName.Length() )
    return cachedTypeName;

  if ( !Parent ) return "";
  if ( Parent == &CategoryRoot )
  {
    CString n = name;
    n.ToLower();
    return n;
  }
  CString pname = Parent->GetFullTypeName();
  CString s = pname + "." + name;
  s.ToLower();

  cachedTypeName = s;
  return s;
}

TBOOL GW2TacticalCategory::IsVisible()
{
  if ( !Parent ) return true;
  return Parent->IsVisible() && IsDisplayed;
}

void POI::SetCategory( CWBApplication *App, GW2TacticalCategory *t )
{
  category = t;
  typeData = t->data;
  Type = t->GetFullTypeName();
  icon = 0;
  iconFile = typeData.iconFile;
  //icon = GetMapIcon( App, typeData.iconFile, zipFile );
  //iconSize = App->GetAtlas()->GetSize( icon );
}

bool POI::IsVisible( const tm& ptm, const time_t& currtime, bool achievementsFetched, CDictionary<TS32, Achievement> &achievements, LIGHTWEIGHT_CRITICALSECTION &dataWriteCritSec )
{
  if ( category && !category->IsVisible() )
    return false;

  if ( typeData.behavior == POIBehavior::ReappearOnDailyReset )
  {
    struct tm lasttime;
    gmtime_s( &lasttime, &lastUpdateTime );
    if ( lasttime.tm_mday == ptm.tm_mday && lasttime.tm_mon == ptm.tm_mon && lasttime.tm_year == ptm.tm_year )
      return false;
  }

  if ( typeData.behavior == POIBehavior::ReappearAfterTimer )
  {
    time_t elapsedtime = currtime - lastUpdateTime;
    if ( elapsedtime < typeData.resetLength )
    {
      if ( !typeData.bits.hasCountdown )
        return false;
    }
  }

  if ( typeData.behavior == POIBehavior::OnlyVisibleBeforeActivation )
  {
    if ( lastUpdateTime != time_t( 0 ) )
      return false;
  }

  if ( typeData.behavior == POIBehavior::OncePerInstance )
  {
    if ( ActivationData.HasKey( POIActivationDataKey( guid, mumbleLink.mapInstance ) ) )
      return false;
  }

  if ( typeData.behavior == POIBehavior::OncePerInstancePerChar )
  {
    if ( ActivationData.HasKey( POIActivationDataKey( guid, mumbleLink.mapInstance^mumbleLink.charIDHash ) ) )
      return false;
  }

  if ( typeData.behavior == POIBehavior::DailyPerChar )
  {
    if ( ActivationData.HasKey( POIActivationDataKey( guid, mumbleLink.charIDHash ) ) )
    {
      struct tm lasttime;
      gmtime_s( &lasttime, &ActivationData[ POIActivationDataKey( guid, mumbleLink.charIDHash ) ].lastUpdateTime );

      if ( lasttime.tm_mday == ptm.tm_mday && lasttime.tm_mon == ptm.tm_mon && lasttime.tm_year == ptm.tm_year )
        return false;
    }
  }

  if ( routeMember && ( ( position - mumbleLink.charPosition ).Length() <= typeData.triggerRange ) )
  {
    for ( TS32 y = 0; y < Routes.NumItems(); y++ )
    {
      if ( Routes[ y ].activeItem < 0 )
        return false;
    }
  }

  if ( achievementsFetched && typeData.achievementId != -1 )
  {
    CLightweightCriticalSection cs( &dataWriteCritSec );
    if ( achievements.HasKey( typeData.achievementId ) )
    {
      if (achievements[typeData.achievementId].done)
        return true;

      if ( ( typeData.achievementBit == -1 ) )
        return false;

      if ( achievements[ typeData.achievementId ].bits.Find( typeData.achievementBit ) >= 0 )
        return false;
    }
  }

  return true;
}
