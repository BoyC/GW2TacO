#include "BasePCH.h"
#include "DX11Texture.h"
#include "../UtilLib/PNGDecompressor.h"
#include "DDSTextureLoader.h"

#ifdef CORE_API_DX11

CCoreDX11Texture2D::CCoreDX11Texture2D( CCoreDX11Device *dev ) : CCoreTexture2D( dev )
{
  Dev = dev->GetDevice();
  DeviceContext = dev->GetDeviceContext();
  TextureHandle = NULL;
  RenderTarget = false;
  View = NULL;
  RTView = NULL;
  DepthView = NULL;
};

CCoreDX11Texture2D::~CCoreDX11Texture2D()
{
  Release();
}

void CCoreDX11Texture2D::Release()
{
  if ( TextureHandle ) TextureHandle->Release();
  TextureHandle = NULL;
  if ( View ) View->Release();
  if ( DepthView ) DepthView->Release();
  if ( RTView ) RTView->Release();
  View = NULL;
  RTView = NULL;
  DepthView = NULL;
}

TBOOL CCoreDX11Texture2D::SetToSampler( const CORESAMPLER smp )
{
  if ( smp >= CORESMP_PS0 && smp <= CORESMP_PS15 )
    DeviceContext->PSSetShaderResources( smp, 1, &View );
  if ( smp >= CORESMP_VS0 && smp <= CORESMP_VS3 )
    DeviceContext->VSSetShaderResources( smp - CORESMP_VS0, 1, &View );
  if ( smp >= CORESMP_GS0 && smp <= CORESMP_GS3 )
    DeviceContext->GSSetShaderResources( smp - CORESMP_GS0, 1, &View );

  return true;
}

TBOOL CCoreDX11Texture2D::Create( const TS32 xres, const TS32 yres, const TU8 *Data, const TS8 BytesPerPixel, const COREFORMAT format, const TBOOL rendertarget )
{
  if ( xres <= 0 || yres <= 0 || format == COREFMT_UNKNOWN ) return false;
  Release();

  D3D11_TEXTURE2D_DESC tex;
  memset( &tex, 0, sizeof( D3D11_TEXTURE2D_DESC ) );
  tex.ArraySize = 1;
  tex.Width = xres;
  tex.Height = yres;
  //tex.MipLevels = 1;// rendertarget ? 0 : 1;
  tex.MipLevels = rendertarget ? 0 : 1;
  tex.MiscFlags = rendertarget ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
  tex.Format = DX11Formats[ format ];
  tex.SampleDesc.Count = 1;
  tex.SampleDesc.Quality = 0;
  tex.BindFlags = D3D11_BIND_SHADER_RESOURCE | ( rendertarget ? D3D11_BIND_RENDER_TARGET : 0 );
  //if ( rendertarget )
  //  tex.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = Data;
  data.SysMemPitch = xres*BytesPerPixel;
  data.SysMemSlicePitch = 0;

  HRESULT res = Dev->CreateTexture2D( &tex, Data ? &data : NULL, &TextureHandle );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] CreateTexture2D failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  res = Dev->CreateShaderResourceView( TextureHandle, NULL, &View );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] CreateShaderResourceView failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  if ( rendertarget )
  {
    D3D11_RENDER_TARGET_VIEW_DESC rt;
    rt.Format = DX11Formats[ Format ];
    rt.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
    rt.Texture2D.MipSlice = 0;
    res = Dev->CreateRenderTargetView( TextureHandle, &rt, &RTView );
    if ( res != S_OK )
    {
      _com_error err( res );
      LOG( LOG_ERROR, _T( "[core] Failed to rendertarget view (%s)" ), err.ErrorMessage() );
      return false;
    }
  }

  XRes = xres;
  YRes = yres;
  Format = format;

  return true;
}

TBOOL CCoreDX11Texture2D::Create( const TU8 *Data, const TS32 Size )
{
  TBOOL ViewCreated = false;

  if ( !Data || Size <= 0 ) return false;
  Release();

#ifdef CORE_API_D3DX
  ID3D11Resource* r = NULL;
  if ( D3DX11CreateTextureFromMemory( Dev, Data, Size, NULL, NULL, &r, NULL ) != S_OK )
  {
    LOG( LOG_ERROR, _T( "[core] Texture Creation From Image failed" ) );
    return false;
  }

  TextureHandle = (ID3D11Texture2D*)r;
#else

  TS32 xr, yr;
  TU8 *Img = DecompressImage( Data, Size, xr, yr );

  if ( !Img )
  {
    if ( !DecompressPNG( Data, Size, Img, xr, yr ) )
    {
      if ( !CreateDDSTextureFromMemory( Dev, Data, Size, (ID3D11Resource**)&TextureHandle, &View ) )
      {

#ifdef CORE_VERBOSE_LOG
        LOG( LOG_ERROR, _T( "Texture Creation From Image failed: d3dx not linked" ) );
#endif			
        return false;
      }
      else ViewCreated = true;
    }
    else
    {
      if ( !Img ) return false;
      ARGBtoABGR( Img, xr, yr );
    }
  }

  if ( !ViewCreated )
  {
    Create( xr, yr, Img );
    SAFEDELETE( Img );
  }

#endif

  if ( !ViewCreated )
  {
    HRESULT res = Dev->CreateShaderResourceView( TextureHandle, NULL, &View );
    if ( res != S_OK )
    {
      _com_error err( res );
      LOG( LOG_ERROR, _T( "[core] CreateShaderResourceView failed (%s)" ), err.ErrorMessage() );
      return false;
    }
  }

  D3D11_TEXTURE2D_DESC texturedesc;
  TextureHandle->GetDesc( &texturedesc );

  XRes = texturedesc.Width;
  YRes = texturedesc.Height;
  Format = GetFormat( texturedesc.Format );
  RenderTarget = false;

  return true;
}

TBOOL CCoreDX11Texture2D::Lock( void **Result, TS32 &pitch )
{
  //if (!TextureHandle) return false;

  //D3DLOCKED_RECT Rect;

  //if (TextureHandle->LockRect(0,&Rect,NULL,D3DLOCK_DISCARD)!=D3D_OK)
  //	return false;
  //
  //*Result=Rect.pBits;
  //pitch=Rect.Pitch;
  return false;
}

TBOOL CCoreDX11Texture2D::UnLock()
{
  //if (!TextureHandle) return false;

  //return TextureHandle->UnlockRect(0)==D3D_OK;
  return true;
}

void CCoreDX11Texture2D::OnDeviceLost()
{
  if ( RenderTarget ) Release();
}

void CCoreDX11Texture2D::OnDeviceReset()
{
  if ( RenderTarget && XRes > 0 && YRes > 0 && Format != COREFMT_UNKNOWN )
    BASEASSERT( Create( XRes, YRes, NULL, 4, Format, RenderTarget ) );
}

TBOOL CCoreDX11Texture2D::Update( const TU8 *Data, const TS32 XRes, const TS32 YRes, const TS8 BytesPerPixel )
{
  if ( !TextureHandle ) return false;
  if ( !View ) return false;

  DeviceContext->UpdateSubresource( TextureHandle, 0, NULL, Data, XRes*BytesPerPixel, 0 );

  return true;
}

CCoreTexture2D * CCoreDX11Texture2D::Copy()
{
  if ( !TextureHandle ) return NULL;
  if ( !View ) return NULL;

  D3D11_TEXTURE2D_DESC desc;
  TextureHandle->GetDesc( &desc );
  desc.Usage = D3D11_USAGE_DEFAULT;

  //D3D11_SUBRESOURCE_DATA data;
  //data.pSysMem=new unsigned char[desc.Width*desc.Height*16];
  //data.SysMemPitch=desc.Width*16;
  //data.SysMemSlicePitch=0;

  ID3D11Texture2D *d2;

  HRESULT res;
  res = Dev->CreateTexture2D( &desc, NULL, &d2 );

  //delete[] data.pSysMem;

  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] Failed to create copy of texture 2d (%s)" ), err.ErrorMessage() );
    return NULL;
  }

  ID3D11ShaderResourceView *v2;

  res = Dev->CreateShaderResourceView( d2, NULL, &v2 );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] CreateShaderResourceView failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  DeviceContext->CopyResource( d2, TextureHandle );

  CCoreDX11Texture2D *nt = new CCoreDX11Texture2D( (CCoreDX11Device*)Device );
  nt->SetTextureHandle( d2 );
  nt->SetView( v2 );
  return nt;
}

#include "../UtilLib/PNGDecompressor.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>

/*
float gamma( float c )
{
  float cs = saturate( c );

  if ( cs < 0.0031308 ) cs = 12.92*cs;
  else cs = 1.055*pow( cs, 1 / 2.4 ) - 0.055;

  return cs;
}

float degamma( float c )
{
  float cs;
  if ( c < 0.04045 ) cs = c / 12.92;
  else cs = pow( ( c + 0.055 ) / 1.055, 2.4 );
  return cs;
}
*/

float degammafloat( float f )
{
  if ( f < 0.0031308f ) return 12.92f*f;
  return 1.055f*powf( f, 1 / 2.4f ) - 0.055f;
}

TU16 degammaint16( TU16 f )
{
  float tf = f / 65535.0f;

  return (TU16)( degammafloat( tf ) * 65535 );
}

void CCoreDX11Texture2D::ExportToImage( CString &Filename, TBOOL ClearAlpha, EXPORTIMAGEFORMAT Format, bool degamma )
{
  if ( !TextureHandle ) return;

  //HRESULT res = D3DX11SaveTextureToFile(DeviceContext, TextureHandle, D3DX11_IFF_PNG, Filename.GetPointer());
  //if (res == S_OK) return;

  CStreamWriterMemory Writer;
  HRESULT res = SaveDDSTexture( DeviceContext, TextureHandle, Writer );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] Failed to export texture to '%s' (%x: %s)" ), Filename.GetPointer(), res, err.ErrorMessage() );
    return;
  }

  struct DDSHEAD
  {
    TU32 DDS;
    TS32 dwSize;
    TS32 dwFlags;
    TS32 dwHeight;
    TS32 dwWidth;
    TS32 dwPitchOrLinearSize;
    TS32 dwDepth;
    TS32 dwMipMapCount;
    TS32 dwReserved1[ 11 ];

    TS32 _dwSize;
    TS32 _dwFlags;
    TS32 dwFourCC;
    TS32 dwRGBBitCount;
    TS32 dwRBitMask;
    TS32 dwGBitMask;
    TS32 dwBBitMask;
    TS32 dwABitMask;

    TS32 dwCaps;
    TS32 dwCaps2;
    TS32 dwCaps3;
    TS32 dwCaps4;
    TS32 dwReserved2;
  };

  struct DDS_HEADER_DXT10
  {
    DXGI_FORMAT              dxgiFormat;
    D3D10_RESOURCE_DIMENSION resourceDimension;
    UINT                     miscFlag;
    UINT                     arraySize;
    UINT                     miscFlags2;
  };

  TU8 *Data = (TU8*)Writer.GetData();
  DDSHEAD head;
  memcpy( &head, Data, sizeof( DDSHEAD ) );
  Data += head.dwSize + 4;

  TU8 *image = new TU8[ head.dwWidth*head.dwHeight * 4 ];

  switch ( head.dwFourCC )
  {
  case 0:
    if ( head._dwFlags == 0x041 )
    {
      //rgba
      memcpy( image, Data, head.dwWidth*head.dwHeight * 4 );

      if ( head.dwRBitMask == 0x00ff0000 && head.dwBBitMask == 0x000000ff )
        for ( TS32 x = 0; x < head.dwWidth*head.dwHeight; x++ )
        {
          TS32 p = x * 4;
          TS32 t = image[ p ];
          image[ p ] = image[ p + 2 ];
          image[ p + 2 ] = t;
        }

      break;
    }
    else
    {
      LOG_ERR( "[core] Failed to export texture: unknown FourCC format (%x)", head.dwFourCC );
      SAFEDELETEA( image );
      return;
    }
    break;
  case 36:
  {
    TU16 *inimg = (TU16*)Data;
    for ( TS32 x = 0; x < head.dwWidth*head.dwHeight * 4; x++ )
      image[ x ] = ( !degamma ? inimg[ x ] : degammaint16( inimg[ x ] ) ) / 256;
  }
  break;
  case 113:
  {
    //D3DXFloat16To32Array()
    float *img2 = new float[ head.dwWidth*head.dwHeight * 4 ];
    DirectX::PackedVector::XMConvertHalfToFloatStream( img2, 4, ( const DirectX::PackedVector::HALF* )Data, 2, head.dwWidth*head.dwHeight * 4 );

    if ( !degamma )
    {
      for ( TS32 x = 0; x < head.dwWidth*head.dwHeight * 4; x++ )
        image[ x ] = (TU8)max( 0, min( 255, img2[ x ] * 255 ) );
    }
    else
    {
      for ( TS32 x = 0; x < head.dwWidth*head.dwHeight * 4; )
      {
        for ( int y = 0; y < 3; y++, x++ )
          image[ x ] = (TU8)max( 0, min( 255, degammafloat( img2[ x ] ) * 255 ) );

        x++;
        image[ x ] = (TU8)max( 0, min( 255, img2[ x ] * 255 ) );
      }
    }

    SAFEDELETEA( img2 );
  }
  break;
  case '01XD': // DX10
  {
    DDS_HEADER_DXT10 *Head = (DDS_HEADER_DXT10*)Data;
    Data += sizeof( DDS_HEADER_DXT10 );
    switch ( Head->dxgiFormat )
    {
    case DXGI_FORMAT_B8G8R8A8_UNORM:
      memcpy( image, Data, head.dwWidth*head.dwHeight * 4 );
      for ( TS32 x = 0; x < head.dwWidth*head.dwHeight; x++ )
      {
        TS32 p = x * 4;
        TS32 t = image[ p ];
        image[ p ] = image[ p + 2 ];
        image[ p + 2 ] = t;
      }
      break;
    default:
      LOG_ERR( "[core] Failed to export texture: unknown DXGI format (%d)", Head->dxgiFormat );
      SAFEDELETEA( image );
      break;
    }
    break;
  }
  default:
    LOG_ERR( "[core] Failed to export texture: unknown FourCC format (%x)", head.dwFourCC );
    SAFEDELETEA( image );
    return;
    break;
  }

  switch ( Format )
  {
  case CORE_PNG:
    ExportPNG( image, head.dwWidth, head.dwHeight, ClearAlpha, Filename );
    break;
  case CORE_TGA:
    ExportTga( image, head.dwWidth, head.dwHeight, ClearAlpha, Filename );
    break;
  case CORE_BMP:
    ExportBmp( image, head.dwWidth, head.dwHeight, Filename );
    break;
  default:
    break;
  }

  SAFEDELETEA( image );
}

TBOOL CCoreDX11Texture2D::CreateDepthBuffer( const TS32 xres, const TS32 yres, const TS32 MSCount )
{
  if ( xres <= 0 || yres <= 0 ) return false;
  Release();

  D3D11_TEXTURE2D_DESC tex;
  memset( &tex, 0, sizeof( D3D11_TEXTURE2D_DESC ) );
  tex.ArraySize = 1;
  tex.Width = xres;
  tex.Height = yres;
  tex.MipLevels = 1;
  tex.Format = DXGI_FORMAT_R24G8_TYPELESS;
  tex.SampleDesc.Count = MSCount;
  tex.SampleDesc.Quality = MSCount > 1 ? D3D10_STANDARD_MULTISAMPLE_PATTERN : 0;
  tex.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

  HRESULT res = Dev->CreateTexture2D( &tex, NULL, &TextureHandle );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] CreateTexture2D failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC deptdesc;

  memset( &deptdesc, 0, sizeof( deptdesc ) );
  deptdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  deptdesc.ViewDimension = MSCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
  deptdesc.Texture2D.MipSlice = 0;

  // Create the depth stencil view.
  res = Dev->CreateDepthStencilView( TextureHandle, &deptdesc, &DepthView );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] CreateDepthStencilView failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC resdesc;
  memset( &resdesc, 0, sizeof( resdesc ) );
  resdesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  resdesc.ViewDimension = MSCount > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
  resdesc.Texture2D.MipLevels = 1;

  res = Dev->CreateShaderResourceView( TextureHandle, &resdesc, &View );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[core] CreateShaderResourceView failed (%s)" ), err.ErrorMessage() );
    return false;
  }

  XRes = xres;
  YRes = yres;
  Format = COREFMT_UNKNOWN;

  return true;
}


CCoreDX11Texture3D::CCoreDX11Texture3D( CCoreDX11Device *dev ) : CCoreTexture3D( dev )
{
  //Dev=dev->GetDevice();
};


CCoreDX11TextureCube::CCoreDX11TextureCube( CCoreDX11Device *dev ) : CCoreTextureCube( dev )
{
  //Dev=dev->GetDevice();
};

static DXGI_FORMAT EnsureNotTypeless( DXGI_FORMAT fmt )
{
  // Assumes UNORM or FLOAT; doesn't use UINT or SINT
  switch ( fmt )
  {
  case DXGI_FORMAT_R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_FLOAT;
  case DXGI_FORMAT_R32G32B32_TYPELESS:    return DXGI_FORMAT_R32G32B32_FLOAT;
  case DXGI_FORMAT_R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_UNORM;
  case DXGI_FORMAT_R32G32_TYPELESS:       return DXGI_FORMAT_R32G32_FLOAT;
  case DXGI_FORMAT_R10G10B10A2_TYPELESS:  return DXGI_FORMAT_R10G10B10A2_UNORM;
  case DXGI_FORMAT_R8G8B8A8_TYPELESS:     return DXGI_FORMAT_R8G8B8A8_UNORM;
  case DXGI_FORMAT_R16G16_TYPELESS:       return DXGI_FORMAT_R16G16_UNORM;
  case DXGI_FORMAT_R32_TYPELESS:          return DXGI_FORMAT_R32_FLOAT;
  case DXGI_FORMAT_R8G8_TYPELESS:         return DXGI_FORMAT_R8G8_UNORM;
  case DXGI_FORMAT_R16_TYPELESS:          return DXGI_FORMAT_R16_UNORM;
  case DXGI_FORMAT_R8_TYPELESS:           return DXGI_FORMAT_R8_UNORM;
  case DXGI_FORMAT_BC1_TYPELESS:          return DXGI_FORMAT_BC1_UNORM;
  case DXGI_FORMAT_BC2_TYPELESS:          return DXGI_FORMAT_BC2_UNORM;
  case DXGI_FORMAT_BC3_TYPELESS:          return DXGI_FORMAT_BC3_UNORM;
  case DXGI_FORMAT_BC4_TYPELESS:          return DXGI_FORMAT_BC4_UNORM;
  case DXGI_FORMAT_BC5_TYPELESS:          return DXGI_FORMAT_BC5_UNORM;
  case DXGI_FORMAT_B8G8R8A8_TYPELESS:     return DXGI_FORMAT_B8G8R8A8_UNORM;
  case DXGI_FORMAT_B8G8R8X8_TYPELESS:     return DXGI_FORMAT_B8G8R8X8_UNORM;
  case DXGI_FORMAT_BC7_TYPELESS:          return DXGI_FORMAT_BC7_UNORM;
  default:                                return fmt;
  }
}

static size_t BitsPerPixel( _In_ DXGI_FORMAT fmt )
{
  switch ( fmt )
  {
  case DXGI_FORMAT_R32G32B32A32_TYPELESS:
  case DXGI_FORMAT_R32G32B32A32_FLOAT:
  case DXGI_FORMAT_R32G32B32A32_UINT:
  case DXGI_FORMAT_R32G32B32A32_SINT:
    return 128;

  case DXGI_FORMAT_R32G32B32_TYPELESS:
  case DXGI_FORMAT_R32G32B32_FLOAT:
  case DXGI_FORMAT_R32G32B32_UINT:
  case DXGI_FORMAT_R32G32B32_SINT:
    return 96;

  case DXGI_FORMAT_R16G16B16A16_TYPELESS:
  case DXGI_FORMAT_R16G16B16A16_FLOAT:
  case DXGI_FORMAT_R16G16B16A16_UNORM:
  case DXGI_FORMAT_R16G16B16A16_UINT:
  case DXGI_FORMAT_R16G16B16A16_SNORM:
  case DXGI_FORMAT_R16G16B16A16_SINT:
  case DXGI_FORMAT_R32G32_TYPELESS:
  case DXGI_FORMAT_R32G32_FLOAT:
  case DXGI_FORMAT_R32G32_UINT:
  case DXGI_FORMAT_R32G32_SINT:
  case DXGI_FORMAT_R32G8X24_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
  case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
  case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
  case DXGI_FORMAT_Y416:
  case DXGI_FORMAT_Y210:
  case DXGI_FORMAT_Y216:
    return 64;

  case DXGI_FORMAT_R10G10B10A2_TYPELESS:
  case DXGI_FORMAT_R10G10B10A2_UNORM:
  case DXGI_FORMAT_R10G10B10A2_UINT:
  case DXGI_FORMAT_R11G11B10_FLOAT:
  case DXGI_FORMAT_R8G8B8A8_TYPELESS:
  case DXGI_FORMAT_R8G8B8A8_UNORM:
  case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
  case DXGI_FORMAT_R8G8B8A8_UINT:
  case DXGI_FORMAT_R8G8B8A8_SNORM:
  case DXGI_FORMAT_R8G8B8A8_SINT:
  case DXGI_FORMAT_R16G16_TYPELESS:
  case DXGI_FORMAT_R16G16_FLOAT:
  case DXGI_FORMAT_R16G16_UNORM:
  case DXGI_FORMAT_R16G16_UINT:
  case DXGI_FORMAT_R16G16_SNORM:
  case DXGI_FORMAT_R16G16_SINT:
  case DXGI_FORMAT_R32_TYPELESS:
  case DXGI_FORMAT_D32_FLOAT:
  case DXGI_FORMAT_R32_FLOAT:
  case DXGI_FORMAT_R32_UINT:
  case DXGI_FORMAT_R32_SINT:
  case DXGI_FORMAT_R24G8_TYPELESS:
  case DXGI_FORMAT_D24_UNORM_S8_UINT:
  case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
  case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
  case DXGI_FORMAT_R8G8_B8G8_UNORM:
  case DXGI_FORMAT_G8R8_G8B8_UNORM:
  case DXGI_FORMAT_B8G8R8A8_UNORM:
  case DXGI_FORMAT_B8G8R8X8_UNORM:
  case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
  case DXGI_FORMAT_B8G8R8A8_TYPELESS:
  case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
  case DXGI_FORMAT_B8G8R8X8_TYPELESS:
  case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
  case DXGI_FORMAT_AYUV:
  case DXGI_FORMAT_Y410:
  case DXGI_FORMAT_YUY2:
    return 32;

  case DXGI_FORMAT_P010:
  case DXGI_FORMAT_P016:
    return 24;

  case DXGI_FORMAT_R8G8_TYPELESS:
  case DXGI_FORMAT_R8G8_UNORM:
  case DXGI_FORMAT_R8G8_UINT:
  case DXGI_FORMAT_R8G8_SNORM:
  case DXGI_FORMAT_R8G8_SINT:
  case DXGI_FORMAT_R16_TYPELESS:
  case DXGI_FORMAT_R16_FLOAT:
  case DXGI_FORMAT_D16_UNORM:
  case DXGI_FORMAT_R16_UNORM:
  case DXGI_FORMAT_R16_UINT:
  case DXGI_FORMAT_R16_SNORM:
  case DXGI_FORMAT_R16_SINT:
  case DXGI_FORMAT_B5G6R5_UNORM:
  case DXGI_FORMAT_B5G5R5A1_UNORM:
  case DXGI_FORMAT_A8P8:
  case DXGI_FORMAT_B4G4R4A4_UNORM:
    return 16;

  case DXGI_FORMAT_NV12:
  case DXGI_FORMAT_420_OPAQUE:
  case DXGI_FORMAT_NV11:
    return 12;

  case DXGI_FORMAT_R8_TYPELESS:
  case DXGI_FORMAT_R8_UNORM:
  case DXGI_FORMAT_R8_UINT:
  case DXGI_FORMAT_R8_SNORM:
  case DXGI_FORMAT_R8_SINT:
  case DXGI_FORMAT_A8_UNORM:
  case DXGI_FORMAT_AI44:
  case DXGI_FORMAT_IA44:
  case DXGI_FORMAT_P8:
    return 8;

  case DXGI_FORMAT_R1_UNORM:
    return 1;

  case DXGI_FORMAT_BC1_TYPELESS:
  case DXGI_FORMAT_BC1_UNORM:
  case DXGI_FORMAT_BC1_UNORM_SRGB:
  case DXGI_FORMAT_BC4_TYPELESS:
  case DXGI_FORMAT_BC4_UNORM:
  case DXGI_FORMAT_BC4_SNORM:
    return 4;

  case DXGI_FORMAT_BC2_TYPELESS:
  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC2_UNORM_SRGB:
  case DXGI_FORMAT_BC3_TYPELESS:
  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC3_UNORM_SRGB:
  case DXGI_FORMAT_BC5_TYPELESS:
  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
  case DXGI_FORMAT_BC6H_TYPELESS:
  case DXGI_FORMAT_BC6H_UF16:
  case DXGI_FORMAT_BC6H_SF16:
  case DXGI_FORMAT_BC7_TYPELESS:
  case DXGI_FORMAT_BC7_UNORM:
  case DXGI_FORMAT_BC7_UNORM_SRGB:
    return 8;

#if defined(_XBOX_ONE) && defined(_TITLE)

  case DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
  case DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
    return 32;

  case DXGI_FORMAT_D16_UNORM_S8_UINT:
  case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
    return 24;

#endif // _XBOX_ONE && _TITLE

  default:
    return 0;
  }
}

static void GetSurfaceInfo( _In_ size_t width,
                            _In_ size_t height,
                            _In_ DXGI_FORMAT fmt,
                            _Out_opt_ size_t* outNumBytes,
                            _Out_opt_ size_t* outRowBytes,
                            _Out_opt_ size_t* outNumRows )
{
  size_t numBytes = 0;
  size_t rowBytes = 0;
  size_t numRows = 0;

  bool bc = false;
  bool packed = false;
  bool planar = false;
  size_t bpe = 0;
  switch ( fmt )
  {
  case DXGI_FORMAT_BC1_TYPELESS:
  case DXGI_FORMAT_BC1_UNORM:
  case DXGI_FORMAT_BC1_UNORM_SRGB:
  case DXGI_FORMAT_BC4_TYPELESS:
  case DXGI_FORMAT_BC4_UNORM:
  case DXGI_FORMAT_BC4_SNORM:
    bc = true;
    bpe = 8;
    break;

  case DXGI_FORMAT_BC2_TYPELESS:
  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC2_UNORM_SRGB:
  case DXGI_FORMAT_BC3_TYPELESS:
  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC3_UNORM_SRGB:
  case DXGI_FORMAT_BC5_TYPELESS:
  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
  case DXGI_FORMAT_BC6H_TYPELESS:
  case DXGI_FORMAT_BC6H_UF16:
  case DXGI_FORMAT_BC6H_SF16:
  case DXGI_FORMAT_BC7_TYPELESS:
  case DXGI_FORMAT_BC7_UNORM:
  case DXGI_FORMAT_BC7_UNORM_SRGB:
    bc = true;
    bpe = 16;
    break;

  case DXGI_FORMAT_R8G8_B8G8_UNORM:
  case DXGI_FORMAT_G8R8_G8B8_UNORM:
  case DXGI_FORMAT_YUY2:
    packed = true;
    bpe = 4;
    break;

  case DXGI_FORMAT_Y210:
  case DXGI_FORMAT_Y216:
    packed = true;
    bpe = 8;
    break;

  case DXGI_FORMAT_NV12:
  case DXGI_FORMAT_420_OPAQUE:
    planar = true;
    bpe = 2;
    break;

  case DXGI_FORMAT_P010:
  case DXGI_FORMAT_P016:
    planar = true;
    bpe = 4;
    break;

#if defined(_XBOX_ONE) && defined(_TITLE)

  case DXGI_FORMAT_D16_UNORM_S8_UINT:
  case DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
  case DXGI_FORMAT_X16_TYPELESS_G8_UINT:
    planar = true;
    bpe = 4;
    break;

#endif
  }

  if ( bc )
  {
    size_t numBlocksWide = 0;
    if ( width > 0 )
    {
      numBlocksWide = max( 1, ( width + 3 ) / 4 );
    }
    size_t numBlocksHigh = 0;
    if ( height > 0 )
    {
      numBlocksHigh = max( 1, ( height + 3 ) / 4 );
    }
    rowBytes = numBlocksWide * bpe;
    numRows = numBlocksHigh;
    numBytes = rowBytes * numBlocksHigh;
  }
  else if ( packed )
  {
    rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
    numRows = height;
    numBytes = rowBytes * height;
  }
  else if ( fmt == DXGI_FORMAT_NV11 )
  {
    rowBytes = ( ( width + 3 ) >> 2 ) * 4;
    numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
    numBytes = rowBytes * numRows;
  }
  else if ( planar )
  {
    rowBytes = ( ( width + 1 ) >> 1 ) * bpe;
    numBytes = ( rowBytes * height ) + ( ( rowBytes * height + 1 ) >> 1 );
    numRows = height + ( ( height + 1 ) >> 1 );
  }
  else
  {
    size_t bpp = BitsPerPixel( fmt );
    rowBytes = ( width * bpp + 7 ) / 8; // round up to nearest byte
    numRows = height;
    numBytes = rowBytes * height;
  }

  if ( outNumBytes )
  {
    *outNumBytes = numBytes;
  }
  if ( outRowBytes )
  {
    *outRowBytes = rowBytes;
  }
  if ( outNumRows )
  {
    *outNumRows = numRows;
  }
}

static bool IsCompressed( _In_ DXGI_FORMAT fmt )
{
  switch ( fmt )
  {
  case DXGI_FORMAT_BC1_TYPELESS:
  case DXGI_FORMAT_BC1_UNORM:
  case DXGI_FORMAT_BC1_UNORM_SRGB:
  case DXGI_FORMAT_BC2_TYPELESS:
  case DXGI_FORMAT_BC2_UNORM:
  case DXGI_FORMAT_BC2_UNORM_SRGB:
  case DXGI_FORMAT_BC3_TYPELESS:
  case DXGI_FORMAT_BC3_UNORM:
  case DXGI_FORMAT_BC3_UNORM_SRGB:
  case DXGI_FORMAT_BC4_TYPELESS:
  case DXGI_FORMAT_BC4_UNORM:
  case DXGI_FORMAT_BC4_SNORM:
  case DXGI_FORMAT_BC5_TYPELESS:
  case DXGI_FORMAT_BC5_UNORM:
  case DXGI_FORMAT_BC5_SNORM:
  case DXGI_FORMAT_BC6H_TYPELESS:
  case DXGI_FORMAT_BC6H_UF16:
  case DXGI_FORMAT_BC6H_SF16:
  case DXGI_FORMAT_BC7_TYPELESS:
  case DXGI_FORMAT_BC7_UNORM:
  case DXGI_FORMAT_BC7_UNORM_SRGB:
    return true;

  default:
    return false;
  }
}

static HRESULT CaptureTexture( ID3D11Device *d3dDevice, _In_ ID3D11DeviceContext* pContext, _In_ ID3D11Resource* pSource, _Inout_ D3D11_TEXTURE2D_DESC& desc, _Inout_ ID3D11Texture2D *&pStaging )
{
  if ( !pContext || !pSource )
    return E_INVALIDARG;

  D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
  pSource->GetType( &resType );

  if ( resType != D3D11_RESOURCE_DIMENSION_TEXTURE2D )
    return HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );

  ID3D11Texture2D *pTexture;
  HRESULT hr = pSource->QueryInterface( __uuidof( ID3D11Texture2D ), (void**)&pTexture );
  if ( FAILED( hr ) )
    return hr;

  //assert(pTexture);

  pTexture->GetDesc( &desc );

  if ( desc.SampleDesc.Count > 1 )
  {
    // MSAA content must be resolved before being copied to a staging texture
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;

    ID3D11Texture2D* pTemp;
    hr = d3dDevice->CreateTexture2D( &desc, 0, &pTemp );
    if ( FAILED( hr ) )
    {
      if ( pTexture ) pTexture->Release();
      return hr;
    }

    if ( !pTemp )
      LOG_ERR( "[Core2] Error creating temp texture" );

    DXGI_FORMAT fmt = EnsureNotTypeless( desc.Format );

    UINT support = 0;
    hr = d3dDevice->CheckFormatSupport( fmt, &support );
    if ( FAILED( hr ) )
    {
      if ( pTemp ) pTemp->Release();
      if ( pTexture ) pTexture->Release();
      return hr;
    }

    if ( !( support & D3D11_FORMAT_SUPPORT_MULTISAMPLE_RESOLVE ) )
    {
      if ( pTemp ) pTemp->Release();
      if ( pTexture ) pTexture->Release();
      return E_FAIL;
    }

    for ( UINT item = 0; item < desc.ArraySize; ++item )
    {
      for ( UINT level = 0; level < desc.MipLevels; ++level )
      {
        UINT index = D3D11CalcSubresource( level, item, desc.MipLevels );
        pContext->ResolveSubresource( pTemp, index, pSource, index, fmt );
      }
    }

    desc.BindFlags = 0;
    desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;

    hr = d3dDevice->CreateTexture2D( &desc, 0, &pStaging );
    if ( FAILED( hr ) )
    {
      if ( pTemp ) pTemp->Release();
      if ( pTexture ) pTexture->Release();
      return hr;
    }

    if ( !pStaging )
      LOG_ERR( "[Core2] Error creating staging texture" );

    pContext->CopyResource( pStaging, pTemp );
    if ( pTemp ) pTemp->Release();
    if ( pTexture ) pTexture->Release();
  }
  else if ( ( desc.Usage == D3D11_USAGE_STAGING ) && ( desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ ) )
  {
    // Handle case where the source is already a staging texture we can use directly
    pStaging = pTexture;
  }
  else
  {
    // Otherwise, create a staging texture from the non-MSAA source
    desc.BindFlags = 0;
    desc.MiscFlags &= D3D11_RESOURCE_MISC_TEXTURECUBE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;

    hr = d3dDevice->CreateTexture2D( &desc, 0, &pStaging );
    if ( FAILED( hr ) )
    {
      if ( pTexture ) pTexture->Release();
      return hr;
    }

    if ( !pStaging )
      LOG_ERR( "[Core2] Error creating staging texture" );

    pContext->CopyResource( pStaging, pSource );
    if ( pTexture ) pTexture->Release();
  }

  return S_OK;
}

HRESULT SaveDDSTexture( _In_ ID3D11DeviceContext* pContext, _In_ ID3D11Resource* pSource, CStreamWriter &Writer )
{
  const TU32 DDS_MAGIC = 0x20534444; // "DDS "

  struct DDS_PIXELFORMAT
  {
    TU32    size;
    TU32    flags;
    TU32    fourCC;
    TU32    RGBBitCount;
    TU32    RBitMask;
    TU32    GBitMask;
    TU32    BBitMask;
    TU32    ABitMask;
  };

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) ((TU32)(TU8)(ch0) | ((TU32)(TU8)(ch1) << 8) | ((TU32)(TU8)(ch2) << 16) | ((TU32)(TU8)(ch3) << 24 ))
#endif // !MAKEFOURCC

#define DDS_HEADER_FLAGS_TEXTURE        0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP         0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH          0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE     0x00080000  // DDSD_LINEARSIZE

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
	DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
	DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME

  // Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
  enum DDS_RESOURCE_DIMENSION
  {
    DDS_DIMENSION_TEXTURE1D = 2,
    DDS_DIMENSION_TEXTURE2D = 3,
    DDS_DIMENSION_TEXTURE3D = 4,
  };

  // Subset here matches D3D10_RESOURCE_MISC_FLAG and D3D11_RESOURCE_MISC_FLAG
  enum DDS_RESOURCE_MISC_FLAG
  {
    DDS_RESOURCE_MISC_TEXTURECUBE = 0x4L,
  };

  enum DDS_MISC_FLAGS2
  {
    DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
  };

  struct DDS_HEADER
  {
    TU32        size;
    TU32        flags;
    TU32        height;
    TU32        width;
    TU32        pitchOrLinearSize;
    TU32        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    TU32        mipMapCount;
    TU32        reserved1[ 11 ];
    DDS_PIXELFORMAT ddspf;
    TU32        caps;
    TU32        caps2;
    TU32        caps3;
    TU32        caps4;
    TU32        reserved2;
  };

  struct DDS_HEADER_DXT10
  {
    DXGI_FORMAT     dxgiFormat;
    TU32        resourceDimension;
    TU32        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
    TU32        arraySize;
    TU32        miscFlags2; // see DDS_MISC_FLAGS2
  };


  const DDS_PIXELFORMAT DDSPF_DXT1 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D', 'X', 'T', '1' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_DXT2 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D', 'X', 'T', '2' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_DXT3 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D', 'X', 'T', '3' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_DXT4 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D', 'X', 'T', '4' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_DXT5 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D', 'X', 'T', '5' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_BC4_UNORM = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B', 'C', '4', 'U' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_BC4_SNORM = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B', 'C', '4', 'S' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_BC5_UNORM = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B', 'C', '5', 'U' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_BC5_SNORM = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'B', 'C', '5', 'S' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_R8G8_B8G8 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'R', 'G', 'B', 'G' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_G8R8_G8B8 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'G', 'R', 'G', 'B' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_YUY2 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'Y', 'U', 'Y', '2' ), 0, 0, 0, 0, 0 };
  const DDS_PIXELFORMAT DDSPF_A8R8G8B8 = { sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };
  const DDS_PIXELFORMAT DDSPF_X8R8G8B8 = { sizeof( DDS_PIXELFORMAT ), DDS_RGB, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };
  const DDS_PIXELFORMAT DDSPF_A8B8G8R8 = { sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 };
  const DDS_PIXELFORMAT DDSPF_X8B8G8R8 = { sizeof( DDS_PIXELFORMAT ), DDS_RGB, 0, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0x00000000 };
  const DDS_PIXELFORMAT DDSPF_G16R16 = { sizeof( DDS_PIXELFORMAT ), DDS_RGB, 0, 32, 0x0000ffff, 0xffff0000, 0x00000000, 0x00000000 };
  const DDS_PIXELFORMAT DDSPF_R5G6B5 = { sizeof( DDS_PIXELFORMAT ), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };
  const DDS_PIXELFORMAT DDSPF_A1R5G5B5 = { sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };
  const DDS_PIXELFORMAT DDSPF_A4R4G4B4 = { sizeof( DDS_PIXELFORMAT ), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };
  const DDS_PIXELFORMAT DDSPF_R8G8B8 = { sizeof( DDS_PIXELFORMAT ), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };
  const DDS_PIXELFORMAT DDSPF_L8 = { sizeof( DDS_PIXELFORMAT ), DDS_LUMINANCE, 0, 8, 0xff, 0x00, 0x00, 0x00 };
  const DDS_PIXELFORMAT DDSPF_L16 = { sizeof( DDS_PIXELFORMAT ), DDS_LUMINANCE, 0, 16, 0xffff, 0x0000, 0x0000, 0x0000 };
  const DDS_PIXELFORMAT DDSPF_A8L8 = { sizeof( DDS_PIXELFORMAT ), DDS_LUMINANCEA, 0, 16, 0x00ff, 0x0000, 0x0000, 0xff00 };
  const DDS_PIXELFORMAT DDSPF_A8 = { sizeof( DDS_PIXELFORMAT ), DDS_ALPHA, 0, 8, 0x00, 0x00, 0x00, 0xff };

  // D3DFMT_A2R10G10B10/D3DFMT_A2B10G10R10 should be written using DX10 extension to avoid D3DX 10:10:10:2 reversal issue
  // This indicates the DDS_HEADER_DXT10 extension is present (the format is in dxgiFormat)
  const DDS_PIXELFORMAT DDSPF_DX10 = { sizeof( DDS_PIXELFORMAT ), DDS_FOURCC, MAKEFOURCC( 'D', 'X', '1', '0' ), 0, 0, 0, 0, 0 };

  D3D11_TEXTURE2D_DESC desc = { 0 };
  ID3D11Texture2D *pStaging = NULL;

  ID3D11Device *d3dDevice;
  pContext->GetDevice( &d3dDevice );

  HRESULT hr = CaptureTexture( d3dDevice, pContext, pSource, desc, pStaging );
  d3dDevice->Release();

  if ( FAILED( hr ) )
  {
    if ( pStaging ) pStaging->Release();
    return hr;
  }

  // Create file
  //#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
  //	ScopedHandle hFile(safe_handle(CreateFile2(fileName, GENERIC_WRITE, 0, CREATE_ALWAYS, 0)));
  //#else
  //	ScopedHandle hFile(safe_handle(CreateFileW(fileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0)));
  //#endif
  //	if (!hFile)
  //		return HRESULT_FROM_WIN32(GetLastError());

  // Setup header
  const size_t MAX_HEADER_SIZE = sizeof( TU32 ) + sizeof( DDS_HEADER ) + sizeof( DDS_HEADER_DXT10 );
  TU8 fileHeader[ MAX_HEADER_SIZE ];

  *( (TU32*)( &fileHeader[ 0 ] ) ) = DDS_MAGIC;

  auto header = (DDS_HEADER*)( &fileHeader[ 0 ] + sizeof( TU32 ) );
  size_t headerSize = sizeof( TU32 ) + sizeof( DDS_HEADER );
  memset( header, 0, sizeof( DDS_HEADER ) );
  header->size = sizeof( DDS_HEADER );
  header->flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
  header->height = desc.Height;
  header->width = desc.Width;
  header->mipMapCount = 1;
  header->caps = DDS_SURFACE_FLAGS_TEXTURE;

  // Try to use a legacy .DDS pixel format for better tools support, otherwise fallback to 'DX10' header extension
  DDS_HEADER_DXT10* extHeader = nullptr;
  switch ( desc.Format )
  {
  case DXGI_FORMAT_R8G8B8A8_UNORM:        memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_A8B8G8R8, sizeof( DDS_PIXELFORMAT ) );    break;
  case DXGI_FORMAT_R16G16_UNORM:          memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_G16R16, sizeof( DDS_PIXELFORMAT ) );      break;
  case DXGI_FORMAT_R8G8_UNORM:            memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_A8L8, sizeof( DDS_PIXELFORMAT ) );        break;
  case DXGI_FORMAT_R16_UNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_L16, sizeof( DDS_PIXELFORMAT ) );         break;
  case DXGI_FORMAT_R8_UNORM:              memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_L8, sizeof( DDS_PIXELFORMAT ) );          break;
  case DXGI_FORMAT_A8_UNORM:              memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_A8, sizeof( DDS_PIXELFORMAT ) );          break;
  case DXGI_FORMAT_R8G8_B8G8_UNORM:       memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_R8G8_B8G8, sizeof( DDS_PIXELFORMAT ) );   break;
  case DXGI_FORMAT_G8R8_G8B8_UNORM:       memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_G8R8_G8B8, sizeof( DDS_PIXELFORMAT ) );   break;
  case DXGI_FORMAT_BC1_UNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_DXT1, sizeof( DDS_PIXELFORMAT ) );        break;
  case DXGI_FORMAT_BC2_UNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_DXT3, sizeof( DDS_PIXELFORMAT ) );        break;
  case DXGI_FORMAT_BC3_UNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_DXT5, sizeof( DDS_PIXELFORMAT ) );        break;
  case DXGI_FORMAT_BC4_UNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_BC4_UNORM, sizeof( DDS_PIXELFORMAT ) );   break;
  case DXGI_FORMAT_BC4_SNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_BC4_SNORM, sizeof( DDS_PIXELFORMAT ) );   break;
  case DXGI_FORMAT_BC5_UNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_BC5_UNORM, sizeof( DDS_PIXELFORMAT ) );   break;
  case DXGI_FORMAT_BC5_SNORM:             memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_BC5_SNORM, sizeof( DDS_PIXELFORMAT ) );   break;
  case DXGI_FORMAT_B5G6R5_UNORM:          memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_R5G6B5, sizeof( DDS_PIXELFORMAT ) );      break;
  case DXGI_FORMAT_B5G5R5A1_UNORM:        memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_A1R5G5B5, sizeof( DDS_PIXELFORMAT ) );    break;
  case DXGI_FORMAT_B8G8R8A8_UNORM:        memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_A8R8G8B8, sizeof( DDS_PIXELFORMAT ) );    break; // DXGI 1.1
  case DXGI_FORMAT_B8G8R8X8_UNORM:        memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_X8R8G8B8, sizeof( DDS_PIXELFORMAT ) );    break; // DXGI 1.1
  case DXGI_FORMAT_YUY2:                  memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_YUY2, sizeof( DDS_PIXELFORMAT ) );        break; // DXGI 1.2
  case DXGI_FORMAT_B4G4R4A4_UNORM:        memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_A4R4G4B4, sizeof( DDS_PIXELFORMAT ) );    break; // DXGI 1.2

    // Legacy D3DX formats using D3DFMT enum value as FourCC
  case DXGI_FORMAT_R32G32B32A32_FLOAT:    header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 116; break; // D3DFMT_A32B32G32R32F
  case DXGI_FORMAT_R16G16B16A16_FLOAT:    header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 113; break; // D3DFMT_A16B16G16R16F
  case DXGI_FORMAT_R16G16B16A16_UNORM:    header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 36;  break; // D3DFMT_A16B16G16R16
  case DXGI_FORMAT_R16G16B16A16_SNORM:    header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 110; break; // D3DFMT_Q16W16V16U16
  case DXGI_FORMAT_R32G32_FLOAT:          header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 115; break; // D3DFMT_G32R32F
  case DXGI_FORMAT_R16G16_FLOAT:          header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 112; break; // D3DFMT_G16R16F
  case DXGI_FORMAT_R32_FLOAT:             header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 114; break; // D3DFMT_R32F
  case DXGI_FORMAT_R16_FLOAT:             header->ddspf.size = sizeof( DDS_PIXELFORMAT ); header->ddspf.flags = DDS_FOURCC; header->ddspf.fourCC = 111; break; // D3DFMT_R16F

  case DXGI_FORMAT_AI44:
  case DXGI_FORMAT_IA44:
  case DXGI_FORMAT_P8:
  case DXGI_FORMAT_A8P8:
  {
    if ( pStaging ) pStaging->Release();
    return HRESULT_FROM_WIN32( ERROR_NOT_SUPPORTED );
  }

  default:
    memcpy_s( &header->ddspf, sizeof( header->ddspf ), &DDSPF_DX10, sizeof( DDS_PIXELFORMAT ) );

    headerSize += sizeof( DDS_HEADER_DXT10 );
    extHeader = (DDS_HEADER_DXT10*)( (TU8*)( &fileHeader[ 0 ] ) + sizeof( TU32 ) + sizeof( DDS_HEADER ) );
    memset( extHeader, 0, sizeof( DDS_HEADER_DXT10 ) );
    extHeader->dxgiFormat = desc.Format;
    extHeader->resourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
    extHeader->arraySize = 1;
    break;
  }

  size_t rowPitch, slicePitch, rowCount;
  GetSurfaceInfo( desc.Width, desc.Height, desc.Format, &slicePitch, &rowPitch, &rowCount );

  if ( IsCompressed( desc.Format ) )
  {
    header->flags |= DDS_HEADER_FLAGS_LINEARSIZE;
    header->pitchOrLinearSize = (TU32)( slicePitch );
  }
  else
  {
    header->flags |= DDS_HEADER_FLAGS_PITCH;
    header->pitchOrLinearSize = (TU32)( rowPitch );
  }

  // Setup pixels
  TU8 *pixels = new TU8[ slicePitch ];// (new (std::nothrow) TU8[slicePitch]);
  if ( !pixels )
  {
    SAFEDELETEA( pixels );
    if ( pStaging ) pStaging->Release();
    return E_OUTOFMEMORY;
  }

  D3D11_MAPPED_SUBRESOURCE mapped;
  hr = pContext->Map( pStaging, 0, D3D11_MAP_READ, 0, &mapped );
  if ( FAILED( hr ) )
  {
    SAFEDELETEA( pixels );
    if ( pStaging ) pStaging->Release();
    return hr;
  }

  auto sptr = (TU8*)( mapped.pData );
  if ( !sptr )
  {
    pContext->Unmap( pStaging, 0 );
    SAFEDELETEA( pixels );
    if ( pStaging ) pStaging->Release();
    return E_POINTER;
  }

  TU8* dptr = pixels;

  size_t msize = min( rowPitch, mapped.RowPitch );
  for ( size_t h = 0; h < rowCount; ++h )
  {
    memcpy_s( dptr, rowPitch, sptr, msize );
    sptr += mapped.RowPitch;
    dptr += rowPitch;
  }

  pContext->Unmap( pStaging, 0 );

  // Write header & pixels

  if ( !Writer.Write( &fileHeader, (DWORD)( headerSize ) ) )
  {
    SAFEDELETEA( pixels );
    if ( pStaging ) pStaging->Release();
    return E_FAIL;
  }
  if ( !Writer.Write( pixels, (DWORD)( slicePitch ) ) )
  {
    SAFEDELETEA( pixels );
    if ( pStaging ) pStaging->Release();
    return E_FAIL;
}

  SAFEDELETEA( pixels );
  if ( pStaging ) pStaging->Release();
  return S_OK;
}


#else
NoEmptyFile();
#endif