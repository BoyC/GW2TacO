#pragma once

class CPlane
{
  void Normalize();

public:

  CVector3 Normal;
  TF32 D;

  CPlane();
  CPlane( const CVector3 &Point, const CVector3 &n );
  CPlane( const CVector3 &a, const CVector3 &b, const CVector3 &c );

  CVector3 Project( const CVector3 &v ) const;
  CLine Project( const CLine &l ) const;
  TF32 Distance( const CVector3 &v ) const;
  TS32 Side( const CVector3 &v ) const;
  CVector3 Intersect( const CLine &l ) const;

  //CLine Intersect(const CPlane &p) const
  //{

  //}

};