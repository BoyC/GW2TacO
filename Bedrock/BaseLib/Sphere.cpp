#include "BaseLib.h"

const TBOOL CSphere::Intersect( const CLine &l, TF32 &tmin, TF32 &tmax ) const
{
  CVector3 oc = l.Point - Position;

  TF32 a = l.Direction*l.Direction;
  TF32 b = 2 * ( l.Direction*oc );
  TF32 c = oc*oc - Radius*Radius;

  TF32 det = b*b - 4 * a*c;
  if ( det < 0 ) return false;
  det = sqrtf( det );
  TF32 t1 = ( -b - det ) / ( 2 * a );
  TF32 t2 = ( -b + det ) / ( 2 * a );
  tmin = min( t1, t2 );
  tmax = max( t1, t2 );

  return true;// l.Distance(Position) < Radius;
}

const TBOOL CSphere::Intersect( const CPlane &p ) const
{
  return abs( p.Distance( Position ) ) < Radius;
}

CSphere::CSphere( const CVector3 &p, const TF32 r )
{
  Position = p;
  Radius = r;
}

CSphere::CSphere()
{

}

CSphere::~CSphere()
{

}

const TBOOL CSphere::Contains( const CVector3 &p ) const
{
  return ( Position - p ).Length() < Radius;
}
