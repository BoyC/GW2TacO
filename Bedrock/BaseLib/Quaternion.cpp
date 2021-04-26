#include "BaseLib.h"

CQuaternion Lerp( const CQuaternion &v1, const CQuaternion &v2, const TF32 t )
{
  return ( ( v2 - v1 )*t + v1 ).Normalized();
}

CQuaternion Slerp( const CQuaternion &v1, const CQuaternion &v2, const TF32 t )
{
  TF32 multiplier = 1;
  TF32 dot = CQuaternion::Dot( v1, v2 );

  if ( dot < 0 ) //v1->v2 angle>90 degrees, invert one to reduce spin
    multiplier = -1;

  TF32 angle = acosf( dot*multiplier );
  return ( v1*sinf( angle*( 1 - t ) ) + v2*multiplier*sinf( angle*t ) ) / sinf( angle );
}

CQuaternion SlerpFast( const CQuaternion &v1, const CQuaternion &v2, const TF32 t )
{
  TF32 multiplier = 1;
  TF32 dot = CQuaternion::Dot( v1, v2 );

  if ( dot < 0 ) //v1->v2 angle>90 degrees, invert one to reduce spin
    multiplier = -1;

  return Lerp( v1, v2*multiplier, t );
}

CQuaternion Squad( const CQuaternion &q1, const CQuaternion &S1, const CQuaternion &S2, const CQuaternion &q2, const TF32 t )
{
  CQuaternion c = Slerp( q1, q2, t );
  CQuaternion d = Slerp( S1, S2, t );
  return Slerp( c, d, 2 * t*( 1 - t ) );
}

INLINE CQuaternion SquadC2Point( const CQuaternion &q0, const CQuaternion &q1, const CQuaternion &q2 )
{
  CQuaternion &invq1 = q1.Inverted();
  return q1*( ( ( invq1*q2 ).Log() + ( invq1*q0 ).Log() )*-0.25f ).Exp();
}

void SquadSetup( CQuaternion &OutA, CQuaternion &OutB, CQuaternion &OutC, const CQuaternion &Q0, const CQuaternion &Q1, const CQuaternion &Q2, const CQuaternion &Q3 )
{
  const CQuaternion &q0 = CQuaternion::Dot( Q0, Q1 ) < 0 ? -Q0 : Q0;
  OutC = CQuaternion::Dot( Q1, Q2 ) < 0 ? -Q2 : Q2;
  const CQuaternion &q3 = CQuaternion::Dot( Q2, Q3 ) < 0 ? -Q3 : Q3;

  OutA = SquadC2Point( q0, Q1, OutC );
  OutB = SquadC2Point( Q1, OutC, q3 );
}

TF32 CQuaternion::Dot( const CQuaternion &v1, const CQuaternion &v2 )
{
  return v1.s*v2.s + v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

CQuaternion CQuaternion::Exp() const
{
  TF32 vlen = sqrtf( x*x + y*y + z*z );

  if ( vlen > 0 )
  {
    TF32 t = sinf( vlen ) / vlen;
    return CQuaternion( x*t, y*t, z*t, cosf( vlen ) );
  }
  else
    return CQuaternion( 0, 0, 0, cosf( vlen ) );
}

CQuaternion CQuaternion::Log() const
{
  TF32 angle = acosf( s );
  TF32 sina = sinf( angle );

  if ( sina > 0 )
  {
    TF32 t = angle / sina;
    return CQuaternion( x*t, y*t, z*t, 0 );
  }
  else
    return CQuaternion( 0, 0, 0, 0 );
}

void CQuaternion::Normalize()
{
  *this = Normalized();
}

CQuaternion CQuaternion::Inverted() const
{
  return CQuaternion( -x, -y, -z, s );
}

void CQuaternion::Invert()
{
  *this = Inverted();
}

void CQuaternion::Conjugate()
{
  x *= -1;
  y *= -1;
  z *= -1;
}

CQuaternion CQuaternion::Normalized() const
{
  return *this*InvSqrt( LengthSquared() );
}

TF32 CQuaternion::LengthSquared() const
{
  return x*x + y*y + z*z + s*s;
}

TF32 CQuaternion::Length() const
{
  return sqrtf( LengthSquared() );
}

CQuaternion::CQuaternion( const CVector3 &v ) //from euler angles
{
  FromEuler( v.x, v.y, v.z );
}

CQuaternion::CQuaternion( const TF32 _x, const TF32 _y, const TF32 _z ) //from euler angles
{
  FromEuler( _x, _y, _z );
}

CQuaternion::CQuaternion( const CQuaternion &v )
{
  x = v.x;
  y = v.y;
  z = v.z;
  s = v.s;
}

CQuaternion::CQuaternion( const TF32* v )
{
  x = v[ 0 ];
  y = v[ 1 ];
  z = v[ 2 ];
  s = v[ 3 ];
}

CQuaternion::CQuaternion( const TF32 _s, const CVector3 &v )
{
  x = v.x;
  y = v.y;
  z = v.z;
  s = _s;
}

CQuaternion::CQuaternion( const TF32 _x, const TF32 _y, const TF32 _z, const TF32 _s )
{
  x = _x;
  y = _y;
  z = _z;
  s = _s;
}

CQuaternion::CQuaternion()
{

}

void CQuaternion::FromRotationMatrix( const CMatrix4x4 &m )
{
  TF32 diag = m( 0, 0 ) + m( 1, 1 ) + m( 2, 2 ) + 1.0f;
  if ( diag > 1.0f )
  {
    TF32 sqd = sqrtf( diag );
    x = ( m( 1, 2 ) - m( 2, 1 ) ) / ( 2.0f*sqd );
    y = ( m( 2, 0 ) - m( 0, 2 ) ) / ( 2.0f*sqd );
    z = ( m( 0, 1 ) - m( 1, 0 ) ) / ( 2.0f*sqd );
    s = sqd / 2.0f;
    return;
  }

  TS32 maxdiagidx = 0;
  TF32 maxdiag = m( 0, 0 );
  for ( TS32 i = 1; i < 3; i++ )
  {
    if ( m( i, i ) > maxdiag )
    {
      maxdiagidx = i;
      maxdiag = m( i, i );
    }
  }

  TF32 S;

  switch ( maxdiagidx )
  {
  case 0:
    S = 2.0f*sqrtf( 1.0f + m( 0, 0 ) - m( 1, 1 ) - m( 2, 2 ) );
    x = 0.25f*S;
    y = ( m( 0, 1 ) + m( 1, 0 ) ) / S;
    z = ( m( 0, 2 ) + m( 2, 0 ) ) / S;
    s = ( m( 1, 2 ) - m( 2, 1 ) ) / S;
    break;

  case 1:
    S = 2.0f*sqrtf( 1.0f + m( 1, 1 ) - m( 0, 0 ) - m( 2, 2 ) );
    x = ( m( 0, 1 ) + m( 1, 0 ) ) / S;
    y = 0.25f*S;
    z = ( m( 1, 2 ) + m( 2, 1 ) ) / S;
    s = ( m( 2, 0 ) - m( 0, 2 ) ) / S;
    break;

  case 2:
    S = 2.0f*sqrtf( 1.0f + m( 2, 2 ) - m( 0, 0 ) - m( 1, 1 ) );
    x = ( m( 0, 2 ) + m( 2, 0 ) ) / S;
    y = ( m( 1, 2 ) + m( 2, 1 ) ) / S;
    z = 0.25f*S;
    s = ( m( 0, 1 ) - m( 1, 0 ) ) / S;
    break;
  }
}

void CQuaternion::ToAxisAngle( CVector3 &Axis, TF32 &Angle ) const
{
  Angle = acosf( s ) / 2.0f;
  Axis = CVector3( x, y, z ) / sinf( Angle*2.0f );
}

CQuaternion CQuaternion::FromAxisAngle( const CVector3 &Axis, const TF32 Angle )
{
  return CQuaternion( cosf( Angle / 2.0f ), Axis*sinf( Angle / 2.0f ) );
}

CVector3 CQuaternion::ToEuler() const
{
  const TF32 sqx = x*x;
  const TF32 sqy = y*y;
  const TF32 sqz = z*z;

  return CVector3( atan2f( 2.0f*( z*y + x*s ), 1 - 2 * ( sqx + sqy ) ),
                   asinf( -2.0f*( x*z - y*s ) ),
                   atan2f( 2.0f*( x*y + z*s ), 1 - 2 * ( sqy + sqz ) ) );
}

void CQuaternion::FromEuler( const TF32 _x, const TF32 _y, const TF32 _z )
{
  const TF32 cos_z = cosf( 0.5f*_z );
  const TF32 cos_y = cosf( 0.5f*_y );
  const TF32 cos_x = cosf( 0.5f*_x );

  const TF32 sin_z = sinf( 0.5f*_z );
  const TF32 sin_y = sinf( 0.5f*_y );
  const TF32 sin_x = sinf( 0.5f*_x );

  s = cos_z*cos_y*cos_x + sin_z*sin_y*sin_x;
  x = cos_z*cos_y*sin_x - sin_z*sin_y*cos_x;
  y = cos_z*sin_y*cos_x + sin_z*cos_y*sin_x;
  z = sin_z*cos_y*cos_x - cos_z*sin_y*sin_x;
}

TF32 const CQuaternion::operator[]( TS32 idx ) const
{
  return ( ( const TF32* )this )[ idx ];
}

TF32 &CQuaternion::operator[]( TS32 idx )
{
  return ( ( TF32* )this )[ idx ];
}

CQuaternion::operator TF32* ( )
{
  return (TF32*)&x;
}

CQuaternion::operator const TF32* ( ) const
{
  return (const TF32*)&x;
}

CQuaternion &CQuaternion::operator= ( const CQuaternion &q )
{
  s = q.s;
  x = q.x;
  y = q.y;
  z = q.z;
  return *this;
}

CQuaternion &CQuaternion::operator+= ( const CQuaternion &v )
{
  x += v.x;
  y += v.y;
  z += v.z;
  s += v.s;
  return *this;
}

CQuaternion &CQuaternion::operator-= ( const CQuaternion &v )
{
  x -= v.x;
  y -= v.y;
  z -= v.z;
  s -= v.s;
  return *this;
}

const CQuaternion &CQuaternion::operator*= ( const CQuaternion &v )
{
  return *this = *this*v;
}

CQuaternion &CQuaternion::operator*= ( const TF32 f )
{
  x *= f;
  y *= f;
  z *= f;
  s *= f;
  return *this;
}

CQuaternion &CQuaternion::operator/= ( const TF32 f )
{
  TF32 fi = 1 / f;
  x *= fi;
  y *= fi;
  z *= fi;
  s *= fi;
  return *this;
}

CQuaternion CQuaternion::operator+ () const
{
  return *this;
}

CQuaternion CQuaternion::operator- () const
{
  return CQuaternion( -x, -y, -z, -s );
}

CQuaternion CQuaternion::operator+ ( const CQuaternion &v ) const
{
  return CQuaternion( x + v.x, y + v.y, z + v.z, s + v.s );
}

CQuaternion CQuaternion::operator- ( const CQuaternion &v ) const
{
  return CQuaternion( x - v.x, y - v.y, z - v.z, s - v.s );
}

CQuaternion CQuaternion::operator* ( const TF32 f ) const
{
  return CQuaternion( x*f, y*f, z*f, s*f );
}

CQuaternion CQuaternion::operator/ ( const TF32 f ) const
{
  TF32 fi = 1 / f;
  return CQuaternion( x*fi, y*fi, z*fi, s*fi );
}

TBOOL CQuaternion::operator== ( const CQuaternion &v ) const
{
  return x == v.x && y == v.y && z == v.z && s == v.s;
}

TBOOL CQuaternion::operator!= ( const CQuaternion &v ) const
{
  return x != v.x || y != v.y || z != v.z || s != v.s;
}

CQuaternion CQuaternion::operator* ( const CQuaternion &q ) const
{
  return CQuaternion( y*q.z - z*q.y + s*q.x + x*q.s,
                      z*q.x - x*q.z + s*q.y + y*q.s,
                      x*q.y - y*q.x + s*q.z + z*q.s,
                      s*q.s - x*q.x - y*q.y - z*q.z );
}
