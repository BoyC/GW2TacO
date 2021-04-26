#include "BasePCH.h"
#include "Skin.h"

//////////////////////////////////////////////////////////////////////////
// metrics

CWBMetricValue::CWBMetricValue()
{
  AutoSize = false;
  memset( Metrics, 0, sizeof( Metrics ) );
  memset( MetricsUsed, 0, sizeof( MetricsUsed ) );
}

void CWBMetricValue::SetMetric( WBMETRICTYPE w, TF32 Value )
{
  Metrics[ w ] = Value;
  MetricsUsed[ w ] = true;
}

void CWBMetricValue::SetValue( TF32 Relative, TF32 Pixels )
{
  Metrics[ WB_RELATIVE ] = Relative;
  Metrics[ WB_PIXELS ] = Pixels;
  MetricsUsed[ WB_RELATIVE ] = true;
  MetricsUsed[ WB_PIXELS ] = true;
}

TF32 CWBMetricValue::GetValue( TF32 ParentSize, TS32 ContentSize )
{
  if ( AutoSize ) return ContentSize + 0.5f;
  TF32 v = 0;
  if ( MetricsUsed[ WB_PIXELS ] )		v += Metrics[ WB_PIXELS ];
  if ( MetricsUsed[ WB_RELATIVE ] )	v += Metrics[ WB_RELATIVE ] * ParentSize;
  return v;
}

TBOOL CWBMetricValue::IsAutoResizer()
{
  return AutoSize;
}

void CWBMetricValue::SetAutoSize( TBOOL Auto )
{
  AutoSize = Auto;
}

//////////////////////////////////////////////////////////////////////////
// position descriptor - general

CRect CWBPositionDescriptor::GetPosition( CSize ParentSize, CSize ContentSize, CRect &Original )
{
  CRect r( 0, 0, 0, 0 );

  TS32 Width = 0;
  TS32 Height = 0;
  TS32 Top = 0;
  TS32 Left = 0;
  TS32 Right = 0;
  TS32 Bottom = 0;

  TBOOL WidthSet = Positions.HasKey( WB_WIDTH );
  TBOOL HeightSet = Positions.HasKey( WB_HEIGHT );
  TBOOL TopSet = Positions.HasKey( WB_MARGIN_TOP );
  TBOOL LeftSet = Positions.HasKey( WB_MARGIN_LEFT );
  TBOOL RightSet = Positions.HasKey( WB_MARGIN_RIGHT );
  TBOOL BottomSet = Positions.HasKey( WB_MARGIN_BOTTOM );

  if ( WidthSet )	Width = (TS32)Positions[ WB_WIDTH ].GetValue( (TF32)ParentSize.x, ContentSize.x );
  if ( HeightSet )	Height = (TS32)Positions[ WB_HEIGHT ].GetValue( (TF32)ParentSize.y, ContentSize.y );
  if ( TopSet )		Top = (TS32)Positions[ WB_MARGIN_TOP ].GetValue( (TF32)ParentSize.y, 0 );
  if ( LeftSet )	Left = (TS32)Positions[ WB_MARGIN_LEFT ].GetValue( (TF32)ParentSize.x, 0 );
  if ( RightSet )	Right = (TS32)Positions[ WB_MARGIN_RIGHT ].GetValue( (TF32)ParentSize.x, 0 );
  if ( BottomSet )	Bottom = (TS32)Positions[ WB_MARGIN_BOTTOM ].GetValue( (TF32)ParentSize.y, 0 );

  r.x1 = LeftSet ? Left : ( ParentSize.x - ( Right + Width ) );
  r.y1 = TopSet ? Top : ( ParentSize.y - ( Bottom + Height ) );
  r.x2 = RightSet ? ( ParentSize.x - Right ) : ( Left + Width );
  r.y2 = BottomSet ? ( ParentSize.y - Bottom ) : ( Top + Height );

  if ( !LeftSet && !RightSet )
  {
    r.x1 = ( ParentSize.x - Width ) / 2;
    r.x2 = r.x1 + Width;
  }

  if ( !TopSet && !BottomSet )
  {
    r.y1 = ( ParentSize.y - Height ) / 2;
    r.y2 = r.y1 + Height;
  }

  if ( !( ( LeftSet && RightSet ) || WidthSet ) )
  { //x position not valid
    r.x1 = Original.x1;
    r.x2 = Original.x2;
  }

  if ( !( ( TopSet && BottomSet ) || HeightSet ) )
  { //y position not valid
    r.y1 = Original.y1;
    r.y2 = Original.y2;
  }

  return r;
}

CRect CWBPositionDescriptor::GetPadding( CSize ParentSize, CRect &BorderSizes )
{
  CRect r( 0, 0, 0, 0 );

  r.x1 = (TS32)Positions[ WB_PADDING_LEFT ].GetValue( (TF32)ParentSize.x, 0 ) + BorderSizes.x1;
  r.y1 = (TS32)Positions[ WB_PADDING_TOP ].GetValue( (TF32)ParentSize.y, 0 ) + BorderSizes.y1;
  r.x2 = ParentSize.x - (TS32)Positions[ WB_PADDING_RIGHT ].GetValue( (TF32)ParentSize.x, 0 ) - BorderSizes.x2;
  r.y2 = ParentSize.y - (TS32)Positions[ WB_PADDING_BOTTOM ].GetValue( (TF32)ParentSize.y, 0 ) - BorderSizes.y2;

  return r;
}

void CWBPositionDescriptor::SetMetric( WBPOSITIONTYPE p, WBMETRICTYPE m, TF32 Value )
{
  Positions[ p ].SetMetric( m, Value );
}

void CWBPositionDescriptor::SetValue( WBPOSITIONTYPE p, TF32 Relative, TF32 Pixels )
{
  Positions[ p ].SetValue( Relative, Pixels );
}

void CWBPositionDescriptor::ClearMetrics( WBPOSITIONTYPE p )
{
  Positions.Delete( p );
}

TBOOL CWBPositionDescriptor::IsWidthSet()
{
  return Positions.HasKey( WB_WIDTH );
}

TBOOL CWBPositionDescriptor::IsHeightSet()
{
  return Positions.HasKey( WB_HEIGHT );
}

TS32 CWBPositionDescriptor::GetWidth( CSize ParentSize, CSize ContentSize )
{
  TBOOL WidthSet = Positions.HasKey( WB_WIDTH );
  if ( WidthSet )	return (TS32)Positions[ WB_WIDTH ].GetValue( (TF32)ParentSize.x, ContentSize.x );
  return 0;
}

TS32 CWBPositionDescriptor::GetHeight( CSize ParentSize, CSize ContentSize )
{
  TBOOL HeightSet = Positions.HasKey( WB_HEIGHT );
  if ( HeightSet )	return (TS32)Positions[ WB_HEIGHT ].GetValue( (TF32)ParentSize.y, ContentSize.y );
  return 0;
}

TBOOL CWBPositionDescriptor::IsAutoResizer()
{
  TBOOL WidthSet = Positions.HasKey( WB_WIDTH );
  if ( WidthSet && Positions[ WB_WIDTH ].IsAutoResizer() ) return true;

  TBOOL HeightSet = Positions.HasKey( WB_HEIGHT );
  if ( HeightSet && Positions[ WB_HEIGHT ].IsAutoResizer() ) return true;

  return false;
}

void CWBPositionDescriptor::SetAutoSize( WBPOSITIONTYPE p )
{
  Positions[ p ].SetAutoSize( true );
}

//////////////////////////////////////////////////////////////////////////
// position descriptor - pixels

CWBPositionDescriptorPixels::CWBPositionDescriptorPixels()
{
  for ( TS32 x = 0; x < 6; x++ )
  {
    Set[ x ] = false;
    Positions[ x ] = 0;
  }
}

void CWBPositionDescriptorPixels::SetValue( WBPOSITIONTYPE p, TS32 Pixels )
{
  if ( (TS32)p<0 || p>WB_HEIGHT ) return;
  Positions[ p ] = Pixels;
  Set[ p ] = true;
}

FORCEINLINE CRect CWBPositionDescriptorPixels::GetPosition( CSize ParentSize )
{
  CRect r( 0, 0, 0, 0 );

  r.x1 = Set[ WB_MARGIN_LEFT ] ? Positions[ WB_MARGIN_LEFT ] : ( ParentSize.x - ( Positions[ WB_MARGIN_RIGHT ] + Positions[ WB_WIDTH ] ) );
  r.y1 = Set[ WB_MARGIN_TOP ] ? Positions[ WB_MARGIN_TOP ] : ( ParentSize.y - ( Positions[ WB_MARGIN_BOTTOM ] + Positions[ WB_HEIGHT ] ) );
  r.x2 = Set[ WB_MARGIN_RIGHT ] ? ( ParentSize.x - Positions[ WB_MARGIN_RIGHT ] ) : ( Positions[ WB_MARGIN_LEFT ] + Positions[ WB_WIDTH ] );
  r.y2 = Set[ WB_MARGIN_BOTTOM ] ? ( ParentSize.y - Positions[ WB_MARGIN_BOTTOM ] ) : ( Positions[ WB_MARGIN_TOP ] + Positions[ WB_HEIGHT ] );

  if ( !Set[ WB_MARGIN_LEFT ] && !Set[ WB_MARGIN_RIGHT ] )
  {
    r.x1 = ( ParentSize.x - Positions[ WB_WIDTH ] ) / 2;
    r.x2 = r.x1 + Positions[ WB_WIDTH ];
  }

  if ( !Set[ WB_MARGIN_TOP ] && !Set[ WB_MARGIN_BOTTOM ] )
  {
    r.y1 = ( ParentSize.y - Positions[ WB_HEIGHT ] ) / 2;
    r.y2 = r.y1 + Positions[ WB_HEIGHT ];
  }

  return r;
}

CWBMosaicImage::CWBMosaicImage()
{
  Tiling[ 0 ] = Tiling[ 1 ] = false;
  Stretching[ 0 ] = Stretching[ 1 ] = false;
  Handle = 0;
  Color = 0xffffffff;
}

void CWBMosaicImage::SetPositionValue( WBPOSITIONTYPE p, TS32 Pixels )
{
  Position.SetValue( p, Pixels );
}

void CWBMosaicImage::SetTiling( TS32 Axis, TBOOL y )
{
  Tiling[ Axis ] = y;
}

void CWBMosaicImage::SetStretching( TS32 Axis, TBOOL y )
{
  Stretching[ Axis ] = y;
}

void CWBMosaicImage::SetHandle( WBATLASHANDLE handle )
{
  Handle = handle;
}

FORCEINLINE void CWBMosaicImage::Render( CWBDrawAPI *API, CRect &Pos )
{
  CRect Croprect = API->GetCropRect();
  API->SetCropRect( Pos + API->GetOffset() );

  CRect displaypos = Position.GetPosition( Pos.Size() ) + Pos.TopLeft();

  API->DrawAtlasElement( Handle, displaypos, Tiling[ 0 ], Tiling[ 1 ], Stretching[ 0 ], Stretching[ 1 ], Color );

  API->SetCropRect( Croprect );
}

void CWBMosaicImage::SetColor( CColor color )
{
  Color = color;
}

void CWBMosaic::AddImage( CWBMosaicImage &Image )
{
  Images += Image;
}

void CWBMosaic::Render( CWBDrawAPI *API, CRect &Position )
{
  for ( TS32 x = 0; x < Images.NumItems(); x++ )
    Images[ x ].Render( API, Position + CRect( Overshoot[ 0 ], Overshoot[ 1 ], Overshoot[ 2 ], Overshoot[ 3 ] ) );
}

void CWBMosaic::SetName( CString name )
{
  Name = name;
}

CString & CWBMosaic::GetName()
{
  return Name;
}

CWBMosaic::CWBMosaic( const CWBMosaic &Copy )
{
  Name = Copy.Name;
  Images = Copy.Images;
  for ( TS32 x = 0; x < 4; x++ )
    Overshoot[ x ] = Copy.Overshoot[ x ];
}

CWBMosaic::CWBMosaic()
{
  for ( TS32 x = 0; x < 4; x++ )
    Overshoot[ x ] = 0;
}

CWBMosaic & CWBMosaic::operator=( const CWBMosaic &Copy )
{
  if ( &Copy == this ) return *this;
  Name = Copy.Name;
  Images = Copy.Images;
  for ( TS32 x = 0; x < 4; x++ )
    Overshoot[ x ] = Copy.Overshoot[ x ];
  return *this;
}

void CWBMosaic::Flush()
{
  Images.Flush();
}

void CWBMosaic::SetOverShoot( WBRECTSIDE side, TS32 val )
{
  Overshoot[ side ] = val;
}

void CWBSkinElement::SetName( CString name )
{
  Name = name;
}

CString & CWBSkinElement::GetName()
{
  return Name;
}

FORCEINLINE void CWBSkinElement::Render( CWBDrawAPI *API, CRect &Pos )
{
  API->DrawAtlasElement( Handle, Pos,
                         DefaultBehavior[ 0 ] == WB_SKINBEHAVIOR_TILE,
                         DefaultBehavior[ 1 ] == WB_SKINBEHAVIOR_TILE,
                         DefaultBehavior[ 0 ] == WB_SKINBEHAVIOR_STRETCH,
                         DefaultBehavior[ 1 ] == WB_SKINBEHAVIOR_STRETCH );
}

void CWBSkinElement::SetHandle( WBATLASHANDLE h )
{
  Handle = h;
}

void CWBSkinElement::SetBehavior( TS32 Axis, WBSKINELEMENTBEHAVIOR Behavior )
{
  DefaultBehavior[ Axis ] = Behavior;
}

WBSKINELEMENTBEHAVIOR CWBSkinElement::GetBehavior( TS32 Axis )
{
  return DefaultBehavior[ Axis ];
}

CWBSkinElement & CWBSkinElement::operator=( const CWBSkinElement &Copy )
{
  if ( &Copy == this ) return *this;
  Name = Copy.Name;
  Handle = Copy.Handle;
  DefaultBehavior[ 0 ] = Copy.DefaultBehavior[ 0 ];
  DefaultBehavior[ 1 ] = Copy.DefaultBehavior[ 1 ];
  return *this;
}

CWBSkinElement::CWBSkinElement( const CWBSkinElement &Copy )
{
  Name = Copy.Name;
  Handle = Copy.Handle;
  DefaultBehavior[ 0 ] = Copy.DefaultBehavior[ 0 ];
  DefaultBehavior[ 1 ] = Copy.DefaultBehavior[ 1 ];
}

CWBSkinElement::CWBSkinElement()
{
  Handle = 0;
  DefaultBehavior[ 0 ] = WB_SKINBEHAVIOR_PIXELCORRECT;
  DefaultBehavior[ 1 ] = WB_SKINBEHAVIOR_PIXELCORRECT;
}

WBATLASHANDLE CWBSkinElement::GetHandle()
{
  return Handle;
}

CSize CWBSkinElement::GetElementSize( CWBDrawAPI *API )
{
  return API->GetAtlasElementSize( Handle );
}

void CWBSkin::RenderElement( CWBDrawAPI *API, WBSKINELEMENTID ID, CRect &Pos )
{
  if ( ID == 0xffffffff ) return;

  TS32 idx = ID & 0x7fffffff;

  if ( !( ID & 0x80000000 ) )
  {
    if ( idx >= 0 && idx < Mosaics.NumItems() )
      Mosaics[ idx ].Render( API, Pos );
    return;
  }

  if ( idx >= 0 && idx < SkinItems.NumItems() )
    SkinItems[ idx ].Render( API, Pos );
}

void CWBSkin::RenderElement( CWBDrawAPI *API, CString &Name, CRect &Pos )
{
  RenderElement( API, GetElementID( Name ), Pos );
}

void CWBSkin::RenderElement( CWBDrawAPI *API, TCHAR *Name, CRect &Pos )
{
  RenderElement( API, CString( Name ), Pos );
}

WBSKINELEMENTID CWBSkin::GetElementID( CString &Name )
{
  for ( TS32 x = 0; x < Mosaics.NumItems(); x++ )
    if ( Mosaics[ x ].GetName() == Name )
      return x;

  for ( TS32 x = 0; x < SkinItems.NumItems(); x++ )
    if ( SkinItems[ x ].GetName() == Name )
      return x | 0x80000000;

  return 0xffffffff;
}

void CWBSkin::AddElement( const CString &Name, WBATLASHANDLE Handle, WBSKINELEMENTBEHAVIOR Xbehav, WBSKINELEMENTBEHAVIOR Ybehav )
{
  for ( TS32 x = 0; x < SkinItems.NumItems(); x++ )
    if ( SkinItems[ x ].GetName() == Name )
    {
      SkinItems[ x ].SetHandle( Handle );
      SkinItems[ x ].SetBehavior( 0, Xbehav );
      SkinItems[ x ].SetBehavior( 1, Ybehav );
      return;
    }

  CWBSkinElement e;
  SkinItems += e;
  SkinItems.Last().SetName( Name );
  SkinItems.Last().SetHandle( Handle );
  SkinItems.Last().SetBehavior( 0, Xbehav );
  SkinItems.Last().SetBehavior( 1, Ybehav );
}

CWBMosaic * CWBSkin::AddMosaic( const CString &Name, CString &Description, TS32 OverShootLeft, TS32 OverShootTop, TS32 OverShootRight, TS32 OverShootBottom )
{
  for ( TS32 x = 0; x < Mosaics.NumItems(); x++ )
    if ( Mosaics[ x ].GetName() == Name )
      return &Mosaics[ x ];

  CWBMosaic m;
  Mosaics += m;
  Mosaics.Last().SetName( Name );

  Mosaics.Last().SetOverShoot( WB_RECTSIDE_LEFT, OverShootLeft );
  Mosaics.Last().SetOverShoot( WB_RECTSIDE_TOP, OverShootTop );
  Mosaics.Last().SetOverShoot( WB_RECTSIDE_RIGHT, OverShootRight );
  Mosaics.Last().SetOverShoot( WB_RECTSIDE_BOTTOM, OverShootBottom );

  CStringArray Lines = Description.Explode( _T( ")" ) );
  for ( TS32 x = 0; x < Lines.NumItems(); x++ )
  {
    CStringArray Data = Lines[ x ].Trimmed().Explode( _T( "(" ) );
    CWBMosaicImage i;

    if ( Data.NumItems() == 2 )
    {
      CWBSkinElement *e = GetElement( Data[ 0 ].Trimmed() );
      if ( e )
      {
        i.SetHandle( e->GetHandle() );
        i.SetTiling( 0, e->GetBehavior( 0 ) == WB_SKINBEHAVIOR_TILE );
        i.SetTiling( 1, e->GetBehavior( 1 ) == WB_SKINBEHAVIOR_TILE );
        i.SetStretching( 0, e->GetBehavior( 0 ) == WB_SKINBEHAVIOR_STRETCH );
        i.SetStretching( 1, e->GetBehavior( 1 ) == WB_SKINBEHAVIOR_STRETCH );

        CStringArray Data2 = Data[ 1 ].Explode( _T( ";" ) );

        for ( TS32 y = 0; y < Data2.NumItems(); y++ )
        {
          CStringArray keyvalue = Data2[ y ].Explode( _T( ":" ) );
          if ( keyvalue.NumItems() > 0 )
          {
            CString key = keyvalue[ 0 ].Trimmed();
            CString value;

            if ( keyvalue.NumItems() > 1 ) value = keyvalue[ 1 ].Trimmed();

            TS32 v = 0;
            if ( key == _T( "top" ) )
            {
              value.Scan( _T( "%d" ), &v );
              i.SetPositionValue( WB_MARGIN_TOP, v );
            }
            if ( key == _T( "left" ) )
            {
              value.Scan( _T( "%d" ), &v );
              i.SetPositionValue( WB_MARGIN_LEFT, v );
            }
            if ( key == _T( "right" ) )
            {
              value.Scan( _T( "%d" ), &v );
              i.SetPositionValue( WB_MARGIN_RIGHT, v );
            }
            if ( key == _T( "bottom" ) )
            {
              value.Scan( _T( "%d" ), &v );
              i.SetPositionValue( WB_MARGIN_BOTTOM, v );
            }
            if ( key == _T( "width" ) )
            {
              value.Scan( _T( "%d" ), &v );
              i.SetPositionValue( WB_WIDTH, v );
            }
            if ( key == _T( "height" ) )
            {
              value.Scan( _T( "%d" ), &v );
              i.SetPositionValue( WB_HEIGHT, v );
            }

            if ( key == _T( "repeat-x" ) ) i.SetTiling( 0, true );
            if ( key == _T( "repeat-y" ) ) i.SetTiling( 1, true );
            if ( key == _T( "stretch-x" ) ) i.SetStretching( 0, true );
            if ( key == _T( "stretch-y" ) ) i.SetStretching( 1, true );
            if ( key == _T( "color" ) )
            {
              value.Scan( _T( "%x" ), &v );
              if ( value.Length() <= 6 ) v = v | 0xff000000;
              i.SetColor( v );
            }
          }
        }
        Mosaics.Last().AddImage( i );
      }
    }
  }

  return &Mosaics.Last();
}

CWBSkinElement * CWBSkin::GetElement( CString &Name )
{
  for ( TS32 x = 0; x < SkinItems.NumItems(); x++ )
    if ( SkinItems[ x ].GetName() == Name )
      return &SkinItems[ x ];

  return NULL;
}

CWBSkinElement * CWBSkin::GetElement( WBSKINELEMENTID id )
{
  if ( id == 0xffffffff ) return NULL;
  if ( !( id & 0x80000000 ) ) return NULL; //mosaic
  TS32 idx = id & 0x7fffffff;
  if ( idx < 0 || idx >= SkinItems.NumItems() ) return NULL;
  return &SkinItems[ idx ];
}

CSize CWBSkin::GetElementSize( CWBDrawAPI *API, WBSKINELEMENTID ID )
{
  if ( ID == 0xffffffff ) return CSize( 0, 0 );

  TS32 idx = ID & 0x7fffffff;

  if ( !( ID & 0x80000000 ) ) return CSize( 0, 0 ); //mosaics don't have sizes

  if ( idx >= 0 && idx < SkinItems.NumItems() )
    return SkinItems[ idx ].GetElementSize( API );

  return CSize( 0, 0 );
}
