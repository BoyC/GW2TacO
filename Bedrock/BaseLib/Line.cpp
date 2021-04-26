#include "BaseLib.h"


TF32 CLine2D::GetProjectionT( const CVector2 &v ) const
{
  return Direction*( v - Point ) / ( Direction*Direction );
}

CVector2 CLine2D::Project( const CVector2 &v ) const
{
  return GetPoint( GetProjectionT( v ) );
}

TF32 CLine2D::Distance( const CVector2 &v ) const
{
  return ( Project( v ) - v ).Length();
}

CVector2 CLine2D::GetPoint( TF32 t ) const
{
  return Point + Direction*t;
}

void CLine2D::Normalize()
{
  Direction.Normalize();
}

void CLine2D::CreateFromPoints( const CVector2 &p1, const CVector2 &p2 )
{
  Point = p1;
  Direction = p2 - p1;
}

CLine2D::CLine2D( const CVector2 &p, const CVector2 &dir )
{
  Point = p;
  Direction = dir;
}

CLine2D::CLine2D()
{

}

TF32 CLine::GetProjectionT( const CVector3 &v ) const
{
  return ( Direction*( v - Point ) ) / Direction.Length();
}

CVector3 CLine::Project( const CVector3 &v ) const
{
  return GetPoint( GetProjectionT( v ) );
}

TF32 CLine::Distance( const CVector3 &v ) const
{
  return ( Project( v ) - v ).Length();
}

CVector3 CLine::GetPoint( TF32 t ) const
{
  return Point + Direction*t;
}

void CLine::Normalize()
{
  Direction.Normalize();
}

void CLine::CreateFromPoints( const CVector3 &p1, const CVector3 &p2 )
{
  Point = p1;
  Direction = p2 - p1;
}

CLine::CLine( const CVector3 &p, const CVector3 &dir )
{
  Point = p;
  Direction = dir;
}

CLine::CLine()
{

}

TF32 linesegment::Distance( const CVector3 &v ) const
{
  TF32 t = GetProjectionT( v );
  t = t < 0 ? 0 : t;
  t = t > 1 ? 1 : t;
  return ( ( Point + Direction*t ) - v ).Length();
}

linesegment::linesegment( const CVector3 &p1, const CVector3 &p2 )
{
  Point = p1;
  Direction = p2 - p1;
}
