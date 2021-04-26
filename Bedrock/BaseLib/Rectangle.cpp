#include "BaseLib.h"

bool IntervalIntersection( TS32 a1, TS32 a2, TS32 b1, TS32 b2 )
{
  TS32 _a1 = min( a1, a2 );
  TS32 _a2 = max( a1, a2 );
  TS32 _b1 = min( b1, b2 );
  TS32 _b2 = max( b1, b2 );

  return  !( ( a1 < b1 && a2 < b1 ) || ( a1 > b2 && a2 > b2 ) );
}

CRect CRect::GetIntersection( const CRect&r ) const
{
  return CRect( x1 > r.x1 ? x1 : r.x1,
                y1 > r.y1 ? y1 : r.y1,
                x2 < r.x2 ? x2 : r.x2,
                y2 < r.y2 ? y2 : r.y2 );
}

CRect CRect::GetCenterRect( CSize &s )
{
  return CRect( Center().x - s.x / 2, Center().y - s.y / 2, Center().x - s.x / 2 + s.x, Center().y - s.y / 2 + s.y );
}

CPoint CRect::Center() const
{
  return CPoint( x1 + x2, y1 + y2 ) / 2;
}

CSize CRect::Size() const
{
  return CSize( Width(), Height() );
}

CPoint CRect::BottomLeft() const
{
  return CPoint( x1, y2 );
}

CPoint CRect::TopRight() const
{
  return CPoint( x2, y1 );
}

CPoint CRect::BottomRight() const
{
  return CPoint( x2, y2 );
}

CPoint CRect::TopLeft() const
{
  return CPoint( x1, y1 );
}

void CRect::Normalize()
{
  TS32 _x1 = x1;
  TS32 _y1 = y1;
  TS32 _x2 = x2;
  TS32 _y2 = y2;

  x1 = _x1 < _x2 ? _x1 : _x2;
  x2 = _x1 < _x2 ? _x2 : _x1;
  y1 = _y1 < _y2 ? _y1 : _y2;
  y2 = _y1 < _y2 ? _y2 : _y1;
}

void CRect::SetSize( TS32 x, TS32 y )
{
  x2 = x1 + x;
  y2 = y1 + y;
}

void CRect::MoveTo( TS32 x, TS32 y )
{
  x2 -= ( x1 - x );
  y2 -= ( y1 - y );
  x1 = x;
  y1 = y;
}

void CRect::Move( const CPoint&p )
{
  Move( p.x, p.y );
}

void CRect::Move( TS32 x, TS32 y )
{
  x1 += x;
  y1 += y;
  x2 += x;
  y2 += y;
}

const TBOOL CRect::Intersects( const CRect &r ) const
{
  return !( x2 <= r.x1 || x1 >= r.x2 ) && !( y2 <= r.y1 || y1 >= r.y2 );
}

const TS32 CRect::Area() const
{
  return Width()*Height();
}

const TS32 CRect::Height() const
{
  return y2 - y1;
}

const TS32 CRect::Width() const
{
  return x2 - x1;
}

const TBOOL CRect::Contains( const CPoint&p ) const
{
  return Contains( p.x, p.y );
}

const TBOOL CRect::Contains( CPoint&p ) const
{
  return Contains( p.x, p.y );
}

const TBOOL CRect::Contains( const TS32 x, const TS32 y ) const
{
  return x >= x1 && x < x2 && y >= y1 && y < y2;
}

CRect::CRect( const CRect &r )
{
  x1 = r.x1;
  y1 = r.y1;
  x2 = r.x2;
  y2 = r.y2;
}

CRect::CRect( const CPoint p1, const CPoint p2 )
{
  x1 = p1.x;
  y1 = p1.y;
  x2 = p2.x;
  y2 = p2.y;
}

CRect::CRect( const TS32 a, const TS32 b, const TS32 c, const TS32 d )
{
  x1 = a;
  y1 = b;
  x2 = c;
  y2 = d;
}

CRect::CRect()
{
  x1 = y1 = x2 = y2 = 0;
}

CRect CRect::operator +( const CRect &a ) const
{
  return CRect( x1 - a.x1, y1 - a.y1, x2 + a.x2, y2 + a.y2 );
}

CRect CRect::operator -( const CRect &a ) const
{
  return CRect( x1 + a.x1, y1 + a.y1, x2 - a.x2, y2 - a.y2 );
}

CRect CRect::operator *( const TS32 a ) const
{
  return CRect( x1*a, y1*a, x2*a, y2*a );
}

CRect &CRect::operator += ( const CRect &a ) //inflate by rect
{
  x1 -= a.x1;
  y1 -= a.y1;
  x2 += a.x2;
  y2 += a.y2;
  return *this;
}

CRect CRect::operator+( const CPoint&p ) const
{
  return CRect( x1 + p.x, y1 + p.y, x2 + p.x, y2 + p.y );
}

CRect &CRect::operator += ( const CPoint&p )
{
  Move( p );
  return *this;
}

CRect CRect::operator-( const CPoint&p ) const
{
  return CRect( x1 - p.x, y1 - p.y, x2 - p.x, y2 - p.y );
}

CRect &CRect::operator -= ( const CPoint&p )
{
  Move( -p );
  return *this;
}

const TBOOL CRect::operator==( const CRect &r ) const
{
  return x1 == r.x1 && y1 == r.y1 && x2 == r.x2 && y2 == r.y2;
}

const TBOOL CRect::operator!=( const CRect &r ) const
{
  return x1 != r.x1 || y1 != r.y1 || x2 != r.x2 || y2 != r.y2;
}

CRect CRect::operator|( const CRect&r )const
{
  if ( !Intersects( r ) ) return CRect( 1, 1, -1, -1 );
  return GetIntersection( r );
}

CRect &CRect::operator |= ( const CRect&r )
{
  *this = *this | r;
  return *this;
}

CRect CRect::operator&( const CRect&r )const
{
  return CRect( x1<r.x1 ? x1 : r.x1,
                y1<r.y1 ? y1 : r.y1,
                x2>r.x2 ? x2 : r.x2,
                y2>r.y2 ? y2 : r.y2 );
}

CRect &CRect::operator &= ( const CRect&r )
{
  *this = *this&r;
  return *this;
}
