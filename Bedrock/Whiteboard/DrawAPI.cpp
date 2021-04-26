#include "BasePCH.h"
#include "DrawAPI.h"
#include "Application.h"

#define VERTEXBUFFERRECTCOUNT 4096
#define VERTEXBUFFERVERTEXCOUNT (VERTEXBUFFERRECTCOUNT*4)

COREVERTEXATTRIBUTE WBGuiVertexFormat[] =
{
  COREVXATTR_POSITIONT4,
  COREVXATTR_TEXCOORD2,
  COREVXATTR_COLOR4,

  COREVXATTR_STOP,
};

void CWBDrawAPI::AddDisplayRect( const CRect &Rect, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v2, const CColor color/*, const CColor c2, const CColor c3, const CColor c4*/ )
{
  if ( !Opacity ) return;

  const CRect rect = Rect + Offset;

  if ( !CropRect.Intersects( rect ) ) return;

  if ( DrawMode != WBD_RECTANGLES ) RenderDisplayList();
  DrawMode = WBD_RECTANGLES;

  TF32 u1f = u1;
  TF32 v1f = v1;
  TF32 u2f = u2;
  TF32 v2f = v2;

  CRect Pos = CropRect.GetIntersection( rect );
  //LOG(LOG_INFO,_T("%d %d %d %d"),r.Pos.x1,r.Pos.y1,r.Pos.x2,r.Pos.y2);

  if ( Pos != rect ) //need to cull UV
  {
    TF32 xs = (TF32)rect.Width();
    TF32 ys = (TF32)rect.Height();
    TF32 us = u2 - u1;
    TF32 vs = v2 - v1;

    TF32 x1f = ( Pos.x1 - rect.x1 ) / xs;
    TF32 y1f = ( Pos.y1 - rect.y1 ) / ys;
    TF32 x2f = ( Pos.x2 - rect.x1 ) / xs;
    TF32 y2f = ( Pos.y2 - rect.y1 ) / ys;

    u1f = us*x1f + u1;
    v1f = vs*y1f + v1;
    u2f = us*x2f + u1;
    v2f = vs*y2f + v1;
  }

  CColor Color = color;
  Color.A() = ( Color.A()*Opacity ) / 255;

  CVector2 TL, BR;
  TL.x = (TF32)Pos.x1;
  TL.y = (TF32)Pos.y1;
  BR.x = (TF32)Pos.x2;
  BR.y = (TF32)Pos.y2;

  DisplayList.AllocateNewUninitialized( 4 );
  TS32 idx = DisplayList.NumItems() - 4;

  DisplayList[ idx + 0 ].Initialize( TL.x, TL.y, u1f, v1f, Color );
  DisplayList[ idx + 1 ].Initialize( BR.x, TL.y, u2f, v1f, Color );
  DisplayList[ idx + 2 ].Initialize( BR.x, BR.y, u2f, v2f, Color );
  DisplayList[ idx + 3 ].Initialize( TL.x, BR.y, u1f, v2f, Color );
}

void CWBDrawAPI::AddDisplayLine( const CPoint &_p1, const CPoint &_p2, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v2, const CColor a, const CColor b )
{
  WBDISPLAYLINE r;

  CPoint p1 = _p1 + Offset;
  CPoint p2 = _p2 + Offset;

  CRect Area = CRect( p1, p2 );
  Area.Normalize();
  CRect rArea = CropRect | Area;
  if ( rArea.Width() < 0 || rArea.Height() < 0 ) return;
  if ( DrawMode != WBD_LINES ) RenderDisplayList();

  DrawMode = WBD_LINES;

  float t1 = 0;
  float t2 = 0;

  float xs = (float)abs( p2.x - p1.x );
  float ys = (float)abs( p2.y - p1.y );

  if ( p1.x < rArea.x1 ) t1 = max( t1, ( rArea.x1 - p1.x ) / xs );
  if ( p1.y < rArea.y1 ) t1 = max( t1, ( rArea.y1 - p1.y ) / ys );

  if ( p1.x > rArea.x2 ) t1 = max( t1, ( p1.x - rArea.x2 ) / xs );
  if ( p1.y > rArea.y2 ) t1 = max( t1, ( p1.y - rArea.y2 ) / ys );

  if ( p2.x < rArea.x1 ) t2 = max( t2, ( rArea.x1 - p2.x ) / xs );
  if ( p2.y < rArea.y1 ) t2 = max( t2, ( rArea.y1 - p2.y ) / ys );

  if ( p2.x > rArea.x2 ) t2 = max( t2, ( p2.x - rArea.x2 ) / xs );
  if ( p2.y > rArea.y2 ) t2 = max( t2, ( p2.y - rArea.y2 ) / ys );

  t2 = 1 - t2;

  if ( t1 >= t2 ) return;

  r.p1 = Lerp( p1, p2, t1 );
  r.p2 = Lerp( p1, p2, t2 );

  r.u1 = Lerp( u1, u2, t1 );
  r.u2 = Lerp( u1, u2, t2 );
  r.v1 = Lerp( v1, v2, t1 );
  r.v2 = Lerp( v1, v2, t2 );

  r.c1 = Lerp( a, b, t1 );
  r.c2 = Lerp( a, b, t2 );

  DisplayList += WBGUIVERTEX( CVector2( r.p1.x + 0.5f, r.p1.y - 0.5f ), CVector2( r.u1, r.v1 ), r.c1 );
  DisplayList += WBGUIVERTEX( CVector2( r.p2.x + 0.5f, r.p2.y - 0.5f ), CVector2( r.u2, r.v2 ), r.c2 );
}

void CWBDrawAPI::AddDisplayRectRotated( const CRect &Rect, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v2, const CColor color, TF32 rotation )
{
  if ( !Opacity ) return;

  const CRect rect = Rect + Offset;

  if ( !CropRect.Intersects( rect ) ) return;

  if ( DrawMode != WBD_RECTANGLES ) RenderDisplayList();
  DrawMode = WBD_RECTANGLES;

  TF32 u1f = u1;
  TF32 v1f = v1;
  TF32 u2f = u2;
  TF32 v2f = v2;

  CRect Pos = CropRect.GetIntersection( rect );
  //LOG(LOG_INFO,_T("%d %d %d %d"),r.Pos.x1,r.Pos.y1,r.Pos.x2,r.Pos.y2);

  if ( Pos != rect ) //need to cull UV
  {
    TF32 xs = (TF32)rect.Width();
    TF32 ys = (TF32)rect.Height();
    TF32 us = u2 - u1;
    TF32 vs = v2 - v1;

    TF32 x1f = ( Pos.x1 - rect.x1 ) / xs;
    TF32 y1f = ( Pos.y1 - rect.y1 ) / ys;
    TF32 x2f = ( Pos.x2 - rect.x1 ) / xs;
    TF32 y2f = ( Pos.y2 - rect.y1 ) / ys;

    u1f = us*x1f + u1;
    v1f = vs*y1f + v1;
    u2f = us*x2f + u1;
    v2f = vs*y2f + v1;
  }

  CColor Color = color;
  Color.A() = ( Color.A()*Opacity ) / 255;

  CVector2 TL, BR;
  TL.x = (TF32)Pos.x1;
  TL.y = (TF32)Pos.y1;
  BR.x = (TF32)Pos.x2;
  BR.y = (TF32)Pos.y2;

  CVector2 center = ( TL + BR ) / 2.0f;
  CVector2 a, b, c, d;
  a = CVector2( TL.x, TL.y ).Rotated( center, rotation );
  b = CVector2( BR.x, TL.y ).Rotated( center, rotation );
  c = CVector2( BR.x, BR.y ).Rotated( center, rotation );
  d = CVector2( TL.x, BR.y ).Rotated( center, rotation );

  DisplayList.AllocateNewUninitialized( 4 );
  TS32 idx = DisplayList.NumItems() - 4;

  DisplayList[ idx + 0 ].Initialize( a.x, a.y, u1f, v1f, Color );
  DisplayList[ idx + 1 ].Initialize( b.x, b.y, u2f, v1f, Color );
  DisplayList[ idx + 2 ].Initialize( c.x, c.y, u2f, v2f, Color );
  DisplayList[ idx + 3 ].Initialize( d.x, d.y, u1f, v2f, Color );
}

WBGUIVERTEX Lerp( WBGUIVERTEX &a, WBGUIVERTEX &b, float t )
{
  WBGUIVERTEX r;
  r.Pos = Lerp( a.Pos, b.Pos, t );
  r.UV = Lerp( a.UV, b.UV, t );
  r.Color = Lerp( a.Color, b.Color, t );
  return r;
}

void CWBDrawAPI::ClipTriX( TS32 x, TBOOL KeepRight, WBGUIVERTEX Vertices[ 6 ], TS32 &VertexCount )
{
  WBGUIVERTEX NewVertices[ 6 ];
  TS32 NewVertexCount = 0;

  if ( VertexCount > 6 )
  {
    LOG_ERR( "Something got fucked up" );
  }

  WBGUIVERTEX S = Vertices[ VertexCount - 1 ];
  for ( TS32 e = 0; e < VertexCount; e++ )
  {
    WBGUIVERTEX E = Vertices[ e ];

    TBOOL eInside = ( E.Pos.x > x && KeepRight ) || ( E.Pos.x < x && !KeepRight );
    TBOOL sInside = ( S.Pos.x > x && KeepRight ) || ( S.Pos.x < x && !KeepRight );

    if ( eInside )
    {
      if ( !sInside )
        NewVertices[ NewVertexCount++ ] = Lerp( S, E, ( x - S.Pos.x ) / ( E.Pos.x - S.Pos.x ) );
      NewVertices[ NewVertexCount++ ] = E;
    }
    else
      if ( sInside )
        NewVertices[ NewVertexCount++ ] = Lerp( S, E, ( x - S.Pos.x ) / ( E.Pos.x - S.Pos.x ) );
    S = E;
  }

  for ( TS32 y = 0; y < NewVertexCount; y++ )
    Vertices[ y ] = NewVertices[ y ];
  VertexCount = NewVertexCount;
}

void CWBDrawAPI::ClipTriY( TS32 y, TBOOL KeepBottom, WBGUIVERTEX Vertices[ 6 ], TS32 &VertexCount )
{
  WBGUIVERTEX NewVertices[ 6 ];
  TS32 NewVertexCount = 0;

  WBGUIVERTEX S = Vertices[ VertexCount - 1 ];
  for ( TS32 e = 0; e < VertexCount; e++ )
  {
    WBGUIVERTEX E = Vertices[ e ];

    TBOOL eInside = ( E.Pos.y > y && KeepBottom ) || ( E.Pos.y < y && !KeepBottom );
    TBOOL sInside = ( S.Pos.y > y && KeepBottom ) || ( S.Pos.y < y && !KeepBottom );

    if ( eInside )
    {
      if ( !sInside )
        NewVertices[ NewVertexCount++ ] = Lerp( S, E, ( y - S.Pos.y ) / ( E.Pos.y - S.Pos.y ) );
      NewVertices[ NewVertexCount++ ] = E;
    }
    else
      if ( sInside )
        NewVertices[ NewVertexCount++ ] = Lerp( S, E, ( y - S.Pos.y ) / ( E.Pos.y - S.Pos.y ) );
    S = E;
  }

  for ( TS32 x = 0; x < NewVertexCount; x++ )
    Vertices[ x ] = NewVertices[ x ];
  VertexCount = NewVertexCount;
}

void CWBDrawAPI::AddDisplayTri( const CPoint &_p1, const CPoint &_p2, const CPoint &_p3, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v2, const TF32 u3, const TF32 v3, const CColor a, const CColor b, const CColor c )
{
  CPoint p1 = _p1 + Offset;
  CPoint p2 = _p2 + Offset;
  CPoint p3 = _p3 + Offset;

  CRect Bound = CRect( p1, p1 );
  Bound.x1 = min( min( p1.x, p2.x ), p3.x );
  Bound.x2 = max( max( p1.x, p2.x ), p3.x );
  Bound.y1 = min( min( p1.y, p2.y ), p3.y );
  Bound.y2 = max( max( p1.y, p2.y ), p3.y );

  Bound |= CropRect | Bound;
  if ( Bound.Width() <= 0 || Bound.Height() <= 0 ) return;

  if ( DrawMode != WBD_TRIANGLES ) RenderDisplayList();
  DrawMode = WBD_TRIANGLES;

  WBGUIVERTEX Vertices[ 6 ]; //max vertex count is 6, when all triangle edges are cut
  TS32 VertexCount = 3;
  Vertices[ 0 ].Pos = CVector4( (TF32)p1.x, (TF32)p1.y, 0, 1 );
  Vertices[ 0 ].UV = CVector2( u1, v1 );
  Vertices[ 0 ].Color = a;
  Vertices[ 1 ].Pos = CVector4( (TF32)p2.x, (TF32)p2.y, 0, 1 );
  Vertices[ 1 ].UV = CVector2( u2, v2 );
  Vertices[ 1 ].Color = b;
  Vertices[ 2 ].Pos = CVector4( (TF32)p3.x, (TF32)p3.y, 0, 1 );
  Vertices[ 2 ].UV = CVector2( u3, v3 );
  Vertices[ 2 ].Color = c;

  if ( !CropRect.Contains( p1 ) || !CropRect.Contains( p2 ) || !CropRect.Contains( p3 ) )
  {
    ClipTriX( CropRect.x1, true, Vertices, VertexCount );
    ClipTriY( CropRect.y1, true, Vertices, VertexCount );
    ClipTriX( CropRect.x2, false, Vertices, VertexCount );
    ClipTriY( CropRect.y2, false, Vertices, VertexCount );
  }

  for ( TS32 x = 0; x < VertexCount; x++ )
  {
    CVector2 p = CVector2( Vertices[ x ].Pos.x, Vertices[ x ].Pos.y );
    Vertices[ x ].Pos = CVector4( p.x, p.y, 0, 1 );
  }

  for ( TS32 x = 2; x < VertexCount; x++ )
  {
    DisplayList += Vertices[ 0 ];
    DisplayList += Vertices[ x - 1 ];
    DisplayList += Vertices[ x ];
  }
}


void CWBDrawAPI::RenderDisplayList()
{
  if ( !DisplayList.NumItems() ) return;
  if ( !VertexBuffer ) return;

  //LOG_DBG("Render Display List");

  //update texture atlas if needed
  if ( Atlas && Device->GetTexture( CORESMP_PS0 ) == Atlas->GetTexture() ) Atlas->UpdateTexture();

  Device->SetVertexBuffer( VertexBuffer, 0 );

  TS32 VxCount = DisplayList.NumItems();
  while ( VxCount > 0 )
  {
    TS32 Count = min( VxCount, VERTEXBUFFERVERTEXCOUNT );

    void *Buffer = NULL;

    if ( !VertexBuffer->Lock( &Buffer, 0, Count * sizeof( WBGUIVERTEX ), CORELOCK_DISCARD ) )
    {
      LOG( LOG_ERROR, _T( "[gui] Error locking UI Vertex Buffer during draw." ) );
      return;
    }

    memcpy( Buffer, DisplayList.GetPointer( DisplayList.NumItems() - VxCount ), Count * sizeof( WBGUIVERTEX ) );

    if ( !VertexBuffer->UnLock() )
    {
      LOG( LOG_ERROR, _T( "[gui] Error unlocking UI Vertex Buffer during draw." ) );
      return;
    }

    switch ( DrawMode )
    {
    case WBD_RECTANGLES:
      if ( !Device->DrawIndexedTriangles( Count / 2, Count ) )
      {
        LOG( LOG_ERROR, _T( "[gui] Error drawing UI Triangles (%d %d)" ), Count / 2, Count );
        return;
      }
      break;
    case WBD_LINES:
      if ( !Device->DrawLines( Count / 2 ) )
      {
        LOG( LOG_ERROR, _T( "[gui] Error drawing UI Lines (%d)" ), Count / 2 );
        return;
      }
      break;
    case WBD_TRIANGLES:
      if ( !Device->DrawTriangles( Count / 3 ) )
      {
        LOG( LOG_ERROR, _T( "[gui] Error drawing UI Triangles (%d)" ), Count / 3 );
        return;
      }
      break;
    default:
      break;
    }

    VxCount -= Count;
  }


  DisplayList.FlushFast();
}

CWBDrawAPI::CWBDrawAPI()
{
  Offset = CPoint( 0, 0 );
  CropRect = CRect( 0, 0, 0, 0 );
  DrawMode = WBD_RECTANGLES;
  Atlas = NULL;

  App = NULL;
  Opacity = 255;

  GuiZState = NULL;
  GuiRasterState = NULL;
  GuiBlendState = NULL;
  ResolutionData = NULL;
  GuiSampler = NULL;
  VxShader = NULL;
  PxShader = NULL;
  VertexBuffer = NULL;
  VertexFormat = NULL;
  rectIndexBuffer = NULL;

  Device = NULL;
}

CWBDrawAPI::~CWBDrawAPI()
{
  SAFEDELETE( GuiZState );
  SAFEDELETE( GuiRasterState );
  SAFEDELETE( GuiBlendState );
  SAFEDELETE( ResolutionData );
  SAFEDELETE( GuiSampler );
  SAFEDELETE( VxShader );
  SAFEDELETE( PxShader );
  SAFEDELETE( VertexBuffer );
  SAFEDELETE( VertexFormat );
  SAFEDELETE( rectIndexBuffer );
}

TBOOL CWBDrawAPI::Initialize( CWBApplication *Application, CCoreDevice *Dev, CAtlas *atlas )
{
  App = Application;
  Device = Dev;
  Atlas = atlas;
  if ( !Device || !Atlas ) return false;

  VertexBuffer = Device->CreateVertexBufferDynamic( sizeof( WBGUIVERTEX )*VERTEXBUFFERRECTCOUNT * 4 );
  if ( !VertexBuffer )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Vertex Buffer" ) );
    return false;
  }

  rectIndexBuffer = Device->CreateIndexBuffer( VERTEXBUFFERRECTCOUNT * 6 );
  if ( !rectIndexBuffer )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Index Buffer" ) );
    return false;
  }

  TU16 *Locked = NULL;

  if ( rectIndexBuffer->Lock( (void**)&Locked ) )
  {
    for ( TS32 x = 0; x < VERTEXBUFFERRECTCOUNT; x++ )
    {
      Locked[ x * 6 + 0 ] = x * 4;
      Locked[ x * 6 + 1 ] = x * 4 + 1;
      Locked[ x * 6 + 2 ] = x * 4 + 2;
      Locked[ x * 6 + 3 ] = x * 4;
      Locked[ x * 6 + 4 ] = x * 4 + 2;
      Locked[ x * 6 + 5 ] = x * 4 + 3;
    }
    if ( !rectIndexBuffer->UnLock() )
    {
      LOG( LOG_ERROR, _T( "[gui] Error unlocking UI Index Buffer during init" ) );
      return false;
    }
  }
  else
  {
    LOG( LOG_ERROR, _T( "[gui] Error locking UI Index Buffer during init" ) );
    return false;
  }

  LPCSTR shader =
    "Texture2D GuiTexture:register(t0);"
    "SamplerState Sampler:register(s0);"
    "cbuffer resdata : register(b0)"
    "{							   "
    "		float4 resolution;	   "
    "}"
    "struct VSIN { float4 Position : POSITIONT; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
    "struct VSOUT { float4 Position : SV_POSITION; float2 UV : TEXCOORD0; float4 Color : COLOR0; };"
    "VSOUT vsmain(VSIN x) { VSOUT k; k.Position=float4(x.Position.x/resolution.x*2-1,-x.Position.y/resolution.y*2+1,0,1); k.UV=x.UV; k.Color=x.Color; return k; }"
    "float4 psmain(VSOUT x) : SV_TARGET0 { return x.Color*GuiTexture.Sample(Sampler,x.UV); }";

  unsigned char raw_ui_vxshader[] =
  {
    68,88,66,67,24,203,140,68,113,109,242,251,155,3,14,72,212,85,154,162,1,0,
    0,0,176,3,0,0,5,0,0,0,52,0,0,0,0,1,0,0,116,1,0,0,232,1,0,0,52,3,0,0,82,68,
    69,70,196,0,0,0,1,0,0,0,68,0,0,0,1,0,0,0,28,0,0,0,0,4,254,255,0,1,0,0,144,
    0,0,0,60,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,114,
    101,115,100,97,116,97,0,60,0,0,0,1,0,0,0,92,0,0,0,16,0,0,0,0,0,0,0,0,0,0,
    0,116,0,0,0,0,0,0,0,16,0,0,0,2,0,0,0,128,0,0,0,0,0,0,0,114,101,115,111,108,
    117,116,105,111,110,0,171,1,0,3,0,1,0,4,0,0,0,0,0,0,0,0,0,77,105,99,114,111,
    115,111,102,116,32,40,82,41,32,72,76,83,76,32,83,104,97,100,101,114,32,67,
    111,109,112,105,108,101,114,32,49,48,46,48,46,49,48,48,49,49,46,49,54,51,
    56,52,0,73,83,71,78,108,0,0,0,3,0,0,0,8,0,0,0,80,0,0,0,0,0,0,0,0,0,0,0,3,
    0,0,0,0,0,0,0,15,3,0,0,90,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,3,3,0,0,99,
    0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,2,0,0,0,15,15,0,0,80,79,83,73,84,73,79,78,84,
    0,84,69,88,67,79,79,82,68,0,67,79,76,79,82,0,171,171,171,79,83,71,78,108,
    0,0,0,3,0,0,0,8,0,0,0,80,0,0,0,0,0,0,0,1,0,0,0,3,0,0,0,0,0,0,0,15,0,0,0,92,
    0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,3,12,0,0,101,0,0,0,0,0,0,0,0,0,0,0,
    3,0,0,0,2,0,0,0,15,0,0,0,83,86,95,80,79,83,73,84,73,79,78,0,84,69,88,67,79,
    79,82,68,0,67,79,76,79,82,0,171,83,72,68,82,68,1,0,0,64,0,1,0,81,0,0,0,89,
    0,0,4,70,142,32,0,0,0,0,0,1,0,0,0,95,0,0,3,50,16,16,0,0,0,0,0,95,0,0,3,50,
    16,16,0,1,0,0,0,95,0,0,3,242,16,16,0,2,0,0,0,103,0,0,4,242,32,16,0,0,0,0,
    0,1,0,0,0,101,0,0,3,50,32,16,0,1,0,0,0,101,0,0,3,242,32,16,0,2,0,0,0,104,
    0,0,2,1,0,0,0,14,0,0,8,18,0,16,0,0,0,0,0,10,16,16,0,0,0,0,0,10,128,32,0,0,
    0,0,0,0,0,0,0,50,0,0,9,18,32,16,0,0,0,0,0,10,0,16,0,0,0,0,0,1,64,0,0,0,0,
    0,64,1,64,0,0,0,0,128,191,14,0,0,9,18,0,16,0,0,0,0,0,26,16,16,128,65,0,0,
    0,0,0,0,0,26,128,32,0,0,0,0,0,0,0,0,0,50,0,0,9,34,32,16,0,0,0,0,0,10,0,16,
    0,0,0,0,0,1,64,0,0,0,0,0,64,1,64,0,0,0,0,128,63,54,0,0,8,194,32,16,0,0,0,
    0,0,2,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,128,63,54,0,0,5,50,32,16,0,1,0,0,
    0,70,16,16,0,1,0,0,0,54,0,0,5,242,32,16,0,2,0,0,0,70,30,16,0,2,0,0,0,62,0,
    0,1,83,84,65,84,116,0,0,0,8,0,0,0,1,0,0,0,0,0,0,0,6,0,0,0,4,0,0,0,0,0,0,0,
    0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  };

  int raw_ui_vxshader_size = 944;

  unsigned char raw_ui_pxshader[] =
  {
    68,88,66,67,24,211,192,194,1,98,151,138,225,231,139,185,192,223,161,191,1,
    0,0,0,160,2,0,0,5,0,0,0,52,0,0,0,224,0,0,0,84,1,0,0,136,1,0,0,36,2,0,0,82,
    68,69,70,164,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,28,0,0,0,0,4,255,255,0,1,0,0,111,
    0,0,0,92,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,100,
    0,0,0,2,0,0,0,5,0,0,0,4,0,0,0,255,255,255,255,0,0,0,0,1,0,0,0,13,0,0,0,83,
    97,109,112,108,101,114,0,71,117,105,84,101,120,116,117,114,101,0,77,105,99,
    114,111,115,111,102,116,32,40,82,41,32,72,76,83,76,32,83,104,97,100,101,114,
    32,67,111,109,112,105,108,101,114,32,49,48,46,48,46,49,48,48,49,49,46,49,
    54,51,56,52,0,171,73,83,71,78,108,0,0,0,3,0,0,0,8,0,0,0,80,0,0,0,0,0,0,0,
    1,0,0,0,3,0,0,0,0,0,0,0,15,0,0,0,92,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,
    3,3,0,0,101,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,2,0,0,0,15,15,0,0,83,86,95,80,79,
    83,73,84,73,79,78,0,84,69,88,67,79,79,82,68,0,67,79,76,79,82,0,171,79,83,
    71,78,44,0,0,0,1,0,0,0,8,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,15,
    0,0,0,83,86,95,84,65,82,71,69,84,0,171,171,83,72,68,82,148,0,0,0,64,0,0,0,
    37,0,0,0,90,0,0,3,0,96,16,0,0,0,0,0,88,24,0,4,0,112,16,0,0,0,0,0,85,85,0,
    0,98,16,0,3,50,16,16,0,1,0,0,0,98,16,0,3,242,16,16,0,2,0,0,0,101,0,0,3,242,
    32,16,0,0,0,0,0,104,0,0,2,1,0,0,0,69,0,0,9,242,0,16,0,0,0,0,0,70,16,16,0,
    1,0,0,0,70,126,16,0,0,0,0,0,0,96,16,0,0,0,0,0,56,0,0,7,242,32,16,0,0,0,0,
    0,70,14,16,0,0,0,0,0,70,30,16,0,2,0,0,0,62,0,0,1,83,84,65,84,116,0,0,0,3,
    0,0,0,1,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,
  };

  int raw_ui_pxshader_size = 672;

  //(a)/(TF32)(c)*2.0f-1,(-b)/(TF32)(d)*2.0f+1

  VxShader = Device->CreateVertexShaderFromBlob( raw_ui_vxshader, raw_ui_vxshader_size );// ( shader, (TS32)strlen( shader ), "vsmain", "vs_4_0" );
  if ( !VxShader )
  {
    LOG( LOG_WARNING, _T( "[gui] Couldn't compile GUI VertexShader - this won't affect DX9 functionality" ) );
    if ( Device->GetAPIType() == COREAPI_DX11 ) return false;
  }

  PxShader = Device->CreatePixelShaderFromBlob( raw_ui_pxshader, raw_ui_pxshader_size );// shader, (TS32)strlen( shader ), "psmain", "ps_4_0" );
  if ( !PxShader )
  {
    LOG( LOG_WARNING, _T( "[gui] Couldn't compile GUI PixelShader - this won't affect DX9 functionality" ) );
    if ( Device->GetAPIType() == COREAPI_DX11 ) return false;
  }

  COREVERTEXATTRIBUTE *vx = WBGuiVertexFormat;
  CArray<COREVERTEXATTRIBUTE> Att;
  while ( *vx != COREVXATTR_STOP ) Att += *vx++;

  VertexFormat = Device->CreateVertexFormat( Att, VxShader );
  if ( !VertexFormat )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Vertex Format" ) );
    return false;
  }


  GuiSampler = Device->CreateSamplerState();
  GuiBlendState = Device->CreateBlendState();
  if ( !GuiBlendState )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Blend State" ) );
    return false;
  }

  GuiBlendState->SetBlendEnable( 0, true );
  GuiBlendState->SetSrcBlend( 0, COREBLEND_SRCALPHA );
  GuiBlendState->SetDestBlend( 0, COREBLEND_INVSRCALPHA );
  GuiBlendState->SetSrcBlendAlpha( 0, COREBLEND_ONE );
  GuiBlendState->SetDestBlendAlpha( 0, COREBLEND_INVSRCALPHA );

  GuiRasterState = Device->CreateRasterizerState();
  if ( !GuiRasterState )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Raster State" ) );
    return false;
  }

  GuiRasterState->SetAntialiasedLineEnable( true );
  GuiRasterState->SetCullMode( CORECULL_NONE );

  GuiZState = Device->CreateDepthStencilState();
  if ( !GuiZState )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Z State" ) );
    return false;
  }

  GuiZState->SetDepthFunc( CORECMP_LEQUAL );

  ResolutionData = Device->CreateConstantBuffer();
  if ( !ResolutionData )
  {
    LOG( LOG_ERROR, _T( "[gui] Error creating UI Resolution Data Buffer" ) );
    return false;
  }

  UVOffset = Device->GetUVOffset();

  return true;
}

void CWBDrawAPI::SetOffset( CPoint &p )
{
  Offset = p;
}

void CWBDrawAPI::SetCropRect( CRect &r )
{
  CropRect = ParentCropRect | r;
}

void CWBDrawAPI::DrawRect( CRect &r, CColor Color )
{
  CPoint white = Atlas->GetWhitePixelUV();
  TF32 u, v;
  u = UVTRANSLATION( white.x, Atlas->GetXRes() );
  v = UVTRANSLATION( white.y, Atlas->GetYRes() );
  AddDisplayRect( r, u, v, u, v, Color );
}

void CWBDrawAPI::DrawRectBorder( CRect &r, CColor Color )
{
  DrawRect( CRect( r.TopLeft(), r.BottomLeft() + CPoint( 1, 0 ) ), Color );
  DrawRect( CRect( r.TopLeft(), r.TopRight() + CPoint( 0, 1 ) ), Color );
  DrawRect( CRect( r.TopRight() - CPoint( 1, 0 ), r.BottomRight() ), Color );
  DrawRect( CRect( r.BottomLeft() - CPoint( 0, 1 ), r.BottomRight() ), Color );
}

void CWBDrawAPI::DrawRectRotated( CRect &r, TF32 u1, TF32 v1, TF32 u2, TF32 v2, CColor Color, TF32 rotation )
{
  AddDisplayRectRotated( r, u1, v1, u2, v2, Color, rotation );
}

void CWBDrawAPI::DrawRect( CRect &r, TF32 u1, TF32 v1, TF32 u2, TF32 v2 )
{
  AddDisplayRect( r, u1, v1, u2, v2, 0xffffffff );
}

void CWBDrawAPI::DrawRect( CRect &r, TF32 u1, TF32 v1, TF32 u2, TF32 v2, CColor Color )
{
  AddDisplayRect( r, u1, v1, u2, v2, Color );
}

void CWBDrawAPI::DrawLine( CPoint &p1, CPoint &p2, CColor Color )
{
  CPoint white = Atlas->GetWhitePixelUV();
  AddDisplayLine( p1, p2, UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ), UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ), Color, Color );
}

void CWBDrawAPI::DrawLine( CPoint &p1, CPoint &p2, CColor Color1, CColor Color2 )
{
  CPoint white = Atlas->GetWhitePixelUV();
  AddDisplayLine( p1, p2, UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ), UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ), Color1, Color2 );
}

void CWBDrawAPI::DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, CColor Color )
{
  CPoint white = Atlas->GetWhitePixelUV();
  AddDisplayTri( p1, p2, p3,
                 UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ),
                 UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ),
                 UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ),
                 Color, Color, Color );
}

void CWBDrawAPI::DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, CColor a, CColor b, CColor c )
{
  CPoint white = Atlas->GetWhitePixelUV();
  AddDisplayTri( p1, p2, p3,
                 UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ),
                 UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ),
                 UVTRANSLATION( white.x, Atlas->GetXRes() ), UVTRANSLATION( white.y, Atlas->GetYRes() ),
                 a, b, c );
}

void CWBDrawAPI::DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, TF32 u1, TF32 v1, TF32 u2, TF32 v2, TF32 u3, TF32 v3, CColor a, CColor b, CColor c )
{
  AddDisplayTri( p1, p2, p3, u1, v1, u2, v2, u3, v3, a, b, c );
}

void CWBDrawAPI::DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, TF32 u1, TF32 v1, TF32 u2, TF32 v2, TF32 u3, TF32 v3 )
{
  AddDisplayTri( p1, p2, p3, u1, v1, u2, v2, u3, v3, 0xffffffff, 0xffffffff, 0xffffffff );
}

void CWBDrawAPI::SetUIRenderState()
{
  Device->SetViewport( CRect( 0, 0, App->GetXRes(), App->GetYRes() ) );
  Device->SetIndexBuffer( rectIndexBuffer );

  Device->SetRenderState( GuiBlendState );
  Device->SetRenderState( GuiZState );
  Device->SetRenderState( GuiRasterState );

  Device->SetVertexShader( VxShader );
  Device->SetVertexFormat( VertexFormat );
  Device->SetVertexBuffer( VertexBuffer, 0 );
  Device->SetGeometryShader( NULL );
  Device->SetHullShader( NULL );
  Device->SetDomainShader( NULL );
  Device->SetPixelShader( PxShader );
  Device->SetTexture( CORESMP_PS0, Atlas->GetTexture() );

  Device->SetRenderTarget( NULL );

  if ( GuiSampler ) GuiSampler->Apply( CORESMP_PS0 );

  ResolutionData->Reset();

  CVector4 Resolution = CVector4( (TF32)App->GetXRes(), (TF32)App->GetYRes(), 0, 1 );
  ResolutionData->AddData( &Resolution, sizeof( CVector4 ) );
  ResolutionData->Upload();
  Device->SetShaderConstants( 0, 1, &ResolutionData );
}

void CWBDrawAPI::SetUIBlendState( CCoreBlendState *BlendState )
{
  SAFEDELETE( GuiBlendState );
  GuiBlendState = BlendState;
}

void CWBDrawAPI::SetUISamplerState( CCoreSamplerState *SamplerState )
{
  SAFEDELETE( GuiSampler );
  GuiSampler = SamplerState;
}


void CWBDrawAPI::SetRenderView( CRect r )
{
  Device->SetViewport( r );
}

CSize CWBDrawAPI::GetAtlasElementSize( WBATLASHANDLE h )
{
  if ( !Atlas ) return CSize( 0, 0 );
  return Atlas->GetSize( h );
}

static TS32 defragmentReportCount = 0;

TBOOL CWBDrawAPI::RequestAtlasImageUse( WBATLASHANDLE h, CRect &UV )
{
  if ( !Atlas ) return false;

  //LOG_DBG("Request Atlas Image Use for %d",h);

  if ( !Atlas->RequestImageUse( h, UV ) )
  {
    LOG_DBG( "Request Atlas Image Failed for %d", h );
    //Atlas->GetTexture()->ExportToPNG(CString::Format(_T("atlasdump%.4d.png"), counter++));

    //LOG_DBG("Failed Request Atlas Image Use for %d", h);
    //atlas is full, attempt to fix

    //first flush display buffer to avoid problems from remapping the atlas
    RenderDisplayList();

    //next make space in the atlas
    Atlas->Optimize();
    //Atlas->GetTexture()->ExportToPNG(CString::Format(_T("atlasdump%.4d.png"), counter++));

    //Try adding the image again
    if ( !Atlas->RequestImageUse( h, UV ) )
    {
      //Atlas->GetTexture()->ExportToPNG(CString::Format(_T("atlasdump%.4d.png"), counter++));
      //atlas is really, really, really full. fail.

      if ( defragmentReportCount == 100 )
        LOG( LOG_ERROR, _T( "[gui] MID-FRAME TEXTURE ATLAS OPTIMIZATION REPORT COUNT REACHED 100, STOPPING REPORTING" ) );

      if ( defragmentReportCount < 100 )
        LOG( LOG_ERROR, _T( "[gui] GUI Texture Atlas too small to render current frame. Flushing mid-frame, thiss will result in reduced performance." ) );

      defragmentReportCount++;

      Atlas->Reset();
      if ( !Atlas->RequestImageUse( h, UV ) )
      {
        LOG( LOG_ERROR, _T( "[gui] Image %d does not fit the atlas." ), h );
        return false;
      }
    }
  }

  return true;
}

void CWBDrawAPI::DrawAtlasElement( WBATLASHANDLE h, TS32 x, TS32 y, CColor Color )
{
  if ( !Atlas ) return;

  CPoint pos = CPoint( x, y ) + Offset;

  if ( !CropRect.Intersects( CRect( pos, pos + Atlas->GetSize( h ) ) ) ) return; //pre-cull invisible items to spare unneeded atlas use

  CRect UV;
  if ( !RequestAtlasImageUse( h, UV ) ) return;

  DrawRect( CRect( x, y, UV.Width() + x, UV.Height() + y ),
            UVTRANSLATION( UV.x1, Atlas->GetXRes() ),
            UVTRANSLATION( UV.y1, Atlas->GetYRes() ),
            UVTRANSLATION( UV.x2, Atlas->GetXRes() ),
            UVTRANSLATION( UV.y2, Atlas->GetYRes() ), Color );
}

void CWBDrawAPI::DrawAtlasElement( WBATLASHANDLE h, CRect &Position, TBOOL TileX, TBOOL TileY, TBOOL StretchX, TBOOL StretchY, CColor Color/*=0xffffffff*/ )
{
  if ( !Atlas ) return;

  CRect rect = CropRect | ( Position + Offset );
  if ( rect.Width() <= 0 || rect.Height() <= 0 ) return; //pre-cull invisible items to spare unneeded atlas use

  CRect UV;
  if ( !RequestAtlasImageUse( h, UV ) ) return;

  CSize tilesize = Position.Size();
  if ( TileX )
    tilesize.x = UV.Width();
  if ( TileY )
    tilesize.y = UV.Height();

  CVector2 uvmod = CVector2( 0, 0 );

  CRect target = CRect( CPoint( 0, 0 ), UV.Size() ) + Position.TopLeft();
  target.x2 = target.x1 + Position.Width();
  if ( !TileX && StretchX ) uvmod.x = 0.5;

  if ( TileX ) //crop unneeded tiles
  {
    if ( target.x1 < CropRect.x1 - Offset.x ) target.x1 += ( ( CropRect.x1 - Offset.x - target.x1 ) / tilesize.x )*tilesize.x;
    if ( target.x2 > CropRect.x2 - Offset.x ) target.x2 = CropRect.x2;
  }

  target.y2 = target.y1 + Position.Height();
  if ( !TileY && StretchY ) uvmod.y = 0.5;

  if ( TileY ) //crop unneeded tiles
  {
    if ( target.y1 < CropRect.y1 - Offset.y ) target.y1 += ( ( CropRect.y1 - Offset.y - target.y1 ) / tilesize.y )*tilesize.y;
    if ( target.y2 > CropRect.y2 - Offset.y ) target.y2 = CropRect.y2;
  }

  CRect p = CRect( CPoint( 0, 0 ), tilesize );

  CRect cr = CropRect;
  SetCropRect( target + Offset );

  for ( TS32 x = target.x1; x < target.x2; x += tilesize.x )
    for ( TS32 y = target.y1; y < target.y2; y += tilesize.y )
    {
      DrawRect( p + CPoint( x, y ),
                UVTRANSLATION( UV.x1 + uvmod.x, Atlas->GetXRes() ),
                UVTRANSLATION( UV.y1 + uvmod.y, Atlas->GetYRes() ),
                UVTRANSLATION( UV.x2 - uvmod.x, Atlas->GetXRes() ),
                UVTRANSLATION( UV.y2 - uvmod.y, Atlas->GetYRes() ), Color );
    }

  CropRect = cr;

}

void CWBDrawAPI::DrawAtlasElementRotated( WBATLASHANDLE h, CRect& position, CColor Color, TF32 rotation )
{
  if ( !Atlas ) return;

  CRect pos = position + Offset;

  if ( !CropRect.Intersects( pos ) ) return; //pre-cull invisible items to spare unneeded atlas use

  CRect UV;
  if ( !RequestAtlasImageUse( h, UV ) ) return;

  DrawRectRotated( pos,
                   UVTRANSLATION( UV.x1 + 0.5f, Atlas->GetXRes() ),
                   UVTRANSLATION( UV.y1 + 0.5f, Atlas->GetYRes() ),
                   UVTRANSLATION( UV.x2 - 0.5f, Atlas->GetXRes() ),
                   UVTRANSLATION( UV.y2 - 0.5f, Atlas->GetYRes() ), Color, rotation );
}

void CWBDrawAPI::SetCropToClient( CWBItem *i )
{
  if ( !i ) return;
  SetCropRect( GetCropRect() | i->ClientToScreen( i->GetClientRect() ) );
}

void CWBDrawAPI::FlushDrawBuffer()
{
  RenderDisplayList();
}

CCoreDevice * CWBDrawAPI::GetDevice()
{
  return Device;
}

CCoreConstantBuffer * CWBDrawAPI::GetResolutionData()
{
  return ResolutionData;
}

void CWBDrawAPI::SetPixelShader( CCorePixelShader *shader )
{
  SAFEDELETE( PxShader );
  PxShader = shader;
}

void CWBDrawAPI::SetOpacity( TU8 o )
{
  Opacity = o;
}

void ZoomToMouseCenter( CPoint &Offset, TS32 &Zoom, TS32 NewZoom, CPoint ZoomCenter )
{
  //dz=zoom'/zoom
  //xoff'=xpos+xoff-xpos*dz
  float dz = NewZoom / (float)Zoom;

  CVector2 v = CVector2( (TF32)ZoomCenter.x, (TF32)ZoomCenter.y )*( 1 - dz );
  Offset += CPoint( (TS32)v.x, (TS32)v.y );

  Zoom = NewZoom;
}

void ZoomToMouseCenter( CPoint &Offset, TF32 &Zoom, TF32 NewZoom, CPoint Pos )
{
  //dz=zoom'/zoom
  //xoff'=xpos+xoff-xpos*dz
  TF32 dz = NewZoom / (TF32)Zoom;

  Offset.x = (TS32)( Pos.x + Offset.x - Pos.x*dz );
  Offset.y = (TS32)( Pos.y + Offset.y - Pos.y*dz );

  Zoom = NewZoom;
}