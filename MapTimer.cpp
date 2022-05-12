#include "MapTimer.h"
#include "GW2API.h"
#include "OverlayConfig.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"

using namespace jsonxx;

void TriggerWaypointCopyPaste( CWBApplication* App, CString copy )
{
  if ( !Config::GetValue( "CanWriteToClipboard" ) )
    return;

  // copy to clipboard here

  if ( OpenClipboard( (HWND)App->GetHandle() ) )
  {
    EmptyClipboard();

    CString out = copy;

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

void GW2MapTimer::OnDraw( CWBDrawAPI* API )
{
  if ( !Config::GetValue( "MapTimerVisible" ) )
    return;

  CString mouseToolTip;

  if ( GW2::apiKeyManager.GetStatus() == GW2::APIKeyManager::Status::OK )
  {
    GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

    if ( key && key->valid && ( globalTimer.GetTime() - lastFetchTime > 150000 || !lastFetchTime ) && !beingFetched && !fetchThread.joinable() )
    {
      beingFetched = true;
      fetchThread = std::thread( [this, key]()
                                 {
                                   SetPerThreadCRTExceptionBehavior();
                                   Object json;

                                   CString lastDungeonStatus = CString( "{\"worldbosses\":" ) + key->QueryAPI( "v2/account/worldbosses" ) + "}";
                                   json.parse( lastDungeonStatus.GetPointer() );

                                   if ( json.has<Array>( "worldbosses" ) )
                                   {
                                     auto dungeonData = json.get<Array>( "worldbosses" ).values();

                                     CLightweightCriticalSection cs( &critSec );

                                     worldBosses.Flush();

                                     for ( unsigned int x = 0; x < dungeonData.size(); x++ )
                                     {
                                       if ( !dungeonData[ x ]->is<String>() )
                                         continue;

                                       CString eventName = CString( dungeonData[ x ]->get<String>().data() );
                                       worldBosses += eventName;
                                     }
                                   }

                                   lastDungeonStatus = CString( "{\"mapchests\":" ) + key->QueryAPI( "v2/account/mapchests" ) + "}";
                                   json.parse( lastDungeonStatus.GetPointer() );

                                   if ( json.has<Array>( "mapchests" ) )
                                   {
                                     auto dungeonData = json.get<Array>( "mapchests" ).values();

                                     CLightweightCriticalSection cs( &critSec );

                                     mapchests.Flush();

                                     for ( unsigned int x = 0; x < dungeonData.size(); x++ )
                                     {
                                       if ( !dungeonData[ x ]->is<String>() )
                                         continue;

                                       CString eventName = CString( dungeonData[ x ]->get<String>().data() );
                                       mapchests += eventName;
                                     }
                                   }

                                   beingFetched = false;
                                 } );
    }
  }

  if ( !beingFetched && fetchThread.joinable() )
  {
    lastFetchTime = globalTimer.GetTime();
    fetchThread.join();
  }

  TBOOL compact = Config::GetValue( "MapTimerCompact" );

  TS32 timeWindow = 120;
  TS32 mapheight = 40;
  TS32 barheight = 20;

  CRect cl = GetClientRect();

  //SYSTEMTIME systime;
  //GetSystemTime(&systime);

  time_t rawtime;
  time( &rawtime );
  struct tm ptm;
  gmtime_s( &ptm, &rawtime );

  WBITEMSTATE i = GetState();
  CWBFont* f = GetFont( i );

  barheight = f->GetLineHeight();
  mapheight = barheight + f->GetLineHeight();

  if ( compact )
    mapheight = barheight;

  TS32 mapCount = 0;
  for ( TS32 x = 0; x < maps.NumItems(); x++ )
    if ( maps[ x ].display )
      mapCount++;

  DrawBackgroundItem( API, CSSProperties.DisplayDescriptor, CRect( cl.TopLeft(), CPoint( cl.Width(), mapCount * mapheight + 1 ) ), GetState() );
  //API->DrawRect(CRect(cl.TopLeft(), CPoint(cl.Width(), maps.NumItems() * mapheight + 1)), 0x20ffffff);

  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  //TS32 minutes = systime.wHour * 60 + systime.wMinute;
  TS32 minutes = ptm.tm_hour * 60 + ptm.tm_min;
  TS32 lefttime = minutes - timeWindow / 2;

  TS32 ypos = 0;

  CArray< CRect > highlightRects;

  for ( int x = 0; x < maps.NumItems(); x++ )
  {
    if ( !maps[ x ].display )
      continue;

    TS32 currtime = -48 * 60 + maps[ x ].Length + maps[ x ].Start - lefttime;
    TS32 currevent = 0;

    if ( ( ypos > cl.y2 ) || ( ypos < cl.y1 - mapheight ) )
      continue;

    CPoint p = f->GetCenter( maps[ x ].name, CRect( cl.x1, ypos, cl.x2, ypos + mapheight - barheight + 1 ), TextTransform );
    //p.y -= 3;
    if ( !compact )
      f->Write( API, maps[ x ].name, CPoint( p.x, ypos + 2 ), 0xffffffff, TextTransform );

    TS32 toppos = ypos + mapheight - barheight;
    TS32 bottompos = ypos + mapheight + 1;

    {
      CLightweightCriticalSection cs( &critSec );
      if ( maps[ x ].chestId.Length() > 0 && mapchests.Find( maps[ x ].chestId ) >= 0 )
        highlightRects += CRect( cl.x1, toppos, cl.x2, bottompos );
    }

    while ( currtime < 72 * 60 )
    {
      TS32 p1 = (TS32)( cl.Width() * currtime / (TF32)timeWindow );
      TS32 p2 = (TS32)( cl.Width() * ( currtime + maps[ x ].events[ currevent ].length ) / (TF32)timeWindow ) + 1;

      if ( p2 >= 0 && p1 <= cl.Width() )
      {
        CRect r = CRect( max( 0, p1 ), toppos, min( cl.Width(), p2 ), bottompos );

        API->DrawRect( r, maps[ x ].events[ currevent ].color );

        CRect cr = API->GetCropRect();
        API->SetCropRect( ClientToScreen( r ) );

        CString text = maps[ x ].events[ currevent ].name;
        
        CString CopyWPText = "";
        TBOOL StartTimeNegativ = FALSE;
        TS32 timestart = currtime * 60 - ptm.tm_sec - timeWindow * 30;
        CString StartTime = CString::Format( "%d:%.2d", timestart / 60, timestart % 60 );

        if ( timestart >= 0 )
        {
          StartTimeNegativ = FALSE;
        }
        else
        {
          StartTimeNegativ = TRUE;
        }

        //if ( minutes >= currtime && minutes < currtime + maps[x].events[currevent].length)
        {
          TS32 timeleft = currtime * 60 - ptm.tm_sec - timeWindow * 30 + maps[ x ].events[ currevent ].length * 60;// (currtime + maps[x].events[currevent].length) * 60 - (minutes * 60 + systime.wSecond);
          if ( timeleft >= 0 && timeleft <= maps[ x ].events[ currevent ].length * 60 )
            text = CString::Format( "%s %d:%.2d", text.GetPointer(), timeleft / 60, timeleft % 60 );
        }

        if ( ClientToScreen( r ).Contains( GetApplication()->GetMousePos() ) )
        {
          mouseToolTip = text;
          if ( GetApplication()->GetLeftButtonState()  )
          {
            if ( StartTimeNegativ )
            {
              CopyWPText = ( maps[ x ].events[ currevent ].name + " WP: " + maps[ x ].events[ currevent ].waypoint );
            }
            else
            {
              CopyWPText = ( maps[ x ].events[ currevent ].name + " in " + StartTime + " WP: " + maps[ x ].events[ currevent ].waypoint );
            }
            
            TriggerWaypointCopyPaste( GetApplication(), CopyWPText );
          }
        }

        CPoint p = f->GetCenter( text, r, TextTransform );
        //p.y -= 3;
        f->Write( API, text, CPoint( p.x, r.y1 + 2 ), 0xffffffff, TextTransform );

        API->SetCropRect( cr );

        bool isHighlighted = false;

        {
          CLightweightCriticalSection cs( &critSec );
          if ( maps[ x ].events[ currevent ].worldBossId.Length() > 0 && worldBosses.Find( maps[ x ].events[ currevent ].worldBossId ) >= 0 )
            isHighlighted = true;
          if ( maps[ x ].events[ currevent ].worldBossId.Length() > 0 && mapchests.Find( maps[ x ].events[ currevent ].worldBossId ) >= 0 )
            isHighlighted = true;
        }

        if ( !isHighlighted )
          API->DrawRectBorder( r, 0x80000000 );
        else
          highlightRects.Add( r );
      }

      currtime += maps[ x ].events[ currevent ].length;
      currevent = ( currevent + 1 ) % maps[ x ].events.NumItems();
    }

    ypos += mapheight;
  }

  for ( int x = 0; x < highlightRects.NumItems(); x++ )
    API->DrawRectBorder( highlightRects[ x ], 0xffffcc00 );

  API->DrawRect( CRect( cl.Width() / 2, 0, cl.Width() / 2 + 1, ypos ), 0x80ffffff );
  SetMouseToolTip( mouseToolTip );
}

void GW2MapTimer::SetLayout( CXMLNode& node )
{
  for ( int x = 0; x < node.GetChildCount( "Map" ); x++ )
  {
    CXMLNode& mapNode = node.GetChild( "Map", x );

    Map map;
    if ( mapNode.HasAttribute( "Name" ) )
      map.name = mapNode.GetAttributeAsString( "Name" );

    if ( mapNode.HasAttribute( "ChestAPIID" ) )
      map.chestId = mapNode.GetAttributeAsString( "ChestAPIID" );

    if ( mapNode.HasAttribute( "id" ) )
    {
      map.id = mapNode.GetAttributeAsString( "id" );
      CString str = CString( "maptimer_mapopen_" ) + map.id;
      if ( Config::HasValue( str.GetPointer() ) )
        map.display = Config::GetValue( str.GetPointer() );
    }

    if ( mapNode.HasAttribute( "Length" ) )
      mapNode.GetAttributeAsInteger( "Length", &map.Length );

    if ( mapNode.HasAttribute( "Start" ) )
      mapNode.GetAttributeAsInteger( "Start", &map.Start );

    int start = 0;

    for ( int y = 0; y < mapNode.GetChildCount( "Event" ); y++ )
    {
      CXMLNode& eventNode = mapNode.GetChild( "Event", y );
      Event event;
      event.length = 0;
      event.start = start;

      if ( eventNode.HasAttribute( "Name" ) )
        event.name = eventNode.GetAttributeAsString( "Name" );

      if ( eventNode.HasAttribute( "WorldBossAPIID" ) )
        event.worldBossId = eventNode.GetAttributeAsString( "WorldBossAPIID" );

      if ( eventNode.HasAttribute( "WayPoint" ) )
        event.waypoint = eventNode.GetAttributeAsString( "WayPoint" );

      if ( eventNode.HasAttribute( "Length" ) )
      {
        eventNode.GetAttributeAsInteger( "Length", &event.length );
        start += event.length;
      }

      if ( eventNode.HasAttribute( "Color" ) )
      {
        CString s = eventNode.GetAttributeAsString( "Color" );
        int c = 0;
        s.Scan( "%x", &c );
        event.color = CColor( c );
      }

      map.events += event;
    }

    maps += map;
  }
}

GW2MapTimer::GW2MapTimer( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "maptimer.xml" ) ) return;

  if ( !d.GetDocumentNode().GetChildCount( "GW2MapTimer" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "GW2MapTimer" );
  SetLayout( root );

  SetID( "MapTimer" );
}

GW2MapTimer::~GW2MapTimer()
{
  if ( fetchThread.joinable() )
    fetchThread.join();
}

CWBItem* GW2MapTimer::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  auto tmr = new GW2MapTimer( Root, Pos );
  return tmr;
}

TBOOL GW2MapTimer::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}
