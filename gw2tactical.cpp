#include "gw2tactical.h"
#include "MumbleLink.h"
#include "Bedrock/UtilLib/PNGDecompressor.h"
#include "OverlayConfig.h"
#include <mmsystem.h>
#include "TrailLogger.h"
#include "WvW.h"
#include "Language.h"
#include "GW2API.h"
#include "OverlayApplication.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"
#include <unordered_map>
#include "MarkerPack.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "MarkerEditor.h"
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
  if ( idx < 0 || idx >= stringArray.NumItems() )
    return emptyString;
  return stringArray[ idx ];
}

void UpdateWvWStatus();

void FindClosestRouteMarkers( TBOOL force )
{
  auto& POIs = GetMapPOIs();

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
        POI& p = POIs[ g ];
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

void TriggerCopyPasteWithMessage( CWBApplication* App, int copy, int copyMessage )
{
  if ( !Config::GetValue( "CanWriteToClipboard" ) )
    return;

  GW2TacticalDisplay* tactical = (GW2TacticalDisplay*)App->GetRoot()->FindChildByID( _T( "tactical" ), _T( "gw2tactical" ) );
  if ( tactical )
    tactical->TriggerBigMessage( copyMessage );

  // copy to clipboard here

  if ( OpenClipboard( (HWND)App->GetHandle() ) )
  {
    EmptyClipboard();

    CString out = GetStringFromMap( copy );

    HGLOBAL clipbuffer = GlobalAlloc( GMEM_DDESHARE | GHND, ( out.Length() + 1 ) * sizeof( TCHAR ) );

    if ( clipbuffer )
    {
      TCHAR* buffer = (TCHAR*)GlobalLock( clipbuffer );
      if ( buffer )
      {
        memcpy( buffer, out.GetPointer(), sizeof( TCHAR ) * out.Length() );
        buffer[ out.Length() ] = 0;
      }

      GlobalUnlock( clipbuffer );

#ifndef  UNICODE
      SetClipboardData( CF_TEXT, clipbuffer );
#else
      SetClipboardData( CF_UNICODETEXT, clipbuffer );
#endif
    }

    CloseClipboard();
  }
}

GW2TacticalCategory* GetCategory( CString s )
{
  s.ToLower();
  if ( CategoryMap.HasKey( s ) )
    return CategoryMap[ s ];
  return nullptr;
}

WBATLASHANDLE GetMapIcon( CWBApplication* App, CString& filename, const CString& zipFile, const CString& categoryZip )
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

    for ( int x = 0; x < 2; x++ )
    {
      if ( !zipFile.Length() && x == 0 )
        continue;

      if ( !categoryZip.Length() && x == 1 )
        continue;

      mz_zip_archive* zip = x == 0 ? OpenZipFile( zipFile ) : OpenZipFile( categoryZip );

      if ( zip )
      {
        int idx = mz_zip_reader_locate_file( zip, filename.GetPointer(), nullptr, 0 );
        if ( idx >= 0 && !mz_zip_reader_is_file_a_directory( zip, idx ) )
        {
          mz_zip_archive_file_stat stat;
          if ( mz_zip_reader_file_stat( zip, idx, &stat ) && stat.m_uncomp_size > 0 )
          {
            TU8* data = new TU8[ (TS32)stat.m_uncomp_size ];

            if ( mz_zip_reader_extract_to_mem( zip, idx, data, (TS32)stat.m_uncomp_size, 0 ) )
            {
              TU8* imageData = nullptr;
              TS32 xres, yres;
              if ( DecompressPNG( data, (TS32)stat.m_uncomp_size, imageData, xres, yres ) )
              {
                ARGBtoABGR( imageData, xres, yres );

                auto handle = App->GetAtlas()->AddImage( imageData, xres, yres, CRect( 0, 0, xres, yres ) );
                //ExportPNG( imageData, xres, yres, false, CString::Format( "debugexport\\%d.png", cntr++ ) );

                SAFEDELETEA( imageData );
                MapIcons[ s ] = handle;

                delete[] data;
                return handle;
              }
              else
                LOG_ERR( "[GWTacO] Failed to decompress png %s form archive %s", filename.GetPointer(), x == 0 ? zipFile.GetPointer() : categoryZip.GetPointer() );
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
  if ( !f.Open( s.GetPointer() ) && !f.Open( ( CString( "POIs\\" ) + s ).GetPointer() ) )
  {
    LOG_ERR( "[GWTacO] Failed to open image %s", s.GetPointer() );
    return DefaultIconHandle;
  }

  TU8* imageData = nullptr;
  TS32 xres, yres;
  if ( !DecompressPNG( f.GetData(), (TS32)f.GetLength(), imageData, xres, yres ) )
  {
    LOG_ERR( "[GWTacO] Failed to decompress png %s", s.GetPointer() );
    return DefaultIconHandle;
  }

  ARGBtoABGR( imageData, xres, yres );

  auto handle = App->GetAtlas()->AddImage( imageData, xres, yres, CRect( 0, 0, xres, yres ) );

  SAFEDELETEA( imageData );
  MapIcons[ s ] = handle;
  //LOG_DBG( "Created icon with handle %d from %s", handle, s.GetPointer() );
  return handle;
}

std::unordered_map<int, CDictionaryEnumerable<GUID, POI>> POISet;
CDictionaryEnumerable<GUID, POI>& GetMapPOIs()
{
  return POISet[ mumbleLink.mapID ];
}

CDictionaryEnumerable<GUID, POI>& GetPOIs( int mapId )
{
  return POISet[ mapId ];
}

CDictionaryEnumerable<POIActivationDataKey, POIActivationData> activationData;
CArray<POIRoute> Routes;

TU32 DictionaryHash( const GUID& i )
{
  TU8* dta = (TU8*)( &i );
  TU32 Hash = 5381;
  for ( int x = 0; x < sizeof( GUID ); x++ )
    Hash = ( ( Hash << 5 ) + Hash ) + dta[ x ]; // hash * 33 + c
  return Hash;
}

TU32 DictionaryHash( const POIActivationDataKey& i )
{
  TU8* dta = (TU8*)( &i );
  TU32 Hash = 5381;
  for ( int x = 0; x < sizeof( POIActivationDataKey ); x++ )
    Hash = ( ( Hash << 5 ) + Hash ) + dta[ x ]; // hash * 33 + c
  return Hash;
}

float distPointPlane( CVector3 vPoint, CPlane plane )
{
  return vPoint * plane.Normal + plane.D;
}

float distRayPlane( CVector3 vRayOrigin, CVector3 vnRayVector, CVector3 vnPlaneNormal, float planeD )
{
  float cosAlpha;
  float deltaD;

  cosAlpha = vnRayVector * vnPlaneNormal;
  // parallel to the plane (alpha=90)
  if ( cosAlpha == 0 ) return -1.0f;
  deltaD = planeD - vRayOrigin * vnPlaneNormal;

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

void SetRotate( CMatrix4x4& m, float x, float y, float z, float phi )
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

  float xfov = atan( asp * tan( yfov ) );

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
      res[ x ] = o + vn * di[ x ];
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

  return p.Normalized() * length;
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

void GW2TacticalDisplay::InsertPOI( POI& poi )
{
/*
  if ( poi.mapID != mumbleLink.mapID )
    return;
*/

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

  poi.cameraSpacePosition = CVector4( poi.position.x, poi.position.y + poi.typeData.height, poi.position.z, 1.0f ) * cam;

  minimapPOIs.Add( &poi );

  if ( poi.typeData.fadeFar >= 0 && poi.typeData.fadeNear >= 0 )
  {
    TF32 dist = WorldToGameCoords( poi.cameraSpacePosition.Length() );
    if ( dist > poi.typeData.fadeFar )
      return;
  }

  mapPOIs.Add( &poi );
}

void GW2TacticalDisplay::DrawPOI( CWBDrawAPI* API, const tm& ptm, const time_t& currtime, POI& poi, bool drawDistance, CString& infoText )
{
  TBOOL drawCountdown = false;
  TS32 timeLeft;
  float alphaMultiplier = 1;

  if ( poi.external && hideExternal )
    return;

  if ( !poi.IsVisible( ptm, currtime ) )
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

    TF32 dist = ( poi.position - mumbleLink.charPosition ).Length();

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
        data = mumbleLink.charIDHash ^ mumbleLink.mapInstance;

      activationData[ POIActivationDataKey( poi.guid, data ) ] = d;
    }
  }

  if ( poi.typeData.copy != -1 && poi.typeData.bits.autoTrigger && triggeredPOI != &poi )
  {
    TF32 dist = ( poi.position - mumbleLink.charPosition ).Length();

    if ( dist <= poi.typeData.triggerRange )
      triggeredPOI = &poi;
  }

  if ( poi.typeData.info >= 0 )
  {
    if ( ( poi.position - mumbleLink.charPosition ).Length() <= poi.typeData.infoRange )
    {
      infoText += GetStringFromMap( poi.typeData.info ) + "\n";
    }
  }


  if ( poi.routeMember && ( ( poi.position - mumbleLink.charPosition ).Length() <= poi.typeData.triggerRange ) )
  {
    for ( TS32 y = 0; y < Routes.NumItems(); y++ )
    {
      if ( Routes[ y ].activeItem < 0 )
        continue;

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
    poi.icon = GetMapIcon( App, GetStringFromMap( poi.iconFile ), GetStringFromMap( poi.zipFile ), poi.category ? GetStringFromMap( poi.category->zipFile ) : "" );
    //LOG_DBG( "[GW2TacO] Icon is %s", GetStringFromMap( poi.iconFile ).GetPointer() );
  }

  WBATLASHANDLE icon = poi.icon;
  TF32 size = poi.typeData.size;
  TF32 Alpha = poi.typeData.alpha;

  auto camspace = poi.cameraSpacePosition;
  auto screenpos = camspace;

  CVector4 camspacex = camspace + CVector4( 0.5f, 0, 0, 0 ) * size;

  if ( tacticalIconsOnEdge )
  {
    screenpos /= screenpos.w;
    CVector3 projpos = ProjectTacticalPos( screenpos, mumbleLink.fov, asp );
    screenpos.x = projpos.x;
    screenpos.y = projpos.y;
    screenpos.z = projpos.z;
    //screenpos.Normalize();
    //screenpos *= camspace.Length();
    camspace = screenpos;
    camspacex = camspace + CVector4( 0.5f, 0, 0, 0 ) * size;
  }

  if ( !tacticalIconsOnEdge && camspace.z <= 0 ) return;

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

  camspace = camspace * persp;
  camspace /= camspace.w;

  screenpos = screenpos * persp;
  screenpos /= screenpos.w;

  camspacex = camspacex * persp;
  camspacex /= camspacex.w;

  //CSize iconsize = poi.iconSize;

  int s = (int)min( poi.typeData.maxSize, max( poi.typeData.minSize, abs( ( camspacex - camspace ).x ) * drawrect.Width() ) );

  //if (TacticalIconsOnEdge)
  //	s = (int)min(max(iconsize.x, iconsize.y) / 2.0f, s);

  if ( poi.typeData.behavior == POIBehavior::WvWObjective )
  {
    alphaMultiplier = max( 0, min( 1, powf( CVector2( screenpos ).Length(), 2 ) + 0.3f ) );
  }

  screenpos = screenpos * 0.5 + CVector4( 0.5, 0.5, 0.5, 0.0 );

  CPoint p = CPoint( (int)( screenpos.x * drawrect.Width() ), (int)( ( 1 - screenpos.y ) * drawrect.Height() ) );

  CRect rect = CRect( p - CPoint( s, s ), p + CPoint( s, s ) );

  if ( storeMarkerPositions )
    markerPositions[ poi.guid ] = rect;

  if ( tacticalIconsOnEdge )
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
      col.A() = TU8( col.A() * mapFade * globalOpacity );
    API->DrawAtlasElement( icon, rect, false, false, true, true, col );
  }

  if ( drawWvWNames && poi.typeData.behavior == POIBehavior::WvWObjective )
  {
    CWBFont* f = App->GetDefaultFont();
    extern CArray<WvWObjective> wvwObjectives;
    CString wvwObjectiveName;

    if ( poi.wvwObjectiveID < wvwObjectives.NumItems() )
      wvwObjectiveName = DICT( wvwObjectives[ poi.wvwObjectiveID ].nameToken, wvwObjectives[ poi.wvwObjectiveID ].name );

    if ( wvwObjectiveName.Length() )
    {
      p = f->GetTextPosition( wvwObjectiveName.GetPointer(), rect, WBTA_CENTERX, WBTA_TOP, WBTT_UPPERCASE, false ) - CPoint( 0, f->GetLineHeight() );
      f->Write( API, wvwObjectiveName, p, CColor( 255, 255, 0, TU8( 255 * alphaMultiplier * mapFade * globalOpacity ) ), WBTT_UPPERCASE, false );
    }
  }

  if ( drawCountdown )
  {
    CWBFont* f = GetFont( GetState() );
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
          col.A() = TU8( col.A() * Alpha * alphaMultiplier * mapFade * globalOpacity );
        else
          col.A() = TU8( col.A() * mapFade * globalOpacity );
        API->DrawAtlasElement( forbiddenIconHandle, rect, false, false, true, true, col );
      }

      p = f->GetTextPosition( txt.GetPointer(), rect, WBTA_CENTERX, WBTA_BOTTOM, WBTT_NONE, false ) + CPoint( 0, f->GetLineHeight() + offset );
      f->Write( API, txt, p, CColor( 255, 255, 0, TU8( 255 * alphaMultiplier * mapFade * globalOpacity ) ), WBTT_NONE, false );
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
    CWBFont* f = App->GetDefaultFont();
    if ( !f )
      return;

    if ( Alpha * alphaMultiplier > 0 )
    {
      TF32 charDist = WorldToGameCoords( ( poi.position - mumbleLink.charPosition ).Length() );

      char text[ 32 ];

      if ( !useMetricDisplay )
      {
        sprintf_s( text, 32, "%d\0", (TS32)charDist );
      }
      else
      {
        charDist *= 0.0254f;
        sprintf_s( text, 32, "%.1fm\0", charDist );
      }

      p = f->GetTextPosition( text, rect, WBTA_CENTERX, WBTA_BOTTOM, WBTT_NONE, false ) + CPoint( 0, f->GetLineHeight() );
      f->Write( API, text, p, CColor( 255, 255, 255, TU8( 255 * Alpha * alphaMultiplier * mapFade * globalOpacity ) ), WBTT_NONE, false );
    }
  }
}

TF32 uiScale = 1.0f;

void GW2TacticalDisplay::DrawPOIMinimap( CWBDrawAPI* API, const CRect& miniRect, CVector2& pos, const tm& ptm, const time_t& currtime, POI& poi, float alpha, float zoomLevel )
{
  if ( alpha <= 0 )
    return;
  if ( poi.external && hideExternal )
    return;
  if ( !poi.IsVisible( ptm, currtime ) )
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
    poi.icon = GetMapIcon( App, GetStringFromMap( poi.iconFile ), GetStringFromMap( poi.zipFile ), poi.category ? GetStringFromMap( poi.category->zipFile ) : "" );
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
  col.A() = TU8( col.A() * alpha * minimapOpacity * poi.typeData.alpha );

  if ( storeMarkerPositions )
    markerMinimapPositions[ poi.guid ] = displayRect;

  API->DrawAtlasElement( poi.icon, displayRect, false, false, true, true, col );
}

void GW2TacticalDisplay::OnDraw( CWBDrawAPI* API )
{
  int opac = Config::GetValue( "OpacityIngame" );
  if ( opac == 0 )
    globalOpacity = 1.0f;
  if ( opac == 1 )
    globalOpacity = 2 / 3.0f;
  if ( opac == 2 )
    globalOpacity = 1 / 3.0f;

  opac = Config::GetValue( "OpacityMap" );
  if ( opac == 0 )
    minimapOpacity = 1.0f;
  if ( opac == 1 )
    minimapOpacity = 2 / 3.0f;
  if ( opac == 2 )
    minimapOpacity = 1 / 3.0f;

  useMetricDisplay = Config::GetValue( "UseMetricDisplay" );

  if ( !Config::GetValue( "TacticalLayerVisible" ) )
    return;

  if ( !mumbleLink.IsValid() )
    return;

  storeMarkerPositions = Config::IsWindowOpen( "MarkerEditor" );
  if ( storeMarkerPositions )
  {
    markerPositions.Flush();
    markerMinimapPositions.Flush();
  }

  uiScale = GetUIScale();

  int showMinimapMarkers = Config::GetValue( "ShowMinimapMarkers" );
  int showBigmapMarkers = Config::GetValue( "ShowBigmapMarkers" );
  int showIngameMarkers = Config::GetValue( "ShowInGameMarkers" );

  Achievements::FetchAchievements();
  UpdateWvWStatus();

  tacticalIconsOnEdge = Config::GetValue( "TacticalIconsOnEdge" );
  drawWvWNames = Config::GetValue( "DrawWvWNames" ) != 0;
  bool drawDistance = Config::GetValue( "TacticalDrawDistance" ) != 0;

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

  int mumbleMapID = mumbleLink.mapID;

  auto& POIs = GetMapPOIs();

  for ( int x = 0; x < POIs.NumItems(); x++ )
  {
    auto& poi = POIs.GetByIndex( x );
    if ( poi.mapID != mumbleMapID )
      continue;
    InsertPOI( poi );
  }

  extern bool wvwCanBeRendered;

  if ( wvwCanBeRendered )
    for ( int x = 0; x < wvwPOIs.NumItems(); x++ )
    {
      auto& poi = wvwPOIs.GetByIndex( x );
      if ( poi.mapID != mumbleMapID )
        continue;
      InsertPOI( poi );
    }

  time_t currtime;
  time( &currtime );

  mapPOIs.Sort( []( POI** a, POI** b ) -> int { return ( *b )->cameraSpacePosition.z > ( *a )->cameraSpacePosition.z; } );

  for ( TS32 x = 0; x < Routes.NumItems(); x++ )
  {
    if ( Routes[ x ].hasResetPos && Routes[ x ].MapID == mumbleLink.mapID && ( Routes[ x ].resetPos - mumbleLink.charPosition ).Length() < Routes[ x ].resetRad )
      Routes[ x ].activeItem = 0;
  }

  CString infoText;

  POI* prevtriggeredPOI = triggeredPOI;
  triggeredPOI = nullptr;
  TS32 prevcopyText = bigMessage;

  hideExternal = Config::GetValue( "HideExternalMarkers" );

  if ( showIngameMarkers > 0 )
    for ( int x = 0; x < mapPOIs.NumItems(); x++ )
    {
      if ( !mapPOIs[ x ]->typeData.bits.inGameVisible && showIngameMarkers != 2 )
        continue;

      DrawPOI( API, ptm, currtime, *mapPOIs[ x ], drawDistance, infoText );
    }

  if ( prevtriggeredPOI == triggeredPOI )
  {
    triggeredPOI = prevtriggeredPOI;
  }
  else
  {
    if ( triggeredPOI && triggeredPOI->typeData.copy != -1 )
    {
      TriggerCopyPasteWithMessage( App, triggeredPOI->typeData.copy, triggeredPOI->typeData.copyMessage );
    }
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
      if ( !minimapPOIs[ x ]->IsVisible( ptm, currtime ) )
        continue;

      CVector3 poiPos = minimapPOIs[ x ]->position * miniMapTrafo;
      DrawPOIMinimap( API, miniRect, CVector2( poiPos.x, poiPos.y ), ptm, currtime, *minimapPOIs[ x ], mapFade, mumbleLink.miniMap.mapScale );
    }
  }

  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( editor && !editor->IsHidden() )
      editor->DrawUberTool( API, GetClientRect() );
  }

  if ( mumbleLink.isMapOpen && mapFade < 1.0 && showBigmapMarkers > 0 )
  {
    miniRect = GetClientRect();
    CMatrix4x4 miniMapTrafo = mumbleLink.bigMap.BuildTransformationMatrix( miniRect, true );
    for ( int x = 0; x < minimapPOIs.NumItems(); x++ )
    {
      if ( !minimapPOIs[ x ]->typeData.bits.bigMapVisible && showBigmapMarkers != 2 )
        continue;
      if ( !minimapPOIs[ x ]->IsVisible( ptm, currtime ) )
        continue;

      CVector3 poiPos = minimapPOIs[ x ]->position * miniMapTrafo;
      DrawPOIMinimap( API, miniRect, CVector2( poiPos.x, poiPos.y ), ptm, currtime, *minimapPOIs[ x ], 1.0f - mapFade, mumbleLink.bigMap.mapScale );
    }
  }

  if ( Config::GetValue( "TacticalInfoTextVisible" ) )
  {
    auto font = GetApplication()->GetRoot()->GetFont( GetState() );
    TS32 width = font->GetWidth( infoText );
    font->Write( API, infoText, CPoint( int( ( GetClientRect().Width() - width ) / 2.0f ), int( GetClientRect().Height() * 0.15f ) ) );
  }

  if ( bigMessage != -1 )
  {
    auto time = globalTimer.GetTime() - bigMessageStart;
    if ( time >= 0 && time < bigMessageLength && bigMessage != -1 )
    {
      auto& string = GetStringFromMap( bigMessage );
      auto timer = App->GetRoot()->FindChildByID( "locationaltimer" );
      if ( timer )
      {
        auto f = timer->GetFont( GetState() );
        float alpha = max( 0, ( 1000 - max( 0, time - bigMessageLength + 1000 ) ) / 1000.0f );
        TS32 ypos = Lerp( GetClientRect().y1, GetClientRect().y2, 0.25f );

        CPoint pos = f->GetTextPosition( string, CRect( GetClientRect().x1, ypos, GetClientRect().x2, ypos ), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
        ypos += f->GetLineHeight();
        f->Write( API, string, pos, CColor( 255, 255, 255, (unsigned char)( 255 * alpha ) ) );
      }
    }
    if ( time > bigMessageLength )
      bigMessage = -1;
  }
}

GW2TacticalDisplay::GW2TacticalDisplay( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{

}

GW2TacticalDisplay::~GW2TacticalDisplay()
{
  Achievements::WaitForFetch();
}

CWBItem* GW2TacticalDisplay::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new GW2TacticalDisplay( Root, Pos );
}

TBOOL GW2TacticalDisplay::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

void GW2TacticalDisplay::RemoveUserMarkersFromMap()
{
  if ( !mumbleLink.IsValid() )
    return;
  auto& POIs = GetMapPOIs();

  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    if ( POIs.GetByIndex( x ).mapID == mumbleLink.mapID && !POIs.GetByIndex( x ).external )
    {
      POIs.DeleteByIndex( x );
      x--;
    }

  }

  ExportPOIS();
}

void GW2TacticalDisplay::TriggerBigMessage( TS32 messageString )
{
  bigMessageStart = globalTimer.GetTime();
  bigMessage = messageString;
}

bool CreateNewPOI( CWBApplication* App, POI& poi, int defaultCategory, bool testRange )
{
  if ( !mumbleLink.IsValid() )
    return false;
  poi = POI();
  poi.position = mumbleLink.charPosition;
  poi.mapID = mumbleLink.mapID;
  poi.icon = DefaultIconHandle;
  //poi.iconSize = DefaultIconSize;

  CoCreateGuid( &poi.guid );

  auto cat = GetCategory( Config::GetString( "defaultcategory0" ) );
  switch ( defaultCategory )
  {
  case 1:
    cat = GetCategory( Config::GetString( "defaultcategory1" ) );
    break;
  case 2:
    cat = GetCategory( Config::GetString( "defaultcategory2" ) );
    break;
  case 3:
    cat = GetCategory( Config::GetString( "defaultcategory3" ) );
    break;
  case 4:
    cat = GetCategory( Config::GetString( "defaultcategory4" ) );
    break;
  }

  if ( poi.mapID == -1 )
    return false;

  auto& POIs = GetMapPOIs();

  if ( testRange )
  {
    for ( TS32 x = 0; x < POIs.NumItems(); x++ )
    {
      if ( POIs.GetByIndex( x ).mapID != poi.mapID ) continue;
      CVector3 v = POIs.GetByIndex( x ).position - poi.position;
      if ( v.Length() < POIs.GetByIndex( x ).typeData.triggerRange && cat == POIs.GetByIndex( x ).category )
        return false;
    }
  }

  if ( cat )
    poi.SetCategory( App, cat );

/*
  POIs[ poi.guid ] = poi;
  ExportPOIS();

  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( editor && !editor->IsHidden() )
      editor->SetEditedGUID( poi.guid );
  }
*/

  return true;// &POIs[ poi.guid ];
}

bool CreateNewTrail( CWBApplication* App, const CString& fileName, GW2Trail*& poi )
{
  if ( !mumbleLink.IsValid() ) 
    return false;
  poi = new GW2Trail();

  CoCreateGuid( &poi->guid );
  auto cat = GetCategory( Config::GetString( "defaultcategory4" ) );
  auto& POIs = GetMapTrails();

  if ( cat )
    poi->SetCategory( App, cat );

  poi->typeData.trailData = AddStringToMap( fileName );
  poi->typeData.saveBits.trailDataSaved = true;
  poi->typeData.fadeNear = 5000;
  poi->typeData.fadeFar = 5100;
  poi->Reload();

/*
  POIs[ poi->guid ] = poi;

*/

/*
  ExportPOIS();

  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( editor && !editor->IsHidden() )
      editor->SetEditedGUID( poi->guid );
  }
*/

  return true;
}

void DeletePOI()
{
  if ( !mumbleLink.IsValid() ) return;
  POI poi;
  poi.position = CVector3( mumbleLink.charPosition );
  poi.mapID = mumbleLink.mapID;

  if ( poi.mapID == -1 ) return;

  auto& POIs = GetMapPOIs();

  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    if ( POIs.GetByIndex( x ).mapID != poi.mapID ) continue;
    CVector3 v = POIs.GetByIndex( x ).position - poi.position;
    if ( v.Length() < POIs.GetByIndex( x ).typeData.triggerRange )
    {
      GUID guid = POIs.GetByIndex( x ).guid;

      POIs.DeleteByIndex( x );
      ExportPOIS();

      if ( Config::IsWindowOpen( "MarkerEditor" ) )
      {
        extern CWBApplication* App;
        auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
        if ( editor && !editor->IsHidden() && editor->GetEditedGUID() == guid )
        {
          editor->SetEditedGUID( GUID{} );
        }
      }
      return;
    }
  }
}

void DeletePOI( const GUID& guid )
{
  auto& POIs = GetMapPOIs();
  if ( POIs.HasKey( guid ) )
  {
    MarkerDOM::DeleteMarkerTrail( guid );
/*
    POIs.Delete( guid );
    ExportPOIS();
*/
  }

  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    extern CWBApplication* App;
    auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( editor && !editor->IsHidden() )
      editor->SetEditedGUID( GUID{} );
  }
}

void DeleteTrail( const GUID& guid )
{
  auto& POIs = GetMapTrails();
  if ( POIs.HasKey( guid ) )
  {
    MarkerDOM::DeleteMarkerTrail( guid );
/*
    POIs.Delete( guid );
    ExportPOIS();
*/
  }

  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    extern CWBApplication* App;
    auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( editor && !editor->IsHidden() )
      editor->SetEditedGUID( GUID{} );
  }
}

void UpdatePOI( CWBApplication* App )
{
  if ( !mumbleLink.IsValid() )
    return;

  if ( mumbleLink.mapID == -1 )
    return;


  time_t rawtime;
  time( &rawtime );
  struct tm ptm;
  gmtime_s( &ptm, &rawtime );

  time_t currtime;
  time( &currtime );

  //TBOOL found = false;

  auto& POIs = GetMapPOIs();

  CArray<GW2TacticalCategory*> categoriesToToggle;

  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    auto& cpoi = POIs.GetByIndex( x );

    if ( cpoi.mapID != mumbleLink.mapID )
      continue;

    CVector3 dist = cpoi.position - CVector3( mumbleLink.charPosition );
    if ( dist.Length() < cpoi.typeData.triggerRange )
    {
      auto& str = GetStringFromMap( cpoi.typeData.toggleCategory );
      if ( str.Length() )
      {
        GW2TacticalCategory* cat = GetCategory( str );
        if ( cat )
          categoriesToToggle.AddUnique( cat );
      }

      if ( /*!found &&*/ cpoi.typeData.behavior != POIBehavior::AlwaysVisible && cpoi.IsVisible( ptm, currtime ) )
      {
        POIActivationData activation;
        time( &activation.lastUpdateTime );
        cpoi.lastUpdateTime = activation.lastUpdateTime;
        activation.poiguid = cpoi.guid;

        activation.uniqueData = 0;
        if ( cpoi.typeData.behavior == POIBehavior::OncePerInstance )
          activation.uniqueData = mumbleLink.mapInstance;
        if ( cpoi.typeData.behavior == POIBehavior::DailyPerChar )
          activation.uniqueData = mumbleLink.charIDHash;
        if ( cpoi.typeData.behavior == POIBehavior::OncePerInstancePerChar )
          activation.uniqueData = mumbleLink.charIDHash ^ mumbleLink.mapInstance;

        activationData[ POIActivationDataKey( cpoi.guid, activation.uniqueData ) ] = activation;
        ExportPOIActivationData();
        //found = true;
      }

      if ( cpoi.typeData.copy != -1 && !cpoi.typeData.bits.autoTrigger && ( !cpoi.category || cpoi.category->IsVisible() ) )
        TriggerCopyPasteWithMessage( App, cpoi.typeData.copy, cpoi.typeData.copyMessage );
    }
  }

  for ( int x = 0; x < categoriesToToggle.NumItems(); x++ )
  {
    auto* cat = categoriesToToggle[ x ];

    cat->isDisplayed = !cat->isDisplayed;
    CategoryRoot.CalculateVisibilityCache();
    Config::SetValue( ( CString( "CategoryVisible_" ) + cat->GetFullTypeName() ).GetPointer(), cat->isDisplayed );
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

void MarkerTypeData::Read( CXMLNode& n, TBOOL StoreSaveState )
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
  TBOOL _infoSaved = n.HasAttribute( "info" );
  TBOOL _infoRangeSaved = n.HasAttribute( "infoRange" );
  TBOOL _copySaved = n.HasAttribute( "copy" );
  TBOOL _copyMessageSaved = n.HasAttribute( "copy-message" );
  TBOOL _defaultToggleSaved = n.HasAttribute( "defaulttoggle" );
  TBOOL _festivalsSaved = n.HasAttribute( "festival" );

  if ( _festivalsSaved )
  {
    CString festivalList = n.GetAttribute( "festival" );
    festivalList.ToLower();
    CStringArray festivalArray = festivalList.Explode( "," );
    int start = 0;
    int cnt = 0;
    for ( auto& festival : GW2::festivals )
    {
      if ( festivalArray.Find( CString( festival.name ) ) >= 0 )
        festivalMask |= 1 << cnt;
      cnt++;
    }
  }

  if ( StoreSaveState )
  {
    saveBits.iconFileSaved = _iconFileSaved;
    saveBits.sizeSaved = _sizeSaved;
    saveBits.alphaSaved = _alphaSaved;
    saveBits.fadeNearSaved = _fadeNearSaved;
    saveBits.fadeFarSaved = _fadeFarSaved;
    saveBits.heightSaved = _heightSaved;
    saveBits.behaviorSaved = _behaviorSaved;
    saveBits.resetLengthSaved = _resetLengthSaved;
    saveBits.autoTriggerSaved = _autoTriggerSaved;
    saveBits.hasCountdownSaved = _hasCountdownSaved;
    saveBits.triggerRangeSaved = _triggerRangeSaved;
    saveBits.minSizeSaved = _minSizeSaved;
    saveBits.maxSizeSaved = _maxSizeSaved;
    saveBits.colorSaved = _colorSaved;
    saveBits.trailDataSaved = _trailDataSaved;
    saveBits.animSpeedSaved = _animSpeedSaved;
    saveBits.textureSaved = _textureSaved;
    saveBits.trailScaleSaved = _trailScaleSaved;
    saveBits.toggleCategorySaved = _toggleCategorySaved;
    saveBits.achievementIdSaved = _achievementIdSaved;
    saveBits.achievementBitSaved = _achievementBitSaved;
    saveBits.miniMapVisibleSaved = _miniMapVisibleSaved;
    saveBits.bigMapVisibleSaved = _bigMapVisibleSaved;
    saveBits.inGameVisibleSaved = _inGameVisibleSaved;
    saveBits.scaleWithZoomSaved = _scaleWithZoomSaved;
    saveBits.miniMapSizeSaved = _miniMapSizeSaved;
    saveBits.miniMapFadeOutLevelSaved = _miniMapFadeOutLevelSaved;
    saveBits.keepOnMapEdgeSaved = _keepOnMapEdgeSaved;
    saveBits.infoSaved = _infoSaved;
    saveBits.infoRangeSaved = _infoRangeSaved;
    saveBits.copySaved = _copySaved;
    saveBits.copyMessageSaved = _copyMessageSaved;
    saveBits.defaultToggleSaved = _defaultToggleSaved;
    festivalSaveMask = festivalMask;
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
  if ( _defaultToggleSaved )
  {
    TS32 val;
    n.GetAttributeAsInteger( "defaulttoggle", &val );
    bits.defaultToggle = val != 0;
    bits.defaultToggleLoaded = true;
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
  if ( _infoSaved )
    info = AddStringToMap( n.GetAttributeAsString( "info" ) );
  if ( _infoRangeSaved )
    n.GetAttributeAsFloat( "infoRange", &infoRange );

  if ( _copySaved )
    copy = AddStringToMap( n.GetAttributeAsString( "copy" ) );

  if ( _copyMessageSaved )
    copyMessage = AddStringToMap( n.GetAttributeAsString( "copy-message" ) );
}

void MarkerTypeData::Write( CXMLNode* n )
{
  if ( saveBits.iconFileSaved )
    n->SetAttribute( "iconFile", GetStringFromMap( iconFile ).GetPointer() );
  if ( saveBits.sizeSaved )
    n->SetAttributeFromFloat( "iconSize", size );
  if ( saveBits.alphaSaved )
    n->SetAttributeFromFloat( "alpha", alpha );
  if ( saveBits.fadeNearSaved )
    n->SetAttributeFromFloat( "fadeNear", fadeNear );
  if ( saveBits.fadeFarSaved )
    n->SetAttributeFromFloat( "fadeFar", fadeFar );
  if ( saveBits.heightSaved )
    n->SetAttributeFromFloat( "heightOffset", height );
  if ( saveBits.behaviorSaved )
    n->SetAttributeFromInteger( "behavior", (TS32)behavior );
  if ( saveBits.resetLengthSaved )
    n->SetAttributeFromInteger( "resetLength", resetLength );
  //if ( saveBits.resetOffsetSaved )
  //  n->SetAttributeFromInteger( "resetOffset", resetOffset );
  if ( saveBits.autoTriggerSaved )
    n->SetAttributeFromInteger( "autoTrigger", bits.autoTrigger );
  if ( saveBits.defaultToggleSaved )
    n->SetAttributeFromInteger( "defaulttoggle", bits.defaultToggle );
  if ( saveBits.hasCountdownSaved )
    n->SetAttributeFromInteger( "hasCountdown", bits.hasCountdown );
  if ( saveBits.triggerRangeSaved )
    n->SetAttributeFromFloat( "triggerRange", triggerRange );
  if ( saveBits.minSizeSaved )
    n->SetAttributeFromInteger( "minSize", minSize );
  if ( saveBits.maxSizeSaved )
    n->SetAttributeFromInteger( "maxSize", maxSize );
  if ( saveBits.colorSaved )
    n->SetAttribute( "color", CString::Format( "%x", color.argb() ).GetPointer() );
  if ( saveBits.trailDataSaved )
    n->SetAttribute( "trailData", GetStringFromMap( trailData ).GetPointer() );
  if ( saveBits.animSpeedSaved )
    n->SetAttributeFromFloat( "animSpeed", animSpeed );
  if ( saveBits.textureSaved )
    n->SetAttribute( "texture", GetStringFromMap( texture ).GetPointer() );
  if ( saveBits.trailScaleSaved )
    n->SetAttributeFromFloat( "trailScale", trailScale );
  if ( saveBits.toggleCategorySaved )
    n->SetAttribute( "toggleCategory", GetStringFromMap( toggleCategory ).GetPointer() );
  if ( saveBits.achievementIdSaved )
    n->SetAttributeFromInteger( "achievementId", achievementId );
  if ( saveBits.achievementBitSaved )
    n->SetAttributeFromInteger( "achievementBit", achievementBit );
  if ( saveBits.miniMapVisibleSaved )
    n->SetAttributeFromInteger( "miniMapVisibility", bits.miniMapVisible );
  if ( saveBits.bigMapVisibleSaved )
    n->SetAttributeFromInteger( "mapVisibility", bits.bigMapVisible );
  if ( saveBits.inGameVisibleSaved )
    n->SetAttributeFromInteger( "inGameVisibility", bits.inGameVisible );
  if ( saveBits.scaleWithZoomSaved )
    n->SetAttributeFromInteger( "scaleOnMapWithZoom", bits.scaleWithZoom );
  if ( saveBits.miniMapFadeOutLevelSaved )
    n->SetAttributeFromFloat( "mapFadeoutScaleLevel", miniMapFadeOutLevel );
  if ( saveBits.miniMapSizeSaved )
    n->SetAttributeFromInteger( "mapDisplaySize", miniMapSize );
  if ( saveBits.keepOnMapEdgeSaved )
    n->SetAttributeFromInteger( "keepOnMapEdge", bits.keepOnMapEdge );
  if ( saveBits.infoSaved )
    n->SetAttribute( "info", GetStringFromMap( info ).GetPointer() );
  if ( saveBits.infoRangeSaved )
    n->SetAttributeFromFloat( "infoRange", infoRange );
  if ( saveBits.copySaved )
    n->SetAttribute( "copy", GetStringFromMap( copy ).GetPointer() );
  if ( saveBits.copyMessageSaved )
    n->SetAttribute( "copy-message", GetStringFromMap( copyMessage ).GetPointer() );
  if ( festivalSaveMask )
  {
    CString output = "";

    int cnt = 0;
    for ( auto& festival : GW2::festivals )
    {
      int m = 1 << cnt;
      if ( festivalSaveMask & m )
      {
        if ( output.Length() )
          output += ",";
        output += festival.name;
      }
      cnt++;
    }

    if ( output.Length() )
      n->SetAttribute( "festival", output.GetPointer() );
  }
}

void MarkerTypeData::ClearSavedBits()
{
  memset( &saveBits, 0, sizeof( saveBits ) );
}

bool SingleAchievementTarget( GW2TacticalCategory* cat, int& resultId )
{
  int id = -1;
  for ( int x = 0; x < cat->children.NumItems(); id++ )
  {
    auto child = cat->children[ x ];
    if ( id == -1 )
      id = child->data.achievementId;

    if ( id == 0 || child->data.achievementId != id )
      return false;

    bool result = SingleAchievementTarget( child, resultId );
    if ( !result || resultId != id )
      return false;
  }

  resultId = id;
  return true;
}

void CalcCategoryHiddenFromContextMenu( GW2TacticalCategory* Parent, CDictionary<TS32, Achievement>& achievements )
{
  for ( TS32 x = 0; x < Parent->children.NumItems(); x++ )
  {
    auto dta = Parent->children[ x ];
    dta->hiddenFromContextMenu = dta->containedMapIds.find( mumbleLink.mapID ) == dta->containedMapIds.end();

    if ( dta->data.achievementId && achievements.HasKey( dta->data.achievementId ) )
    {
      auto& achi = achievements[ dta->data.achievementId ];

      dta->hiddenFromContextMenu |= achi.done;
      if ( !dta->hiddenFromContextMenu && dta->data.achievementBit )
        dta->hiddenFromContextMenu |= achi.bits.Find( dta->data.achievementBit ) >= 0;
    }

    CalcCategoryHiddenFromContextMenu( dta, achievements );
  }

  // restore needed headers
  for ( TS32 x = 0; x < Parent->children.NumItems(); x++ )
  {
    auto dta = Parent->children[ x ];
    if ( !dta->isOnlySeparator )
      continue;

    for ( int y = x + 1; y < Parent->children.NumItems(); y++ )
    {
      auto dta2 = Parent->children[ y ];
      if ( dta2->isOnlySeparator )
        break;

      if ( !dta2->hiddenFromContextMenu )
      {
        dta->hiddenFromContextMenu = false;
        break;
      }
    }
  }

  bool allChildrenHidden = true;
  for ( TS32 x = 0; x < Parent->children.NumItems(); x++ )
    if ( !Parent->children[ x ]->hiddenFromContextMenu )
    {
      allChildrenHidden = false;
      break;
    }

  if ( allChildrenHidden && !Parent->markerCount )
    Parent->hiddenFromContextMenu = true;
}

void SetAllCategoriesToVisibleInContext( GW2TacticalCategory* Parent )
{
  Parent->hiddenFromContextMenu = false;
  for ( int x = 0; x < Parent->children.NumItems(); x++ )
    SetAllCategoriesToVisibleInContext( Parent->children[ x ] );
}

GW2TacticalCategory* FindInCategoryTree( GW2TacticalCategory* ref, GW2TacticalCategory* cat )
{
  if ( ref == cat )
    return ref;

  for ( int x = 0; x < cat->children.NumItems(); x++ )
  {
    auto result = FindInCategoryTree( ref, cat->children[ x ] );
    if ( result )
      return result;
  }

  return nullptr;
}

GW2TacticalCategory* FindInCategoryTree( GW2TacticalCategory* cat )
{
  return FindInCategoryTree( cat, &CategoryRoot );
}

void AddTypeContextMenu( CWBContextItem* ctx, CArray<GW2TacticalCategory*>& CategoryList, GW2TacticalCategory* Parent, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL closeOnClick )
{
  int hiddenCount = 0;

  bool noHiding = Config::GetValue( "NoCategoryHiding" );

  for ( TS32 x = 0; x < Parent->children.NumItems(); x++ )
  {
    auto dta = Parent->children[ x ];
    if ( dta->hiddenFromContextMenu && !noHiding )
    {
      if ( !dta->isOnlySeparator )
        hiddenCount++;
      continue;
    }

    CString txt;
    if ( AddVisibilityMarkers )
      txt += "[" + CString( dta->isDisplayed ? "x" : " " ) + "] ";
    if ( dta->displayName.Length() )
      txt += dta->displayName;
    else
      txt += dta->name;

    if ( dta->isOnlySeparator )
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
      auto n = ctx->AddItem( txt.GetPointer(), CategoryList.NumItems() + BaseID, AddVisibilityMarkers && dta->isDisplayed, closeOnClick );
      CategoryList += dta;
      AddTypeContextMenu( n, CategoryList, dta, AddVisibilityMarkers, BaseID, closeOnClick );
    }
  }

  if ( hiddenCount )
    ctx->AddItem( CString::Format( ( hiddenCount == 1 ? DICT( "HiddenCategorySingular" ) : DICT( "HiddenCategoryPlural" ) ).GetPointer(), hiddenCount ), (int)Parent, false, closeOnClick );
}

void AddTypeContextMenu( CWBContextMenu* ctx, CArray<GW2TacticalCategory*>& CategoryList, GW2TacticalCategory* Parent, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL closeOnClick )
{
  int hiddenCount = 0;

  bool noHiding = Config::GetValue( "NoCategoryHiding" );

  for ( TS32 x = 0; x < Parent->children.NumItems(); x++ )
  {
    auto dta = Parent->children[ x ];
    if ( dta->hiddenFromContextMenu && !noHiding )
    {
      if ( !dta->isOnlySeparator )
        hiddenCount++;
      continue;
    }

    CString txt;
    if ( AddVisibilityMarkers )
      txt += "[" + CString( dta->isDisplayed ? "x" : " " ) + "] ";
    if ( dta->displayName.Length() )
      txt += dta->displayName;
    else
      txt += dta->name;

    if ( dta->isOnlySeparator )
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
      auto n = ctx->AddItem( txt.GetPointer(), CategoryList.NumItems() + BaseID, AddVisibilityMarkers && dta->isDisplayed, closeOnClick );
      CategoryList += dta;
      AddTypeContextMenu( n, CategoryList, dta, AddVisibilityMarkers, BaseID, closeOnClick );
    }
  }

  if ( hiddenCount )
    ctx->AddItem( CString::Format( ( hiddenCount == 1 ? DICT( "HiddenCategorySingular" ) : DICT( "HiddenCategoryPlural" ) ).GetPointer(), hiddenCount ), (int)Parent, false, closeOnClick );
}

void OpenTypeContextMenu( CWBContextItem* ctx, CArray<GW2TacticalCategory*>& CategoryList, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL markerEditor, CDictionary<TS32, Achievement>& achievements )
{
  CategoryList.Flush();
  if ( !markerEditor )
    CalcCategoryHiddenFromContextMenu( &CategoryRoot, achievements );
  else
    SetAllCategoriesToVisibleInContext( &CategoryRoot );
  AddTypeContextMenu( ctx, CategoryList, &CategoryRoot, AddVisibilityMarkers, BaseID, markerEditor );
}

void OpenTypeContextMenu( CWBContextMenu* ctx, CArray<GW2TacticalCategory*>& CategoryList, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL markerEditor, CDictionary<TS32, Achievement>& achievements )
{
  CategoryList.Flush();
  if ( !markerEditor )
    CalcCategoryHiddenFromContextMenu( &CategoryRoot, achievements );
  else
    SetAllCategoriesToVisibleInContext( &CategoryRoot );
  AddTypeContextMenu( ctx, CategoryList, &CategoryRoot, AddVisibilityMarkers, BaseID, markerEditor );
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

  if ( !parent ) return "";
  if ( parent == &CategoryRoot )
  {
    CString n = name;
    n.ToLower();
    return n;
  }
  CString pname = parent->GetFullTypeName();
  CString s = pname + "." + name;
  s.ToLower();

  cachedTypeName = s;
  return s;
}

TBOOL GW2TacticalCategory::IsVisible() const
{
  if ( visibilityCached )
    return cachedVisibility;

  if ( !parent )
    return isDisplayed;
  return isDisplayed && parent->IsVisible();
}

void GW2TacticalCategory::CacheVisibility()
{
  cachedVisibility = IsVisible();
  for ( int x = 0; x < children.NumItems(); x++ )
    children[ x ]->CacheVisibility();
}

bool GW2TacticalCategory::visibilityCached = false;

void GW2TacticalCategory::CalculateVisibilityCache()
{
  visibilityCached = false;
  CacheVisibility();
  visibilityCached = true;
}

void GW2TacticalCategory::SetDefaultToggleValues()
{
  if ( data.bits.defaultToggleLoaded )
  {
    CString catConfig = ( CString( "CategoryVisible_" ) + GetFullTypeName() );

    if ( !Config::HasValue( catConfig.GetPointer() ) )
      Config::SetValue( catConfig.GetPointer(), data.bits.defaultToggle );
  }

  for ( int x = 0; x < children.NumItems(); x++ )
    children[ x ]->SetDefaultToggleValues();
}

void GW2TacticalCategory::SetExportNeeded()
{
  needsExport = true;
/*
  if ( parent )
    parent->SetExportNeeded();
*/
}

void GW2TacticalCategory::ClearExportNeeded()
{
  needsExport = false;
  for ( int x = 0; x < children.NumItems(); x++ )
    children[ x ]->ClearExportNeeded();
}

void POI::SetCategory( CWBApplication* App, GW2TacticalCategory* t )
{
  category = t;
  typeData = t->data;
  typeData.ClearSavedBits();
  Type = AddStringToMap( t->GetFullTypeName() );
  icon = 0;
  iconFile = typeData.iconFile;
  //icon = GetMapIcon( App, typeData.iconFile, zipFile );
  //iconSize = App->GetAtlas()->GetSize( icon );
}

bool POI::IsVisible( const tm& ptm, const time_t& currtime )
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
    if ( activationData.HasKey( POIActivationDataKey( guid, mumbleLink.mapInstance ) ) )
      return false;
  }

  if ( typeData.behavior == POIBehavior::OncePerInstancePerChar )
  {
    if ( activationData.HasKey( POIActivationDataKey( guid, mumbleLink.mapInstance ^ mumbleLink.charIDHash ) ) )
      return false;
  }

  if ( typeData.behavior == POIBehavior::DailyPerChar )
  {
    if ( activationData.HasKey( POIActivationDataKey( guid, mumbleLink.charIDHash ) ) )
    {
      struct tm lasttime;
      gmtime_s( &lasttime, &activationData[ POIActivationDataKey( guid, mumbleLink.charIDHash ) ].lastUpdateTime );

      if ( lasttime.tm_mday == ptm.tm_mday && lasttime.tm_mon == ptm.tm_mon && lasttime.tm_year == ptm.tm_year )
        return false;
    }
  }

/*
  if ( routeMember && ( ( position - mumbleLink.charPosition ).Length() <= typeData.triggerRange ) )
  {
    for ( TS32 y = 0; y < Routes.NumItems(); y++ )
    {
      if ( Routes[ y ].activeItem < 0 )
        return false;
    }
  }
*/

  if ( Achievements::fetched && typeData.achievementId != -1 )
  {
    CLightweightCriticalSection cs( &Achievements::critSec );
    if ( Achievements::achievements.HasKey( typeData.achievementId ) )
    {

      if ( typeData.achievementBit == -1 )
        return !Achievements::achievements[ typeData.achievementId ].done;

      return !Achievements::achievements[ typeData.achievementId ].done && Achievements::achievements[ typeData.achievementId ].bits.Find( typeData.achievementBit ) < 0;
    }
  }

  if ( typeData.festivalMask )
  {
    bool hasActiveFestival = false;
    int cnt = 0;
    for ( auto& festival : GW2::festivals )
    {
      if ( typeData.festivalMask & ( 1 << cnt ) && festival.active )
        hasActiveFestival = true;
      cnt++;
    }

    return hasActiveFestival;
  }

  return true;
}