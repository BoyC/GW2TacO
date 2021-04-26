#pragma once

class CSphere
{
public:

  TF32 Radius;
  CVector3 Position;

  CSphere();
  ~CSphere();
  CSphere( const CVector3 &p, const TF32 r );

  const TBOOL Intersect( const CPlane &p ) const;
  const TBOOL Intersect( const CLine &l, TF32 &tmin, TF32 &tmax ) const;
  const TBOOL Contains( const CVector3 &p ) const;
};