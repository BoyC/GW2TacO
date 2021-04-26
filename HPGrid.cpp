#include "HPGrid.h"
#include "OverlayConfig.h"
#include "MumbleLink.h"

void GW2HPGrid::OnDraw( CWBDrawAPI *API )
{
  if ( !HasConfigValue( "HPGridVisible" ) )
    SetConfigValue( "HPGridVisible", 1 );

  if ( !GetConfigValue( "HPGridVisible" ) )
    return;

  CRect cl = GetClientRect();

  for ( int x = 0; x < Grids.NumItems(); x++ )
  {
    if ( mumbleLink.mapID != Grids[ x ].mapID )
      continue;

    if ( !Grids[ x ].bSphere.Contains( mumbleLink.charPosition ) )
      continue;

    for ( int y = 0; y < Grids[ x ].displayedPercentages.NumItems(); y++ )
    {
      int pos = (int)( cl.Width()*Grids[ x ].displayedPercentages[ y ].percentage / 100.0f );
      CRect r = CRect( pos, cl.y1, pos + 1, cl.y2 );
      API->DrawRect( r, Grids[ x ].displayedPercentages[ y ].color );
    }
  }
}

GW2HPGrid::GW2HPGrid( CWBItem *Parent, CRect Position ) : CWBItem( Parent, Position )
{
  LoadGrids();
}

GW2HPGrid::~GW2HPGrid()
{

}

CWBItem * GW2HPGrid::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new GW2HPGrid( Root, Pos );
}

TBOOL GW2HPGrid::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

void GW2HPGrid::LoadGrids()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "hpgrids.xml" ) ) return;

  if ( !d.GetDocumentNode().GetChildCount( "hpgrids" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "hpgrids" );

  for ( TS32 x = 0; x < root.GetChildCount( "grid" ); x++ )
  {
    CXMLNode node = root.GetChild( "grid", x );
    GridData d;

    if ( node.HasAttribute( "mapid" ) )
      node.GetAttributeAsInteger( "mapid", &d.mapID );

    if ( node.HasAttribute( "centerx" ) )
      node.GetAttributeAsFloat( "centerx", &d.bSphere.Position.x );
    if ( node.HasAttribute( "centery" ) )
      node.GetAttributeAsFloat( "centery", &d.bSphere.Position.y );
    if ( node.HasAttribute( "centerz" ) )
      node.GetAttributeAsFloat( "centerz", &d.bSphere.Position.z );
    if ( node.HasAttribute( "radius" ) )
      node.GetAttributeAsFloat( "radius", &d.bSphere.Radius );

    for ( TS32 y = 0; y < node.GetChildCount( "percentage" ); y++ )
    {
      CXMLNode perc = node.GetChild( "percentage", y );
      GridLine line;
      if ( perc.HasAttribute( "value" ) )
        perc.GetAttributeAsFloat( "value", &line.percentage );
      if ( perc.HasAttribute( "color" ) )
      {
        CString colhex = perc.GetAttributeAsString( "color" );
        unsigned int val = 0;
        colhex.Scan( "%x", &val );
        line.color = CColor( val );
      }
      d.displayedPercentages += line;
    }

    Grids += d;
  }
}
