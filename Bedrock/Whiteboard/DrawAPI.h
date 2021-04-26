#pragma once

#include "../CoRE2/Core2.h"
#include "Atlas.h"
#include "Font.h"

//struct WBDISPLAYRECT
//{
//	CRect Pos;
//	TF32 u1,v1,u2,v2;
//	//CColor a,b,c,d;
//};

struct WBDISPLAYLINE
{
  CPoint p1, p2;
  TF32 u1, v1, u2, v2;
  CColor c1, c2;
};

struct WBGUIVERTEX
{
  CVector4 Pos;
  CVector2 UV;
  CColor Color;

  WBGUIVERTEX()
  {

  }

  WBGUIVERTEX( const CVector2 &pos, const CVector2 &uv, const CColor &color )
  {
    Initialize( pos.x, pos.y, uv.x, uv.y, color );
  }

  INLINE void Initialize( const TF32 x, const TF32 y, const TF32 u, const TF32 v, const CColor &color )
  {
    Pos.x = x;
    Pos.y = y;
    Pos.z = 0;
    Pos.w = 1;
    UV.x = u;
    UV.y = v;
    Color = color;
  }

};

enum WBDRAWMODE
{
  WBD_RECTANGLES,
  WBD_LINES,
  WBD_TRIANGLES
};

class CWBDrawAPI
{
  friend class CWBApplication;
  friend class CWBItem; //these are the only things that need to access the renderdisplaylist function

  CPoint Offset; //screen space top left corner of the client rect for the currently drawn item
  CRect CropRect; //screen space window rect for the currently drawn item
  WBDRAWMODE DrawMode;
  TU8 Opacity;
  CWBApplication *App;

  CArray<WBGUIVERTEX> DisplayList;
  CAtlas *Atlas;
  CCoreDevice *Device;

  TF32 UVOffset = 0; //texel offset to fix 0.5 texel shift in Directx9

  CRect ParentCropRect;

  CCoreIndexBuffer *rectIndexBuffer;
  CCoreVertexBuffer *VertexBuffer;
  CCoreVertexFormat *VertexFormat;

  CCoreVertexShader *VxShader;
  CCorePixelShader *PxShader;

  CCoreSamplerState *GuiSampler;
  CCoreBlendState *GuiBlendState;
  CCoreRasterizerState *GuiRasterState;
  CCoreDepthStencilState *GuiZState;

  CCoreConstantBuffer *ResolutionData;

  void AddDisplayRect( const CRect &r, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v3, const CColor a );
  void AddDisplayRectRotated( const CRect &r, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v3, const CColor a, TF32 rotation );
  void AddDisplayLine( const CPoint &p1, const CPoint &p2, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v2, const CColor a, const CColor b );
  void AddDisplayTri( const CPoint &p1, const CPoint &p2, const CPoint &p3, const TF32 u1, const TF32 v1, const TF32 u2, const TF32 v2, const TF32 u3, const TF32 v3, const CColor a, const CColor b, const CColor c );
  void ClipTriX( TS32 x, TBOOL KeepRight, WBGUIVERTEX Vertices[ 6 ], TS32 &VertexCount );
  void ClipTriY( TS32 y, TBOOL KeepBottom, WBGUIVERTEX Vertices[ 6 ], TS32 &VertexCount );
  void RenderDisplayList();

  TBOOL RequestAtlasImageUse( WBATLASHANDLE h, CRect &r );

public:

  CWBDrawAPI();
  virtual ~CWBDrawAPI();

  TBOOL Initialize( CWBApplication *App, CCoreDevice *Device, CAtlas *Atlas );

  void DrawRect( CRect &r, CColor Color );
  void DrawRect( CRect &r, TF32 u1, TF32 v1, TF32 u2, TF32 v2 );
  void DrawRect( CRect &r, TF32 u1, TF32 v1, TF32 u2, TF32 v2, CColor Color );
  void DrawRectBorder( CRect &r, CColor Color );
  void DrawRectRotated( CRect &r, TF32 u1, TF32 v1, TF32 u2, TF32 v2, CColor Color, TF32 rotation );

  void DrawLine( CPoint &p1, CPoint &p2, CColor Color );
  void DrawLine( CPoint &p1, CPoint &p2, CColor Color1, CColor Color2 );

  void DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, CColor Color );
  void DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, CColor a, CColor b, CColor c );
  void DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, TF32 u1, TF32 v1, TF32 u2, TF32 v2, TF32 u3, TF32 v3, CColor a, CColor b, CColor c );
  void DrawTriangle( CPoint &p1, CPoint &p2, CPoint &p3, TF32 u1, TF32 v1, TF32 u2, TF32 v2, TF32 u3, TF32 v3 );

  void SetOffset( CPoint &p );
  void SetCropRect( CRect &r );
  void SetParentCropRect( CRect &r ) { ParentCropRect = r; };
  INLINE CRect &GetCropRect() { return CropRect; }
  INLINE CRect &GetParentCropRect() { return ParentCropRect; }
  INLINE CPoint &GetOffset() { return Offset; }

  void SetUIRenderState();
  void SetUIBlendState( CCoreBlendState *BlendState );
  void SetUISamplerState( CCoreSamplerState *SamplerState );

  CSize GetAtlasElementSize( WBATLASHANDLE h );
  void DrawAtlasElement( WBATLASHANDLE h, TS32 x, TS32 y, CColor Color = 0xffffffff );
  void DrawAtlasElement( WBATLASHANDLE h, CRect &Position, TBOOL TileX, TBOOL TileY, TBOOL StretchX, TBOOL StretchY, CColor Color = 0xffffffff );
  void DrawAtlasElementRotated( WBATLASHANDLE h, CRect& Position, CColor Color, TF32 rotation );
  void SetCropToClient( CWBItem *i );

  void FlushDrawBuffer();
  CCoreDevice *GetDevice();
  CCoreConstantBuffer *GetResolutionData();

  void SetOpacity( TU8 o );
  void SetRenderView( CRect r );

  void SetPixelShader( CCorePixelShader *shader );
};

//helper functions for common use cases
void ZoomToMouseCenter( CPoint &Offset, TS32 &Zoom, TS32 NewZoom, CPoint ZoomCenter );
void ZoomToMouseCenter( CPoint &Offset, TF32 &Zoom, TF32 NewZoom, CPoint Pos );
