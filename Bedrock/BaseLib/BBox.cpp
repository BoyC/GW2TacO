#include "BaseLib.h"

CBBox::CBBox()
{
  lo.x = 1;
  hi.x = -1;
}

CBBox::CBBox( const CBBox &aabb )
{
  lo = aabb.lo;
  hi = aabb.hi;
}

CBBox::CBBox( const CVector3 &v )
{
  lo = hi = v;
}

CBBox::CBBox( const CVector3 &min, const CVector3 &max )
{
  lo.x = 1;
  hi.x = -1;
  Expand( min );
  Expand( max );
}

TBOOL CBBox::isEmpty()
{
  return lo.x > hi.x;
}

void CBBox::Expand( const CVector3 &v )
{
  if ( isEmpty() )
  {
    lo = hi = v;
    return;
  }

  for ( TS32 i = 0; i < 3; i++ )
  {
    lo[ i ] = lo[ i ] < v[ i ] ? lo[ i ] : v[ i ];
    hi[ i ] = hi[ i ] > v[ i ] ? hi[ i ] : v[ i ];
  }
}

void CBBox::Expand( const CBBox &v )
{
  Expand( v.lo );
  Expand( v.hi );
}

CBBox CBBox::operator+( const CVector3 &v ) const
{
  CBBox aabb( *this );
  aabb.Expand( v );
  return aabb;
}

CBBox &CBBox::operator+=( const CVector3 &v )
{
  Expand( v );
  return *this;
}

CBBox CBBox::operator+( const CBBox &v ) const
{
  CBBox aabb( *this );
  aabb.Expand( v );
  return aabb;
}

CBBox &CBBox::operator+=( const CBBox &v )
{
  Expand( v );
  return *this;
}

TBOOL CBBox::Intersect( const CLine &ln, TF32 &tmin, TF32 &tmax )
{
  CVector3 invdir = CVector3( 1 / ln.Direction.x, 1 / ln.Direction.y, 1 / ln.Direction.z );
  TBOOL sign[ 3 ];
  sign[ 0 ] = ( invdir.x < 0 );
  sign[ 1 ] = ( invdir.y < 0 );
  sign[ 2 ] = ( invdir.z < 0 );

  CVector3 bounds[ 2 ];
  bounds[ 0 ] = lo;
  bounds[ 1 ] = hi;

  TF32 tymin, tymax, tzmin, tzmax;

  tmin = ( bounds[ sign[ 0 ] ].x - ln.Point.x ) * invdir.x;
  tmax = ( bounds[ 1 - sign[ 0 ] ].x - ln.Point.x ) * invdir.x;
  tymin = ( bounds[ sign[ 1 ] ].y - ln.Point.y ) * invdir.y;
  tymax = ( bounds[ 1 - sign[ 1 ] ].y - ln.Point.y ) * invdir.y;

  if ( ( tmin > tymax ) || ( tymin > tmax ) ) return false;

  if ( tymin > tmin ) tmin = tymin;
  if ( tymax < tmax ) tmax = tymax;

  tzmin = ( bounds[ sign[ 2 ] ].z - ln.Point.z ) * invdir.z;
  tzmax = ( bounds[ 1 - sign[ 2 ] ].z - ln.Point.z ) * invdir.z;

  if ( ( tmin > tzmax ) || ( tzmin > tmax ) ) return false;

  if ( tzmin > tmin ) tmin = tzmin;
  if ( tzmax < tmax ) tmax = tzmax;
  return true;
}

TBOOL CBBox::Intersect( const CPlane &pl )
{
  return pl.Distance( CVector3( pl.Normal.x >= 0 ? hi.x : lo.x,
                                pl.Normal.y >= 0 ? hi.y : lo.y,
                                pl.Normal.z >= 0 ? hi.z : lo.z ) ) < 0;
}
