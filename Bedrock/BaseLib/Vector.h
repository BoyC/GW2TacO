#pragma once

class CQuaternion;
class CMatrix3x3;
class CMatrix4x4;

class CVector2
{
public:

  TF32 x, y;

  CVector2();
  CVector2( const TF32 _x, const TF32 _y );
  CVector2( const TF32* v );
  CVector2( const CVector2 &v );

  TF32 const operator[]( TS32 idx ) const;
  TF32 &operator[]( TS32 idx );
  operator TF32* ( );
  operator const TF32* ( ) const;
  CVector2 &operator+= ( const CVector2 &v );
  CVector2 &operator-= ( const CVector2 &v );
  CVector2 &operator*= ( const TF32 f );
  CVector2 &operator/= ( const TF32 f );
  CVector2 operator+ () const;
  CVector2 operator- () const;
  CVector2 operator+ ( const CVector2 &v ) const;
  CVector2 operator- ( const CVector2 &v ) const;
  CVector2 operator* ( const TF32 f ) const;
  CVector2 operator/ ( const TF32 f ) const;
  TBOOL operator== ( const CVector2 &v ) const;
  TBOOL operator!= ( const CVector2 &v ) const;
  TF32 operator* ( const CVector2 &v ) const; //dot product

  CVector2 Rotated( const CVector2 &center, TF32 rotation );

  TF32 Length() const;
  TF32 LengthSquared() const;
  CVector2 Normalized() const;
  void Normalize();
  static TF32 Dot( const CVector2 &v1, const CVector2 &v2 );
};

class CVector2I
{
public:

  TS32 x, y;

  CVector2I();
  CVector2I( const TS32 _x, const TS32 _y );
  CVector2I( const TS32* v );
  CVector2I( const CVector2I &v );

  TS32 const operator[]( TS32 idx ) const;
  TS32 &operator[]( TS32 idx );
  operator TS32* ( );
  operator const TS32* ( ) const;
  CVector2I &operator+= ( const CVector2I &v );
  CVector2I &operator-= ( const CVector2I &v );
  CVector2I &operator*= ( const TF32 f );
  CVector2I &operator/= ( const TF32 f );
  CVector2I &operator/= ( const TS32 f );
  CVector2I operator+ () const;
  CVector2I operator- () const;
  CVector2I operator+ ( const CVector2I &v ) const;
  CVector2I operator- ( const CVector2I &v ) const;
  CVector2I operator* ( const TF32 f ) const;
  CVector2I operator/ ( const TF32 f ) const;
  CVector2I operator/ ( const TS32 f ) const;
  TBOOL operator== ( const CVector2I &v ) const;
  TBOOL operator!= ( const CVector2I &v ) const;
  TS32 operator* ( const CVector2I &v ) const; //dot product
  TF32 Length() const;
  TF32 LengthSquared() const;
  CVector2I Normalized() const;
  void Normalize();
  static TS32 Dot( const CVector2I &v1, const CVector2I &v2 );
};

typedef CVector2I CPoint;
typedef CVector2I CSize;
class CVector4;
class CPRS;

class CVector3
{
public:

  TF32 x, y, z;

  CVector3();
  CVector3( const TF32 _x, const TF32 _y, const TF32 _z );
  CVector3( const TF32* v );
  CVector3( const CVector3 &v );

  TF32 const operator[]( TS32 idx ) const;
  TF32 &operator[]( TS32 idx );
  operator TF32* ( );
  operator const TF32* ( ) const;
  CVector3 &operator+= ( const CVector3 &v );
  CVector3 &operator-= ( const CVector3 &v );
  CVector3 &operator*= ( const TF32 f );
  CVector3 &operator/= ( const TF32 f );
  CVector3 operator+ () const;
  CVector3 operator- () const;
  CVector3 operator+ ( const CVector3 &v ) const;
  CVector3 operator- ( const CVector3 &v ) const;
  CVector3 operator* ( const TF32 f ) const;
  CVector3 operator/ ( const TF32 f ) const;
  TBOOL operator== ( const CVector3 &v ) const;
  TBOOL operator!= ( const CVector3 &v ) const;
  TF32 operator* ( const CVector3 &v ) const; //dot product
  CVector3 operator% ( const CVector3 &v ) const; //cross product
  CVector3 operator* ( const CQuaternion &q ) const; //rotation
  CVector3 &operator*= ( const CQuaternion &q );     //rotation
  CVector3 operator/ ( const CQuaternion &q ) const; //reverse rotation
  CVector3 &operator/= ( const CQuaternion &q );     //reverse rotation
  CVector4 operator* ( const CMatrix4x4 &q ) const; //transformation
  CVector3 operator* ( const CMatrix3x3 &q ) const; //transformation
  CVector3 &operator*= ( const CMatrix3x3 &q );     //transformation
  CVector3 operator* ( const CPRS &q ) const; //transformation
  CVector3 &operator*= ( const CPRS &q );     //transformation

  TF32 Length() const;
  TF32 LengthSquared() const;
  CVector3 Normalized() const;
  void Normalize();
  static TF32 Dot( const CVector3 &v1, const CVector3 &v2 );
  static CVector3 Cross( const CVector3 &v1, const CVector3 &v2 );
};

class CVector4
{
public:

  TF32 x, y, z, w;

  CVector4();
  CVector4( const TF32 _x, const TF32 _y, const TF32 _z, const TF32 _w );
  CVector4( const TF32* v );
  CVector4( const CVector4 &v );

  TF32 const operator[]( TS32 idx ) const;
  TF32 &operator[]( TS32 idx );
  operator TF32* ( );
  operator const TF32* ( ) const;
  operator CVector3 ();
  operator const CVector3() const;
  CVector4 &operator+= ( const CVector4 &v );
  CVector4 &operator-= ( const CVector4 &v );
  CVector4 &operator*= ( const TF32 f );
  CVector4 &operator/= ( const TF32 f );
  CVector4 operator+ () const;
  CVector4 operator- () const;
  CVector4 operator+ ( const CVector4 &v ) const;
  CVector4 operator- ( const CVector4 &v ) const;
  CVector4 operator* ( const TF32 f ) const;
  CVector4 operator/ ( const TF32 f ) const;
  TBOOL operator== ( const CVector4 &v ) const;
  TBOOL operator!= ( const CVector4 &v ) const;
  TF32 operator* ( const CVector4 &v ) const; //dot product
  CVector4 operator* ( const CMatrix4x4 &q ) const; //transformation
  TF32 Length() const;
  TF32 LengthSquared() const;
  CVector4 Normalized() const;
  void Normalize();
  CVector4 Homogenized() const;
  void Homogenize();
  static TF32 Dot( const CVector4 &v1, const CVector4 &v2 );
  static CVector4 Cross( const CVector4 &v1, const CVector4 &v2, const CVector4 &v3 );
};


const TS32 Lerp( const TS32 v1, const TS32 v2, const TF32 t );
const TF32 Lerp( const TF32 v1, const TF32 v2, const TF32 t );
const TU32 Lerp( const TU32 v1, const TU32 v2, const TF32 t );
const TS64 Lerp( const TS64 v1, const TS64 v2, const TF32 t );
const TF64 Lerp( const TF64 v1, const TF64 v2, const TF32 t );
const TU64 Lerp( const TU64 v1, const TU64 v2, const TF32 t );
const CVector2 Lerp( const CVector2 &v1, const CVector2 &v2, const TF32 t );
const CVector2I Lerp( const CVector2I &v1, const CVector2I &v2, const TF32 t );
const CVector3 Lerp( const CVector3 &v1, const CVector3 &v2, const TF32 t );
const CVector4 Lerp( const CVector4 &v1, const CVector4 &v2, const TF32 t );
TF32 mod( TF32 v, TF32 m );
TS32 Mod( TS32 a, TS32 b );
TF32 Mod( TF32 a, TF32 b );
TF32 Mod( TF32 a, TS32 b );
