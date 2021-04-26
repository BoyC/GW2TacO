#include "BaseLib.h"

const TS32 Lerp( const TS32 v1, const TS32 v2, const TF32 t )
{
  return (TS32)( ( v2 - v1 )*t + v1 );
}

const TF32 Lerp( const TF32 v1, const TF32 v2, const TF32 t )
{
  return ( v2 - v1 )*t + v1;
}

const TU32 Lerp( const TU32 v1, const TU32 v2, const TF32 t )
{
  return (TU32)( ( v2 - v1 )*t + v1 );
}

const TS64 Lerp( const TS64 v1, const TS64 v2, const TF32 t )
{
  return (TS64)( ( v2 - v1 )*t + v1 );
}

const TF64 Lerp( const TF64 v1, const TF64 v2, const TF32 t )
{
  return ( v2 - v1 )*t + v1;
}

const TU64 Lerp( const TU64 v1, const TU64 v2, const TF32 t )
{
  return (TU64)( ( v2 - v1 )*t + v1 );
}

const CVector2 Lerp( const CVector2 &v1, const CVector2 &v2, const TF32 t )
{
  return ( v2 - v1 )*t + v1;
}

const CVector2I Lerp( const CVector2I &v1, const CVector2I &v2, const TF32 t )
{
  return ( v2 - v1 )*t + v1;
}

const CVector3 Lerp( const CVector3 &v1, const CVector3 &v2, const TF32 t )
{
  return ( v2 - v1 )*t + v1;
}

const CVector4 Lerp( const CVector4 &v1, const CVector4 &v2, const TF32 t )
{
  return ( v2 - v1 )*t + v1;
}

TF32 mod( TF32 v, TF32 m )
{
  return m * (TS32)floor( v / m );
}

TS32 Mod( TS32 a, TS32 b )
{
  return ( ( a%b ) + b ) % b;
}

TF32 Mod( TF32 a, TF32 b )
{
  return fmodf( ( ( fmodf( a, b ) ) + b ), b );
}

TF32 Mod( TF32 a, TS32 b )
{
  return fmodf( ( ( fmodf( a, (TF32)b ) ) + b ), (TF32)b );
}

CVector3 CVector3::operator*( const CQuaternion &q ) const
{
  return *this*CMatrix3x3( q );
}

CVector3 &CVector3::operator*=( const CQuaternion &q )
{
  return *this = *this*CMatrix3x3( q );
}

CVector3 CVector3::operator/( const CQuaternion &q ) const
{
  return *this*CMatrix3x3( q ).Transposed(); //rot matrix inverse=transpose
}

CVector3 &CVector3::operator/=( const CQuaternion &q )
{
  return *this = *this*CMatrix3x3( q ).Transposed(); //rot matrix inverse=transpose
}

CVector3 CVector3::operator*( const CMatrix3x3 &q ) const
{
  return q.Apply( *this );
}

CVector3 &CVector3::operator*=( const CMatrix3x3 &q )
{
  return *this = q.Apply( *this );
}

CVector4 CVector3::operator*( const CMatrix4x4 &q ) const
{
  return q.Apply( *this );
}

CVector4 CVector4::operator*( const CMatrix4x4 &q ) const
{
  return q.Apply( *this );
}

CVector4 CVector4::Cross( const CVector4 &v1, const CVector4 &v2, const CVector4 &v3 )
{
  return CVector4(
    v1.y*( v2.z*v3.w - v3.z*v2.w ) - v1.z*( v2.y*v3.w - v3.y*v2.w ) + v1.w*( v2.y*v3.z - v2.z*v3.y ),
    -( v1.x*( v2.z*v3.w - v3.z*v2.w ) - v1.z*( v2.x*v3.w - v3.x*v2.w ) + v1.w*( v2.x*v3.z - v3.x*v2.z ) ),
    v1.x*( v2.y*v3.w - v3.y*v2.w ) - v1.y*( v2.x*v3.w - v3.x*v2.w ) + v1.w*( v2.x*v3.y - v3.x*v2.y ),
    -( v1.x*( v2.y*v3.z - v3.y*v2.z ) - v1.y*( v2.x*v3.z - v3.x*v2.z ) + v1.z*( v2.x*v3.y - v3.x*v2.y ) ) );
}

TF32 CVector4::Dot( const CVector4 &v1, const CVector4 &v2 )
{
  return v1*v2;
}

void CVector4::Homogenize()
{
  *this = Homogenized();
}

CVector4 CVector4::Homogenized() const
{
  return *this / w;
}

void CVector4::Normalize()
{
  *this = Normalized();
}

CVector4 CVector4::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

TF32 CVector4::LengthSquared() const
{
  return x*x + y*y + z*z + w*w;
}

TF32 CVector4::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector4::CVector4( const CVector4 &v )
{
  x = v.x;
  y = v.y;
  z = v.z;
  w = v.w;
}

CVector4::CVector4( const TF32* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
  z = v[ 2 ];
  w = v[ 3 ];
}

CVector4::CVector4( const TF32 _x, const TF32 _y, const TF32 _z, const TF32 _w )
{
  x = _x;
  y = _y;
  z = _z;
  w = _w;
}

CVector4::CVector4()
{

}

CVector3 CVector3::operator* ( const CPRS &q ) const
{
  return q.Apply( *this );
}

CVector3 &CVector3::operator*= ( const CPRS &q )
{
  return *this = q.Apply( *this );
}

CVector3 CVector3::Cross( const CVector3 &v1, const CVector3 &v2 )
{
  return v1%v2;
}

TF32 CVector3::Dot( const CVector3 &v1, const CVector3 &v2 )
{
  return v1*v2;
}

void CVector3::Normalize()
{
  *this = Normalized();
}

CVector3 CVector3::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

TF32 CVector3::LengthSquared() const
{
  return x*x + y*y + z*z;
}

TF32 CVector3::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector3::CVector3( const CVector3 &v )
{
  x = v.x;
  y = v.y;
  z = v.z;
}

CVector3::CVector3( const TF32* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
  z = v[ 2 ];
}

CVector3::CVector3( const TF32 _x, const TF32 _y, const TF32 _z )
{
  x = _x;
  y = _y;
  z = _z;
}

CVector3::CVector3()
{

}


TF32 CVector2::Dot( const CVector2 &v1, const CVector2 &v2 )
{
  return v1*v2;
}

void CVector2::Normalize()
{
  *this = Normalized();
}

CVector2 CVector2::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

TF32 CVector2::LengthSquared() const
{
  return x*x + y*y;
}

TF32 CVector2::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector2::CVector2( const CVector2 &v )
{
  x = v.x;
  y = v.y;
}

CVector2::CVector2( const TF32* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
}

CVector2::CVector2( const TF32 _x, const TF32 _y )
{
  x = _x;
  y = _y;
}

CVector2::CVector2()
{

}

TF32 const CVector2::operator[]( TS32 idx ) const
{
  return ( ( const TF32* )this )[ idx ];
}

TF32 &CVector2::operator[]( TS32 idx )
{
  return ( ( TF32* )this )[ idx ];
}

CVector2::operator TF32* ( )
{
  return (TF32*)&x;
}

CVector2::operator const TF32* ( ) const
{
  return (const TF32*)&x;
}

CVector2 &CVector2::operator+= ( const CVector2 &v )
{
  x += v.x;
  y += v.y;
  return *this;
}

CVector2 &CVector2::operator-= ( const CVector2 &v )
{
  x -= v.x;
  y -= v.y;
  return *this;
}

CVector2 &CVector2::operator*= ( const TF32 f )
{
  x *= f;
  y *= f;
  return *this;
}

CVector2 CVector2::Rotated( const CVector2 &center, TF32 rotation )
{
  CVector2 n = ( *this ) - center;
  return CVector2( n.x*cosf( rotation ) - n.y*sinf( rotation ),
                   n.y*cosf( rotation ) + n.x*sinf( rotation ) ) + center;
}

CVector2 &CVector2::operator/= ( const TF32 f )
{
  TF32 fi = 1 / f;
  x *= fi;
  y *= fi;
  return *this;
}

CVector2 CVector2::operator+ () const
{
  return *this;
}

CVector2 CVector2::operator- () const
{
  return CVector2( -x, -y );
}

CVector2 CVector2::operator+ ( const CVector2 &v ) const
{
  return CVector2( x + v.x, y + v.y );
}

CVector2 CVector2::operator- ( const CVector2 &v ) const
{
  return CVector2( x - v.x, y - v.y );
}

CVector2 CVector2::operator* ( const TF32 f ) const
{
  return CVector2( x*f, y*f );
}

CVector2 CVector2::operator/ ( const TF32 f ) const
{
  TF32 fi = 1 / f;
  return CVector2( x*fi, y*fi );
}

TBOOL CVector2::operator== ( const CVector2 &v ) const
{
  return x == v.x && y == v.y;
}

TBOOL CVector2::operator!= ( const CVector2 &v ) const
{
  return x != v.x || y != v.y;
}

TF32 CVector2::operator* ( const CVector2 &v ) const //dot product
{
  return x*v.x + y*v.y;
}

TS32 CVector2I::Dot( const CVector2I &v1, const CVector2I &v2 )
{
  return v1*v2;
}

void CVector2I::Normalize()
{
  *this = Normalized();
}

CVector2I CVector2I::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

TF32 CVector2I::LengthSquared() const
{
  return (TF32)( x*x + y*y );
}

TF32 CVector2I::Length() const
{
  return sqrtf( LengthSquared() );
}

CVector2I::CVector2I( const CVector2I &v )
{
  x = v.x;
  y = v.y;
}

CVector2I::CVector2I( const TS32* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
}

CVector2I::CVector2I( const TS32 _x, const TS32 _y )
{
  x = _x;
  y = _y;
}

CVector2I::CVector2I()
{

}

TS32 const CVector2I::operator[]( TS32 idx ) const
{
  return ( ( const TS32* )this )[ idx ];
}

TS32 &CVector2I::operator[]( TS32 idx )
{
  return ( ( TS32* )this )[ idx ];
}

CVector2I::operator TS32* ( )
{
  return (TS32*)&x;
}

CVector2I::operator const TS32* ( ) const
{
  return (const TS32*)&x;
}

CVector2I &CVector2I::operator+= ( const CVector2I &v )
{
  x += v.x;
  y += v.y;
  return *this;
}

CVector2I &CVector2I::operator-= ( const CVector2I &v )
{
  x -= v.x;
  y -= v.y;
  return *this;
}

CVector2I &CVector2I::operator*= ( const TF32 f )
{
  x = (TS32)( x*f );
  y = (TS32)( y*f );
  return *this;
}

CVector2I &CVector2I::operator/= ( const TF32 f )
{
  TF32 fi = 1 / f;
  x = (TS32)( x*fi );
  y = (TS32)( y*fi );
  return *this;
}

CVector2I &CVector2I::operator/= ( const TS32 f )
{
  x = x / f;
  y = y / f;
  return *this;
}

CVector2I CVector2I::operator+ () const
{
  return *this;
}

CVector2I CVector2I::operator- () const
{
  return CVector2I( -x, -y );
}

CVector2I CVector2I::operator+ ( const CVector2I &v ) const
{
  return CVector2I( x + v.x, y + v.y );
}

CVector2I CVector2I::operator- ( const CVector2I &v ) const
{
  return CVector2I( x - v.x, y - v.y );
}

CVector2I CVector2I::operator* ( const TF32 f ) const
{
  return CVector2I( (TS32)( x*f ), (TS32)( y*f ) );
}

CVector2I CVector2I::operator/ ( const TF32 f ) const
{
  TF32 fi = 1 / f;
  return CVector2I( (TS32)( x*fi ), (TS32)( y*fi ) );
}

CVector2I CVector2I::operator/ ( const TS32 f ) const
{
  return CVector2I( x / f, y / f );
}

TBOOL CVector2I::operator== ( const CVector2I &v ) const
{
  return x == v.x && y == v.y;
}

TBOOL CVector2I::operator!= ( const CVector2I &v ) const
{
  return x != v.x || y != v.y;
}

TS32 CVector2I::operator* ( const CVector2I &v ) const //dot product
{
  return x*v.x + y*v.y;
}

TF32 const CVector3::operator[]( TS32 idx ) const
{
  return ( ( const TF32* )this )[ idx ];
}

TF32 &CVector3::operator[]( TS32 idx )
{
  return ( ( TF32* )this )[ idx ];
}

CVector3::operator TF32* ( )
{
  return (TF32*)&x;
}

CVector3::operator const TF32* ( ) const
{
  return (const TF32*)&x;
}

CVector3 &CVector3::operator+= ( const CVector3 &v )
{
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

CVector3 &CVector3::operator-= ( const CVector3 &v )
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

CVector3 &CVector3::operator*= ( const TF32 f )
{
  x *= f;
  y *= f;
  z *= f;
  return *this;
}

CVector3 &CVector3::operator/= ( const TF32 f )
{
  TF32 fi = 1 / f;
  x *= fi;
  y *= fi;
  z *= fi;
  return *this;
}

CVector3 CVector3::operator+ () const
{
  return *this;
}

CVector3 CVector3::operator- () const
{
  return CVector3( -x, -y, -z );
}

CVector3 CVector3::operator+ ( const CVector3 &v ) const
{
  return CVector3( x + v.x, y + v.y, z + v.z );
}

CVector3 CVector3::operator- ( const CVector3 &v ) const
{
  return CVector3( x - v.x, y - v.y, z - v.z );
}

CVector3 CVector3::operator* ( const TF32 f ) const
{
  return CVector3( x*f, y*f, z*f );
}

CVector3 CVector3::operator/ ( const TF32 f ) const
{
  TF32 fi = 1 / f;
  return CVector3( x*fi, y*fi, z*fi );
}

TBOOL CVector3::operator== ( const CVector3 &v ) const
{
  return x == v.x && y == v.y && z == v.z;
}

TBOOL CVector3::operator!= ( const CVector3 &v ) const
{
  return x != v.x || y != v.y || z != v.z;
}

TF32 CVector3::operator* ( const CVector3 &v ) const //dot product
{
  return x*v.x + y*v.y + z*v.z;
}

CVector3 CVector3::operator% ( const CVector3 &v ) const //cross product
{
  return CVector3( y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x );
}

TF32 const CVector4::operator[]( TS32 idx ) const
{
  return ( ( const TF32* )this )[ idx ];
}

TF32 &CVector4::operator[]( TS32 idx )
{
  return ( ( TF32* )this )[ idx ];
}

CVector4::operator TF32* ( )
{
  return (TF32*)&x;
}

CVector4::operator const TF32* ( ) const
{
  return (const TF32*)&x;
}

CVector4::operator CVector3 ()
{
  return CVector3( x, y, z );
}

CVector4::operator const CVector3() const
{
  return CVector3( x, y, z );
}

CVector4 &CVector4::operator+= ( const CVector4 &v )
{
  x += v.x;
  y += v.y;
  z += v.z;
  w += v.w;
  return *this;
}

CVector4 &CVector4::operator-= ( const CVector4 &v )
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  w -= v.w;
  return *this;
}

CVector4 &CVector4::operator*= ( const TF32 f )
{
  x *= f;
  y *= f;
  z *= f;
  w *= f;
  return *this;
}

CVector4 &CVector4::operator/= ( const TF32 f )
{
  TF32 fi = 1 / f;
  x *= fi;
  y *= fi;
  z *= fi;
  w *= fi;
  return *this;
}

CVector4 CVector4::operator+ () const
{
  return *this;
}

CVector4 CVector4::operator- () const
{
  return CVector4( -x, -y, -z, -w );
}

CVector4 CVector4::operator+ ( const CVector4 &v ) const
{
  return CVector4( x + v.x, y + v.y, z + v.z, w + v.w );
}

CVector4 CVector4::operator- ( const CVector4 &v ) const
{
  return CVector4( x - v.x, y - v.y, z - v.z, w - v.w );
}

CVector4 CVector4::operator* ( const TF32 f ) const
{
  return CVector4( x*f, y*f, z*f, w*f );
}

CVector4 CVector4::operator/ ( const TF32 f ) const
{
  TF32 fi = 1 / f;
  return CVector4( x*fi, y*fi, z*fi, w*fi );
}

TBOOL CVector4::operator== ( const CVector4 &v ) const
{
  return x == v.x && y == v.y && z == v.z && w == v.w;
}

TBOOL CVector4::operator!= ( const CVector4 &v ) const
{
  return x != v.x || y != v.y || z != v.z || w != v.w;
}

TF32 CVector4::operator* ( const CVector4 &v ) const //dot product
{
  return x*v.x + y*v.y + z*v.z + w*v.w;
}
