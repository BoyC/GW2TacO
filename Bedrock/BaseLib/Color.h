#pragma once

class CColor
{
  TU8 b, g, r, a;

public:

  CColor();
  CColor( const TU8 _r, const TU8 _g, const TU8 _b, const TU8 _a );
  CColor( const TU8* c );
  CColor( const TU32 argb );

  static CColor FromFloat( const TF32 _r, const TF32 _g, const TF32 _b, const TF32 _a );
  static CColor FromARGB( const TU32 v );
  static CColor FromABGR( const TU32 v );

  TU8 const operator[]( TS32 idx ) const;
  TU8 &operator[]( TS32 idx );
  operator TU8* ( );
  operator const TU8* ( ) const;
  TBOOL operator== ( const CColor &c ) const;
  TBOOL operator!= ( const CColor &c ) const;
  TU32 argb() const;

  operator TU32() const;
  TU8 &R();
  TU8 &G();
  TU8 &B();
  TU8 &A();
};

const CColor Lerp( const CColor a, const CColor b, const TF32 t );