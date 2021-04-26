#include "BaseLib.h"


const CColor Lerp( const CColor a, const CColor b, const TF32 t )
{
  CColor c;
  for ( TS32 x = 0; x < 4; x++ )
    c[ x ] = (TU8)( ( b[ x ] - a[ x ] )*t + a[ x ] );
  return c;
}

TU8 & CColor::A()
{
  return a;
}

TU8 & CColor::B()
{
  return b;
}

TU8 & CColor::G()
{
  return g;
}

TU8 & CColor::R()
{
  return r;
}

CColor::operator TU32() const
{
  return argb();
}

TU32 CColor::argb() const
{
  return ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;
}

CColor CColor::FromABGR( const TU32 v )
{
  CColor res;
  res.a = (TU8)( v >> 24 );
  res.b = (TU8)( v >> 16 );
  res.g = (TU8)( v >> 8 );
  res.r = (TU8)( v >> 0 );
  return res;
}

CColor CColor::FromARGB( const TU32 v )
{
  CColor res;
  res.a = (TU8)( v >> 24 );
  res.r = (TU8)( v >> 16 );
  res.g = (TU8)( v >> 8 );
  res.b = (TU8)( v >> 0 );
  return res;
}

CColor CColor::FromFloat( const TF32 _r, const TF32 _g, const TF32 _b, const TF32 _a )
{
  CColor res;

  TF32 c[ 4 ];
  c[ 0 ] = _r;
  c[ 1 ] = _g;
  c[ 2 ] = _b;
  c[ 3 ] = _a;
  for ( TS32 x = 0; x < 4; x++ )
  {
    if ( c[ x ] < 0 ) c[ x ] = 0;
    if ( c[ x ] > 1 ) c[ x ] = 1;
    res[ x ] = (TU8)( c[ x ] * 255 );
  }
  return res;
}

CColor::CColor( const TU32 argb )
{
  r = (TU8)( argb >> 16 );
  g = (TU8)( argb >> 8 );
  b = (TU8)( argb >> 0 );
  a = (TU8)( argb >> 24 );
}

CColor::CColor( const TU8* c )
{
  r = c[ 0 ];
  g = c[ 1 ];
  b = c[ 2 ];
  a = c[ 3 ];
}

CColor::CColor( const TU8 _r, const TU8 _g, const TU8 _b, const TU8 _a )
{
  r = _r;
  g = _g;
  b = _b;
  a = _a;
}

CColor::CColor()
{

}

TU8 const CColor::operator[]( TS32 idx ) const
{
  return ( ( const TU8* )this )[ idx ]; // nem fog ez kesobb fajni?!
}

TU8 &CColor::operator[]( TS32 idx )
{
  return ( ( TU8* )this )[ idx ];
}

CColor::operator TU8* ( )
{
  return ( TU8* )this;
}

CColor::operator const TU8* ( ) const
{
  return ( const TU8* )this;
}

TBOOL CColor::operator== ( const CColor &c ) const
{
  return r == c.r && g == c.g && b == c.b && a == c.a;
}

TBOOL CColor::operator!= ( const CColor &c ) const
{
  return r != c.r || g != c.g || b != c.b || a != c.a;
}
