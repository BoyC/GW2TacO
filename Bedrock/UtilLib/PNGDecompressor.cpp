#include "PNGDecompressor.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

TBOOL DecompressPNG( const TU8 *IData, TS32 IDataSize, TU8 *&Image, TS32 &XRes, TS32 &YRes )
{
  TS32 x, y, n;
  TU8 *Data = stbi_load_from_memory( IData, IDataSize, &x, &y, &n, 4 );

  if ( !Data )
  {
    LOG_ERR( "[png] Image decompression failed: %s", stbi_failure_reason() );
    return false;
  }

  XRes = x;
  YRes = y;
  Image = new TU8[ XRes*YRes * 4 ];
  memcpy( Image, Data, XRes*YRes * 4 );

  stbi_image_free( Data );
  return true;
}

void ARGBtoABGR( TU8 *Image, TS32 XRes, TS32 YRes )
{
  TU8 *img = Image;

  for ( TS32 x = 0; x < XRes*YRes; x++ )
  {
    TU8 t = img[ 0 ];
    img[ 0 ] = img[ 2 ];
    img[ 2 ] = t;
    img += 4;
  }
}

void ClearZeroAlpha( TU8 *Image, TS32 XRes, TS32 YRes )
{
  TU8 *img = Image;

  for ( TS32 x = 0; x < XRes*YRes; x++ )
  {
    if ( img[ 3 ] == 0 )
      img[ 0 ] = img[ 1 ] = img[ 2 ] = 0;

    img += 4;
  }
}

TBOOL ExportPNG( TU8 *Image, TS32 XRes, TS32 YRes, TBOOL ClearAlpha, CString OutFile )
{
  TU8 *Data = new TU8[ XRes*YRes * 4 ];
  memcpy( Data, Image, XRes*YRes * 4 );

  if ( ClearAlpha )
    for ( TS32 x = 0; x < XRes*YRes; x++ )
      Image[ x * 4 + 3 ] = 255;

  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  TBOOL result = stbi_write_png( FileName, XRes, YRes, 4, Image, XRes * 4 );

  if ( !result )
    LOG_ERR( "[png] PNG export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( Data );
  SAFEDELETEA( FileName );

  return result;
}

TBOOL ExportTga( TU8 *Image, TS32 XRes, TS32 YRes, TBOOL ClearAlpha, CString OutFile )
{
  if ( ClearAlpha )
    for ( TS32 x = 0; x < XRes*YRes; x++ )
      Image[ x * 4 + 3 ] = 255;

  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  TBOOL result = stbi_write_tga( FileName, XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] TGA export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( FileName );

  return result;
}

TBOOL ExportBmp( TU8 *Image, TS32 XRes, TS32 YRes, CString OutFile )
{
  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  TBOOL result = stbi_write_bmp( FileName, XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] BMP export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( FileName );

  return result;
}

TBOOL ExportRaw( TU8 *Image, TS32 XRes, TS32 YRes, CString OutFile )
{
  TS8 *FileName = new TS8[ OutFile.Length() + 1 ];
  OutFile.WriteAsMultiByte( FileName, OutFile.Length() + 1 );

  TBOOL result = stbi_write_bmp( FileName, XRes, YRes, 4, Image );

  if ( !result )
    LOG_ERR( "[png] BMP export error ('%s')", OutFile.GetPointer() );

  SAFEDELETEA( FileName );

  return result;
}
