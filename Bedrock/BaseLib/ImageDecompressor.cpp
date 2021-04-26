#include "BaseLib.h"
#include <olectl.h>

#define HIMETRIC_INCH	2540

TU8 *DecompressImage( const TU8 *ImageData, TS32 ImageDataSize, TS32 &XSize, TS32 &YSize )
{
  if ( !ImageData || !ImageDataSize ) return 0;

  XSize = YSize = 0;
  LPPICTURE gpPicture = NULL;

  LPVOID pvData = NULL;
  HGLOBAL hGlobal = GlobalAlloc( GMEM_MOVEABLE, ImageDataSize );

  if ( !hGlobal )
    return 0;

  pvData = GlobalLock( hGlobal );
  if ( !pvData )
  {
    GlobalFree( hGlobal );
    return 0;
  }

  memcpy( pvData, ImageData, ImageDataSize );

  GlobalUnlock( hGlobal );

  if ( GetLastError() != NO_ERROR )
  {
    GlobalFree( hGlobal );
    return 0;
  }

  LPSTREAM pstm = NULL;
  HRESULT res = CreateStreamOnHGlobal( hGlobal, TRUE, &pstm );
  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[base] CreateStreamOnHGlobal failed (%s)" ), err.ErrorMessage() );
    return 0;
  }

  res = OleLoadPicture( pstm, ImageDataSize, FALSE, IID_IPicture, (LPVOID *)&gpPicture );

  if ( res != S_OK )
  {
    if ( res != 0x800A01E1 ) // this is given when a png is loaded through oleloadpicture
    {
      _com_error err( res );
      LOG( LOG_DEBUG, _T( "[base] OleLoadPicture failed (%s)" ), err.ErrorMessage() );
    }
    pstm->Release();
    GlobalFree( hGlobal );
    return 0;
  }

  pstm->Release();
  GlobalFree( hGlobal );

  if ( !gpPicture ) return NULL;

  OLE_XSIZE_HIMETRIC hmWidth;
  OLE_YSIZE_HIMETRIC hmHeight;

  gpPicture->get_Width( &hmWidth );
  gpPicture->get_Height( &hmHeight );

  HDC hdc = GetDC( NULL );
  HDC mdc = CreateCompatibleDC( hdc );

  XSize = MulDiv( hmWidth, GetDeviceCaps( mdc, LOGPIXELSX ), HIMETRIC_INCH );
  YSize = MulDiv( hmHeight, GetDeviceCaps( mdc, LOGPIXELSY ), HIMETRIC_INCH );

  TU8 *Image = new TU8[ XSize*YSize * 4 ];
  memset( Image, 0, XSize*YSize * 4 );

  HBITMAP bm = CreateCompatibleBitmap( hdc, XSize, YSize );
  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof( bmi.bmiHeader );
  bmi.bmiHeader.biWidth = XSize;
  bmi.bmiHeader.biHeight = YSize;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  HGDIOBJ oldbm = SelectObject( mdc, bm );
  RECT r = { 0, 0, XSize, YSize };

  FillRect( mdc, &r, (HBRUSH)GetStockObject( BLACK_BRUSH ) );

  SetBkMode( mdc, TRANSPARENT );

  res = gpPicture->Render( mdc, 0, YSize - 1, XSize, -YSize, 0, hmHeight, hmWidth, -hmHeight, &r );

  if ( res != S_OK )
  {
    _com_error err( res );
    LOG( LOG_ERROR, _T( "[base] gpPicture->Render failed (%s)" ), err.ErrorMessage() );
    SAFEDELETEA( Image );
  }
  else
  {
    GetDIBits( mdc, bm, 0, YSize, Image, &bmi, DIB_RGB_COLORS );
  }

  SelectObject( mdc, oldbm );
  DeleteDC( mdc );
  DeleteObject( bm );
  ReleaseDC( NULL, hdc );

  gpPicture->Release();

  for ( TS32 x = 0; x < XSize*YSize; x++ )
    Image[ x * 4 + 3 ] = 255;

  return Image;
}
