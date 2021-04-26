#pragma once

class CBBox
{
public:
  CVector3 lo, hi;

  CBBox();
  CBBox( const CBBox &aabb );
  CBBox( const CVector3 &v );
  CBBox( const CVector3 &min, const CVector3 &max );

  TBOOL isEmpty();
  void Expand( const CVector3 &v );
  void Expand( const CBBox &v );
  CBBox operator+( const CVector3 &v ) const;
  CBBox &operator+=( const CVector3 &v );
  CBBox operator+( const CBBox &v ) const;
  CBBox &operator+=( const CBBox &v );
  TBOOL Intersect( const CLine &ln, TF32 &tmin, TF32 &tmax );
  TBOOL Intersect( const CPlane &pl );
};