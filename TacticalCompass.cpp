#include "TacticalCompass.h"
#include "MumbleLink.h"
#include "OverlayConfig.h"
#include "Language.h"

float GetMapFade();

void GW2TacticalCompass::DrawTacticalCompass( CWBDrawAPI* API )
{
  CRect drawrect = GetClientRect();

  CMatrix4x4 cam;
  cam.SetLookAtLH( mumbleLink.camPosition, mumbleLink.camPosition + mumbleLink.camDir, CVector3( 0, 1, 0 ) );
  CMatrix4x4 persp;
  persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (TF32)drawrect.Height(), 0.01f, 1000.0f );

  TS32 resolution = 60;

  CVector4 charpos = CVector4( mumbleLink.averagedCharPosition.x, mumbleLink.averagedCharPosition.y, mumbleLink.averagedCharPosition.z, 1.0f );;
  float rworld = GameToWorldCoords( 40 );

  CVector4 camSpaceChar = charpos;
  CVector4 camSpaceEye = charpos + CVector4( 0, 3, 0, 0 );

  CVector4 screenSpaceChar = ( camSpaceChar * cam ) * persp;
  screenSpaceChar /= screenSpaceChar.w;
  CVector4 screenSpaceEye = ( camSpaceEye * cam ) * persp;
  screenSpaceEye /= screenSpaceEye.w;

  //camSpaceChar = camSpaceChar*cam;
  //camSpaceEye = camSpaceEye*cam;
  //camSpaceChar /= camSpaceChar.w;
  //camSpaceEye /= camSpaceEye.w;

  auto pos = ( CVector3( mumbleLink.averagedCharPosition ) - mumbleLink.camPosition );
  pos.y = 0;
  bool zoomedin = pos.Length() < 0.13;

  CVector4 campos = CVector4( mumbleLink.camPosition.x, mumbleLink.camPosition.y, mumbleLink.camPosition.z, 1.0f );
  CVector2 camDir = CVector2( camSpaceChar.x - campos.x, camSpaceChar.z - campos.z ).Normalized();

  CVector3 toChar = CVector3( charpos - campos );
  float dist = toChar.Length();

  CWBFont* f = GetFont( GetState() );

  CString txt[ 4 ] = { DICT( "compassnorth" ),DICT( "compasseast" ),DICT( "compasssouth" ),DICT( "compasswest" ) };

  for ( int x = 0; x < 4; x++ )
  {
    float a1 = 1.0f;
    float f1 = x / (TF32)4 * PI * 2;
    CVector4 p1 = CVector4( rworld * sinf( f1 ), 1.0f, rworld * cosf( f1 ), 0.0f );

    CVector3 toPoint = CVector3( p1 - campos );

    if ( !zoomedin )
      a1 = 1 - powf( max( 0, camDir * CVector2( p1.x, p1.z ).Normalized() ), 10.0f );

    p1 = p1 + charpos;
    p1 = p1 * cam;
    p1 /= p1.w;

    if ( p1.z < 0.01 )
      continue;

    p1.z = max( 0.01f, p1.z );
    p1 = p1 * persp;
    p1 /= p1.w;

    if ( a1 < 1 )
      a1 = 1 - ( 1 - a1 ) * ( 1 - powf( ( p1.y - screenSpaceChar.y ) / ( screenSpaceEye.y - screenSpaceChar.y ), 10.0f ) );

    p1 = p1 * 0.5 + CVector4( 0.5, 0.5, 0.5, 0.0 );

    CPoint pa = CPoint( (int)( p1.x * drawrect.Width() ), (int)( ( 1 - p1.y ) * drawrect.Height() ) );

    a1 = max( 0, min( 1, a1 ) ) * 255;

    CRect cent = CRect( pa, pa );
    CPoint p = f->GetCenter( txt[ x ], cent );
    f->Write( API, txt[ x ], p, CColor( 228, 210, 157, (TU8)( a1 * GetMapFade() ) ) );
  }
}

void GW2TacticalCompass::OnDraw( CWBDrawAPI* API )
{
  if ( !mumbleLink.IsValid() )
    return;

  if ( Config::GetValue( "TacticalCompassVisible" ) )
    DrawTacticalCompass( API );
}

GW2TacticalCompass::GW2TacticalCompass( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{

}
GW2TacticalCompass::~GW2TacticalCompass()
{

}

CWBItem* GW2TacticalCompass::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new GW2TacticalCompass( Root, Pos );
}

TBOOL GW2TacticalCompass::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}
