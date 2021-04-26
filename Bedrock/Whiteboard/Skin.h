#pragma once
#include "DrawAPI.h"

enum WBMETRICTYPE
{
  WB_UNDEFINED = 0,
  WB_PIXELS,
  WB_RELATIVE,

  WB_METRIC_COUNT, //used as array size, don't remove
};

class CWBMetricValue
{
  TF32 Metrics[ WB_METRIC_COUNT ];
  TBOOL MetricsUsed[ WB_METRIC_COUNT ];
  TBOOL AutoSize;

public:

  CWBMetricValue();
  void SetMetric( WBMETRICTYPE w, TF32 Value );
  void SetValue( TF32 Relative, TF32 Pixels );
  TF32 GetValue( TF32 ParentSize, TS32 ContentSize );
  void SetAutoSize( TBOOL Auto );
  TBOOL IsAutoResizer();
};

enum WBPOSITIONTYPE
{
  WB_MARGIN_LEFT = 0,
  WB_MARGIN_RIGHT = 1,
  WB_MARGIN_TOP = 2,
  WB_MARGIN_BOTTOM = 3,
  WB_WIDTH = 4,
  WB_HEIGHT = 5,
  WB_PADDING_LEFT = 6,
  WB_PADDING_RIGHT = 7,
  WB_PADDING_TOP = 8,
  WB_PADDING_BOTTOM = 9
};

class CWBPositionDescriptor
{
  CDictionary<WBPOSITIONTYPE, CWBMetricValue> Positions;

public:

  void SetValue( WBPOSITIONTYPE p, TF32 Relative, TF32 Pixels );
  void SetMetric( WBPOSITIONTYPE p, WBMETRICTYPE m, TF32 Value );
  void SetAutoSize( WBPOSITIONTYPE p );
  void ClearMetrics( WBPOSITIONTYPE p );
  CRect GetPosition( CSize ParentSize, CSize ContentSize, CRect &Original );
  CRect GetPadding( CSize ParentSize, CRect &BorderSize );

  TBOOL IsWidthSet();
  TBOOL IsHeightSet();
  TS32 GetWidth( CSize ParentSize, CSize ContentSize );
  TS32 GetHeight( CSize ParentSize, CSize ContentSize );

  TBOOL IsAutoResizer();
};

class CWBPositionDescriptorPixels
{
  TBOOL Set[ 6 ];
  TS32 Positions[ 6 ];

public:

  CWBPositionDescriptorPixels();
  void SetValue( WBPOSITIONTYPE p, TS32 Pixels );
  FORCEINLINE CRect GetPosition( CSize ParentSize );
};

enum WBSKINELEMENTBEHAVIOR
{
  WB_SKINBEHAVIOR_PIXELCORRECT = 0,
  WB_SKINBEHAVIOR_STRETCH,
  WB_SKINBEHAVIOR_TILE,
};

enum WBRECTSIDE
{
  WB_RECTSIDE_LEFT = 0,
  WB_RECTSIDE_TOP = 1,
  WB_RECTSIDE_RIGHT = 2,
  WB_RECTSIDE_BOTTOM = 3,
};

class CWBSkinElement
{
  CString Name;
  WBATLASHANDLE Handle;
  WBSKINELEMENTBEHAVIOR DefaultBehavior[ 2 ]; //x-y stretching behaviors

public:

  CWBSkinElement();
  CWBSkinElement( const CWBSkinElement &Copy );
  CWBSkinElement &operator=( const CWBSkinElement &Copy );

  void SetHandle( WBATLASHANDLE h );
  void SetBehavior( TS32 Axis, WBSKINELEMENTBEHAVIOR Behavior );
  WBSKINELEMENTBEHAVIOR GetBehavior( TS32 Axis );
  void SetName( CString Name );
  WBATLASHANDLE GetHandle();
  CString &GetName();

  FORCEINLINE void Render( CWBDrawAPI *API, CRect &Pos );
  CSize GetElementSize( CWBDrawAPI *API );
};

class CWBMosaicImage
{
  CWBPositionDescriptorPixels Position;
  TBOOL Tiling[ 2 ];
  TBOOL Stretching[ 2 ];
  WBATLASHANDLE Handle;
  CColor Color;

public:

  CWBMosaicImage();
  void SetPositionValue( WBPOSITIONTYPE p, TS32 Pixels );
  void SetTiling( TS32 Axis, TBOOL v );
  void SetStretching( TS32 Axis, TBOOL v );
  void SetHandle( WBATLASHANDLE handle );
  void SetColor( CColor color );

  FORCEINLINE void Render( CWBDrawAPI *API, CRect &Pos );
};

class CWBMosaic
{
  CString Name;
  CArray<CWBMosaicImage> Images;
  TS32 Overshoot[ 4 ];

public:

  CWBMosaic();
  CWBMosaic( const CWBMosaic &Copy );
  CWBMosaic &operator=( const CWBMosaic &Copy );

  void SetName( CString Name );
  CString &GetName();
  void AddImage( CWBMosaicImage &Image );
  void Flush();
  void Render( CWBDrawAPI *API, CRect &Position );
  void SetOverShoot( WBRECTSIDE side, TS32 val );
};

typedef TU32 WBSKINELEMENTID;

class CWBSkin
{
  CArray<CWBSkinElement> SkinItems;
  CArray<CWBMosaic> Mosaics;

  CWBSkinElement *GetElement( CString &Name );

public:

  void AddElement( const CString &Name, WBATLASHANDLE Handle, WBSKINELEMENTBEHAVIOR Xbehav, WBSKINELEMENTBEHAVIOR Ybehav );
  CWBMosaic *AddMosaic( const CString &Name, CString &Description, TS32 OverShootLeft = 0, TS32 OverShootTop = 0, TS32 OverShootRight = 0, TS32 OverShootBottom = 0 );

  void RenderElement( CWBDrawAPI *API, WBSKINELEMENTID ID, CRect &Pos );
  void RenderElement( CWBDrawAPI *API, CString &Name, CRect &Pos );
  void RenderElement( CWBDrawAPI *API, TCHAR *Name, CRect &Pos );
  WBSKINELEMENTID GetElementID( CString &Name );
  CWBSkinElement *GetElement( WBSKINELEMENTID id );
  CSize GetElementSize( CWBDrawAPI *API, WBSKINELEMENTID id );
};