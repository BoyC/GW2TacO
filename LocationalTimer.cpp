#include "LocationalTimer.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"

CArray<LocationalTimer> LocationalTimers;

void ImportLocationalTimers()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "locationaltimers.xml" ) ) return;

  if ( !d.GetDocumentNode().GetChildCount( "timers" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "timers" );

  for ( TS32 x = 0; x < root.GetChildCount( "areatriggeredtimer" ); x++ )
  {
    LocationalTimer t;
    t.ImportData( root.GetChild( "areatriggeredtimer", x ) );
    LocationalTimers += t;
  }
}

LocationalTimer::LocationalTimer()
{

}

LocationalTimer::~LocationalTimer()
{

}

void LocationalTimer::Update()
{
  if ( mumbleLink.mapID != MapID )
  {
    IsRunning = false;
    return;
  }

  if ( !IsRunning )
  {
    if ( EnterSphere.Contains( mumbleLink.charPosition ) )
    {
      IsRunning = true;
      StartTime = globalTimer.GetTime();
    }
  }

  if ( IsRunning )
  {
    if ( ( globalTimer.GetTime() - StartTime ) / 1000.0f > TimerLength )
      IsRunning = false;
    if ( !ExitSphere.Contains( mumbleLink.charPosition ) )
      IsRunning = false;
    if ( ( ResetPoint - mumbleLink.charPosition ).Length() < 0.1 )
      IsRunning = false;
  }
}

void LocationalTimer::ImportData( CXMLNode& node )
{
  if ( node.HasAttribute( "mapid" ) ) node.GetAttributeAsInteger( "mapid", &MapID );
  if ( node.HasAttribute( "length" ) ) node.GetAttributeAsInteger( "length", &TimerLength );
  if ( node.HasAttribute( "startdelay" ) ) node.GetAttributeAsInteger( "startdelay", &StartDelay );

  if ( node.HasAttribute( "enterspherex" ) ) node.GetAttributeAsFloat( "enterspherex", &EnterSphere.Position.x );
  if ( node.HasAttribute( "enterspherey" ) ) node.GetAttributeAsFloat( "enterspherey", &EnterSphere.Position.y );
  if ( node.HasAttribute( "enterspherez" ) ) node.GetAttributeAsFloat( "enterspherez", &EnterSphere.Position.z );
  if ( node.HasAttribute( "entersphererad" ) ) node.GetAttributeAsFloat( "entersphererad", &EnterSphere.Radius );

  if ( node.HasAttribute( "exitspherex" ) ) node.GetAttributeAsFloat( "exitspherex", &ExitSphere.Position.x );
  if ( node.HasAttribute( "exitspherey" ) ) node.GetAttributeAsFloat( "exitspherey", &ExitSphere.Position.y );
  if ( node.HasAttribute( "exitspherez" ) ) node.GetAttributeAsFloat( "exitspherez", &ExitSphere.Position.z );
  if ( node.HasAttribute( "exitsphererad" ) ) node.GetAttributeAsFloat( "exitsphererad", &ExitSphere.Radius );

  if ( node.HasAttribute( "resetpointx" ) ) node.GetAttributeAsFloat( "resetpointx", &ResetPoint.x );
  if ( node.HasAttribute( "resetpointy" ) ) node.GetAttributeAsFloat( "resetpointy", &ResetPoint.y );
  if ( node.HasAttribute( "resetpointz" ) ) node.GetAttributeAsFloat( "resetpointz", &ResetPoint.z );

  for ( TS32 x = 0; x < node.GetChildCount( "timeevent" ); x++ )
  {
    CXMLNode te = node.GetChild( "timeevent", x );
    TimerEvent tmr;
    if ( te.HasAttribute( "text" ) ) tmr.Text = te.GetAttributeAsString( "text" );
    if ( te.HasAttribute( "timestamp" ) ) te.GetAttributeAsInteger( "timestamp", &tmr.Time );
    if ( te.HasAttribute( "countdown" ) ) te.GetAttributeAsInteger( "countdown", &tmr.CountdownLength );
    if ( te.HasAttribute( "onscreentime" ) ) te.GetAttributeAsInteger( "onscreentime", &tmr.OnScreenLength );
    Events += tmr;
  }
}

void TimerDisplay::OnDraw( CWBDrawAPI* API )
{
  if ( !Config::GetValue( "LocationalTimersVisible" ) )
    return;

  TS32 tme = globalTimer.GetTime();
  CWBFont* f = GetFont( GetState() );

  TS32 ypos = Lerp( GetClientRect().y1, GetClientRect().y2, 0.25f );

  for ( TS32 x = 0; x < LocationalTimers.NumItems(); x++ )
  {
    LocationalTimer& t = LocationalTimers[ x ];

    t.Update();
    if ( !t.IsRunning )
      continue;

    TF32 timepos = ( tme - LocationalTimers[ x ].StartTime ) / 1000.0f - t.StartDelay;

    for ( TS32 y = 0; y < t.Events.NumItems(); y++ )
    {
      auto& e = t.Events[ y ];
      if ( !( timepos > e.Time - e.CountdownLength && timepos < e.Time + e.OnScreenLength ) )
        continue;

      CString s = e.Text;
      if ( timepos<e.Time && timepos>e.Time - e.CountdownLength )
        s += CString::Format( " in %d", (TS32)( e.Time - timepos ) );

      CPoint pos = f->GetTextPosition( s, CRect( GetClientRect().x1, ypos, GetClientRect().x2, ypos ), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
      ypos += f->GetLineHeight();
      f->Write( API, s, pos );
    }

  }
}

TBOOL TimerDisplay::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

TimerDisplay::TimerDisplay( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{

}

TimerDisplay::~TimerDisplay()
{

}

CWBItem* TimerDisplay::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new TimerDisplay( Root, Pos );
}
