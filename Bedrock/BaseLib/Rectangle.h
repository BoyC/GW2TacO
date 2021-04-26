#pragma once

class CRect
{
public:
  TS32 x1, y1, x2, y2;

  CRect();
  CRect( const TS32 a, const TS32 b, const TS32 c, const TS32 d );
  CRect( const CPoint p1, const CPoint p2 );
  CRect( const CRect &r );
  const TBOOL Contains( const TS32 x, const TS32 y ) const;
  const TBOOL Contains( CPoint&p ) const;
  const TBOOL Contains( const CPoint&p ) const;
  const TS32 Width() const;
  const TS32 Height() const;
  const TS32 Area() const;
  const TBOOL Intersects( const CRect &r ) const;

  CRect operator +( const CRect &a ) const;
  CRect operator -( const CRect &a ) const;
  CRect operator *( const TS32 a ) const;
  CRect &operator += ( const CRect &a ); //inflate by rect
  CRect operator+( const CPoint&p ) const;
  CRect &operator += ( const CPoint&p );
  CRect operator-( const CPoint&p ) const;
  CRect &operator -= ( const CPoint&p );
  const TBOOL operator==( const CRect &r ) const;
  const TBOOL operator!=( const CRect &r ) const;
  CRect operator|( const CRect&r )const;
  CRect &operator |= ( const CRect&r );
  CRect operator&( const CRect&r )const;
  CRect &operator &= ( const CRect&r );

  void Move( TS32 x, TS32 y );
  void Move( const CPoint&p );
  void MoveTo( TS32 x, TS32 y );
  void SetSize( TS32 x, TS32 y );
  void Normalize();
  CPoint TopLeft() const;
  CPoint BottomRight() const;
  CPoint TopRight() const;
  CPoint BottomLeft() const;
  CSize Size() const;
  CPoint Center() const;
  CRect GetCenterRect( CSize &s );
  CRect GetIntersection( const CRect&r )const;
};

bool IntervalIntersection( TS32 a1, TS32 a2, TS32 b1, TS32 b2 );
