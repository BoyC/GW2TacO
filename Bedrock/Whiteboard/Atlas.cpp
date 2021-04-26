#include "BasePCH.h"
#include "Atlas.h"

static WBATLASHANDLE AtlasHandle = 1;

CAtlasNode::CAtlasNode()
{
  Children[ 0 ] = NULL;
  Children[ 1 ] = NULL;
  Occupied = false;
  Image = NULL;
}

CAtlasNode::~CAtlasNode()
{
  if ( Children[ 0 ] ) SAFEDELETE( Children[ 0 ] );
  if ( Children[ 1 ] ) SAFEDELETE( Children[ 1 ] );
}

CRect &CAtlasNode::GetArea()
{
  return Area;
}

CAtlasNode *CAtlasNode::AddNode( TS32 width, TS32 height )
{
  CAtlasNode *NewNode;
  if ( Children[ 0 ] )
  {
    NewNode = Children[ 0 ]->AddNode( width, height );
    return NewNode ? NewNode : Children[ 1 ]->AddNode( width, height );
  }

  if ( Occupied || Area.Width() < width || Area.Height() < height ) return 0;

  if ( Area.Width() == width && Area.Height() == height )
  {
    Occupied = true;
    return this;
  }

  Children[ 0 ] = new CAtlasNode();
  Children[ 1 ] = new CAtlasNode();

  if ( Area.Width() - width > Area.Height() - height )
  {
    Children[ 0 ]->Area = CRect( Area.x1, Area.y1, Area.x1 + width, Area.y2 );
    Children[ 1 ]->Area = CRect( Area.x1 + width, Area.y1, Area.x2, Area.y2 );
  }
  else
  {
    Children[ 0 ]->Area = CRect( Area.x1, Area.y1, Area.x2, Area.y1 + height );
    Children[ 1 ]->Area = CRect( Area.x1, Area.y1 + height, Area.x2, Area.y2 );
  }

  return Children[ 0 ]->AddNode( width, height );
}

CAtlasImage * CAtlasNode::GetImage()
{
  return Image;
}

CAtlasImage::CAtlasImage()
{
  Image = NULL;
  XRes = YRes = 0;
  Handle = AtlasHandle++;
  Required = false;
}

CAtlasImage::CAtlasImage( TU8 *SourceImage, TS32 SrcXRes, TS32 SrcYRes, CRect &Source )
{
  Image = NULL;
  XRes = Source.Width();
  YRes = Source.Height();
  Handle = AtlasHandle++;
  Required = false;

  if ( Source.Area() > 0 )
  {
    Image = new TU8[ XRes*YRes * 4 ];
    memset( Image, 0, XRes*YRes * 4 );

    TU8 *i = Image;

    for ( TS32 y = 0; y < YRes; y++ )
    {
      if ( y + Source.y1 < 0 || y + Source.y1 >= SrcYRes )
      {
        LOG( LOG_ERROR, _T( "[gui] Atlas source image out of bounds, failed to add image" ) );
        return;
      }

      for ( TS32 x = 0; x < XRes; x++ )
      {
        if ( x + Source.x1 < 0 || x + Source.x1 >= SrcXRes )
        {
          LOG( LOG_ERROR, _T( "[gui] Atlas source image out of bounds, failed to add image" ) );
          return;
        }

        TS32 k = ( x + Source.x1 + ( y + Source.y1 )*SrcXRes ) * 4;
        i[ 0 ] = SourceImage[ k + 0 ];
        i[ 1 ] = SourceImage[ k + 1 ];
        i[ 2 ] = SourceImage[ k + 2 ];
        i[ 3 ] = SourceImage[ k + 3 ];
        i += 4;
      }
    }
  }
}

CAtlasImage::~CAtlasImage()
{
  SAFEDELETEA( Image );
}

WBATLASHANDLE CAtlasImage::GetHandle()
{
  return Handle;
}

TU8 *CAtlasImage::GetImage()
{
  return Image;
}

CSize CAtlasImage::GetSize() const
{
  return CSize( XRes, YRes );
}

void CAtlasImage::TagRequired()
{
  Required = true;
}

void CAtlasImage::ClearRequired()
{
  Required = false;
}

TBOOL CAtlasImage::IsRequired()
{
  return Required;
}

CAtlas::CAtlas( TS32 XSize, TS32 YSize )
{
  FlushCache();
  XRes = XSize;
  YRes = YSize;
  Image = new TU8[ XRes*YRes * 4 ];
  memset( Image, 0, XRes*YRes * 4 );
  Root = new CAtlasNode();
  Root->Area = CRect( 0, 0, XRes, YRes );
  Root->Occupied = false;
  Atlas = NULL;
  TextureUpdateNeeded = false;

  TS32 White[ 4 ];
  memset( White, 0xff, 4 * 4 );

  {
    CLightweightCriticalSection cs( &critsec );
    CAtlasImage *img = new CAtlasImage( (TU8*)&White, 2, 2, CRect( 0, 0, 2, 2 ) );
    WhitePixel = ImageStorage[ img->GetHandle() ] = img;
  }

  CRect r;
  RequestImageUse( WhitePixel->GetHandle(), r );
  WhitePixelPosition = r.TopLeft();
}

CAtlas::~CAtlas()
{
  {
    CLightweightCriticalSection cs( &critsec );
    for ( int x = 0; x < ImageStorage.NumItems(); x++ )
      delete ImageStorage.GetByIndex( x );
  }
  SAFEDELETE( Root );
  SAFEDELETEA( Image );
  SAFEDELETE( Atlas );
}

TBOOL CAtlas::PackImage( CAtlasImage *img )
{
  if ( !img ) return false;

  //LOG(LOG_DEBUG,_T("Packing Image %d"),img->GetHandle());

  FlushCache();
  CSize s = img->GetSize();

  CAtlasNode *n = Root->AddNode( s.x, s.y );
  if ( !n )
  {
    LOG( LOG_WARNING, _T( "[gui] Atlas full. Image can't be added." ) );
    return 0;
  }

  TU8 *target = Image + ( n->Area.x1 + n->Area.y1*XRes ) * 4;
  TU8 *source = img->GetImage();

  for ( TS32 y = 0; y < s.y; y++ )
  {
    memcpy( target, source, img->GetSize().x * 4 );
    target += XRes * 4;
    source += img->GetSize().x * 4;
  }

  WBATLASHANDLE Handle = img->GetHandle();

  n->Image = img;
  Dictionary[ Handle ] = n;
  img->TagRequired();

  TextureUpdateNeeded = true;

  return img->GetHandle() != 0;
}

TBOOL CAtlas::InitializeTexture( CCoreDevice *Device )
{
  if ( !Device ) return false;
  if ( Atlas ) SAFEDELETE( Atlas );
  return ( Atlas = Device->CreateTexture2D( XRes, YRes, Image ) ) != NULL;
}

WBATLASHANDLE CAtlas::AddImage( TU8 *i, TS32 xs, TS32 ys, CRect &a )
{
  if ( a.Width() == 0 || a.Height() == 0 ) return 0;

  CLightweightCriticalSection cs( &critsec );

  CAtlasImage *img = new CAtlasImage( i, xs, ys, a );
  ImageStorage[ img->GetHandle() ] = img;
  return img->GetHandle();
}

TBOOL CAtlas::UpdateTexture()
{
  //LOG_DBG("Atlas Texture Update Request");

  if ( !TextureUpdateNeeded ) return true;

  //LOG(LOG_DEBUG,_T("Updating Atlas Texture"));

  if ( !Atlas || !Atlas->Update( Image, XRes, YRes, 4 ) ) return false;

  TextureUpdateNeeded = false;
  return true;
}

int SortImageStorage( CAtlasImage * const &a, CAtlasImage * const &b )
{
  CSize ra = a->GetSize();
  CSize rb = b->GetSize();

  TS32 w = rb.x - ra.x;
  TS32 h = rb.y - ra.y;

  if ( w != 0 ) return w;
  return h;
}

TBOOL CAtlas::Optimize( TBOOL DebugMode )
{
  //rearranges the atlas in a more optimal fashion and removes unused

  LOG( LOG_DEBUG, _T( "[gui] Optimizing Atlas" ) );

  memset( Image, 0, XRes*YRes * 4 );

  if ( DebugMode )
  {
    CLightweightCriticalSection cs( &critsec );
    for ( TS32 x = 0; x < ImageStorage.NumItems(); x++ )
    {
      if ( Dictionary.HasKey( ImageStorage.GetByIndex( x )->GetHandle() ) )
        ImageStorage.GetByIndex( x )->TagRequired();
    }
  }

  SAFEDELETE( Root );
  Root = new CAtlasNode();
  Root->Area = CRect( 0, 0, XRes, YRes );
  Root->Occupied = false;

  Dictionary.Flush();
  FlushCache();

  {
    CLightweightCriticalSection cs( &critsec );
    TS32 RequiredCount = 0;
    for ( TS32 x = 0; x < ImageStorage.NumItems(); x++ )
      RequiredCount += ImageStorage.GetByIndex( x )->IsRequired();

    WhitePixel->TagRequired();

    if ( RequiredCount )
    {
      ImageStorage.SortByValue( SortImageStorage );
      for ( TS32 x = 0; x < ImageStorage.NumItems(); x++ )
      {
        if ( ImageStorage.GetByIndex( x )->IsRequired() )
          if ( !PackImage( ImageStorage.GetByIndex( x ) ) )
            return false;
      }
    }
    else
    {
      if ( !PackImage( WhitePixel ) )
        return false;
    }
  }

  CRect r;
  RequestImageUse( WhitePixel->GetHandle(), r );
  WhitePixelPosition = r.TopLeft() + CPoint( 1, 1 );

  TextureUpdateNeeded = true;
  return true;
}

void CAtlas::DeleteImage( const WBATLASHANDLE h )
{
  {
    CLightweightCriticalSection cs( &critsec );
    if ( ImageStorage.HasKey( h ) )
    {
      SAFEDELETE( ImageStorage[ h ] );
      ImageStorage.Delete( h );
    }
  }

  CAtlasNode *n = GetNodeCached( h );
  if ( !n ) return;
  n->Image = NULL;
  Dictionary.Delete( h );
  FlushCache();
  return;
}

CCoreTexture2D *CAtlas::GetTexture()
{
  return Atlas;
}

CSize CAtlas::GetSize( WBATLASHANDLE h )
{
  CAtlasNode *n = GetNodeCached( h );
  if ( n ) return n->GetArea().Size();

  {
    CLightweightCriticalSection cs( &critsec );
    if ( ImageStorage.HasKey( h ) )
      return ImageStorage[ h ]->GetSize();
  }

  return CSize( 0, 0 );
}

TBOOL CAtlas::RequestImageUse( WBATLASHANDLE h, CRect &r )
{
  if ( !h )
  {
    r = CRect( 0, 0, 0, 0 );
    return true;
  }

  CAtlasNode *n = GetNodeCached( h );

  if ( !n ) //image not on atlas, add it
  {
    {
      CLightweightCriticalSection cs( &critsec );
      if ( ImageStorage.HasKey( h ) )
      {
        if ( PackImage( ImageStorage[ h ] ) )
          n = GetNodeCached( h );
      }
    }

    if ( !n )
    {
      r = CRect( 0, 0, 0, 0 );
      return false;
    }
  }

  //need to tag image as required HERE
  if ( n->GetImage() ) n->GetImage()->TagRequired();

  r = n->Area;

  return true;
}

CPoint CAtlas::GetWhitePixelUV()
{
  return WhitePixelPosition;
}

void CAtlas::ClearImageUsageflags()
{
  CLightweightCriticalSection cs( &critsec );
  for ( TS32 x = 0; x < ImageStorage.NumItems(); x++ )
    ImageStorage.GetByIndex( x )->ClearRequired();
}

void CAtlas::FlushCache()
{
  memset( AtlasCache, 0, sizeof( AtlasCache ) );
}

CAtlasNode * CAtlas::GetNodeCached( WBATLASHANDLE Handle )
{
  TS32 idx = Handle&( ATLASCACHESIZE - 1 );

  if ( AtlasCache[ idx ].Handle == Handle ) return AtlasCache[ idx ].Node;

  CAtlasNode *n = Dictionary.GetExisting( Handle );

  AtlasCache[ idx ].Handle = Handle;
  AtlasCache[ idx ].Node = n;

  return n;
}

TBOOL CAtlas::Reset()
{
  SAFEDELETE( Root );
  Root = new CAtlasNode();
  Root->Area = CRect( 0, 0, XRes, YRes );
  Root->Occupied = false;

  Dictionary.Flush();
  FlushCache();
  WhitePixel->TagRequired();

  if ( !PackImage( WhitePixel ) )
    return false;

  CRect r;
  RequestImageUse( WhitePixel->GetHandle(), r );
  WhitePixelPosition = r.TopLeft() + CPoint( 1, 1 );

  TextureUpdateNeeded = true;
  return true;
}

TBOOL CAtlas::Resize( CCoreDevice *Device, TS32 XSize, TS32 YSize )
{
  SAFEDELETE( Root );
  SAFEDELETEA( Image );
  SAFEDELETE( Atlas );

  FlushCache();
  XRes = XSize;
  YRes = YSize;

  Image = new TU8[ XRes*YRes * 4 ];
  memset( Image, 0, XRes*YRes * 4 );

  Root = new CAtlasNode();
  Root->Area = CRect( 0, 0, XRes, YRes );
  Root->Occupied = false;

  Dictionary.Flush();
  WhitePixel->TagRequired();

  if ( !InitializeTexture( Device ) )
    return false;

  if ( !PackImage( WhitePixel ) )
    return false;

  CRect r;
  RequestImageUse( WhitePixel->GetHandle(), r );
  WhitePixelPosition = r.TopLeft() + CPoint( 1, 1 );

  return true;
}
