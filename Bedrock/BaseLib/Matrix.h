#pragma once

class CQuaternion;

class CMatrix2x2
{
  union
  {
    struct
    {
      TF32 _11, _12;
      TF32 _21, _22;
    };
    TF32 m[ 2 ][ 2 ];
  };

public:

  CMatrix2x2();
  CMatrix2x2( const TF32 *f );
  CMatrix2x2( const CMatrix2x2 &mx );
  CMatrix2x2( TF32 f11, TF32 f12,
              TF32 f21, TF32 f22 );

  CMatrix2x2( const CQuaternion &q );

  TF32 &operator() ( TU32 Row, TU32 Col );
  TF32 operator() ( TU32 Row, TU32 Col ) const;
  operator TF32*( );
  operator const TF32*( ) const;

  CMatrix2x2 &operator*= ( const CMatrix2x2 &mat );
  CMatrix2x2 &operator+= ( const CMatrix2x2 &mat );
  CMatrix2x2 &operator-= ( const CMatrix2x2 &mat );
  CMatrix2x2 &operator*= ( const TF32 f );
  CMatrix2x2 &operator/= ( const TF32 f );
  CMatrix2x2 operator+ () const;
  CMatrix2x2 operator- () const;
  CMatrix2x2 operator *( const CMatrix2x2& mat ) const;
  CMatrix2x2 operator +( const CMatrix2x2& mat ) const;
  CMatrix2x2 operator -( const CMatrix2x2& mat ) const;
  CMatrix2x2 operator *( const TF32 f ) const;
  CMatrix2x2 operator /( const TF32 f ) const;
  friend CMatrix2x2 operator *( const TF32 f, const CMatrix2x2& mat );
  TBOOL operator ==( const CMatrix2x2& mat ) const;
  TBOOL operator !=( const CMatrix2x2& mat ) const;

  void Transpose();
  CMatrix2x2 Transposed() const;
  CMatrix2x2 &SetIdentity();
  CVector2 Apply( const CVector2 &v ) const;
  void Invert();
  CMatrix2x2 Inverted() const;
};


class CMatrix3x3
{
  union
  {
    struct
    {
      TF32 _11, _12, _13;
      TF32 _21, _22, _23;
      TF32 _31, _32, _33;
    };
    TF32 m[ 3 ][ 3 ];
  };

public:

  CMatrix3x3();;
  CMatrix3x3( const TF32 *f );
  CMatrix3x3( const CMatrix3x3 &mx );
  CMatrix3x3( TF32 f11, TF32 f12, TF32 f13,
              TF32 f21, TF32 f22, TF32 f23,
              TF32 f31, TF32 f32, TF32 f33 );
  CMatrix3x3( const CQuaternion &q );

  TF32 &operator() ( TU32 Row, TU32 Col );
  TF32 operator() ( TU32 Row, TU32 Col ) const;
  operator TF32*( );
  operator const TF32*( ) const;
  CMatrix3x3 &operator*= ( const CMatrix3x3 &mat );
  CMatrix3x3 &operator+= ( const CMatrix3x3 &mat );
  CMatrix3x3 &operator-= ( const CMatrix3x3 &mat );
  CMatrix3x3 &operator*= ( const TF32 f );
  CMatrix3x3 &operator/= ( const TF32 f );
  CMatrix3x3 operator+ () const;
  CMatrix3x3 operator- () const;
  CMatrix3x3 operator *( const CMatrix3x3& mat ) const;
  CMatrix3x3 operator +( const CMatrix3x3& mat ) const;
  CMatrix3x3 operator -( const CMatrix3x3& mat ) const;
  CMatrix3x3 operator *( const TF32 f ) const;
  CMatrix3x3 operator /( const TF32 f ) const;
  friend CMatrix3x3 operator *( const TF32 f, const CMatrix3x3& mat );
  TBOOL operator ==( const CMatrix3x3& mat ) const;
  TBOOL operator !=( const CMatrix3x3& mat ) const;
  void Transpose();
  CMatrix3x3 Transposed() const;
  CMatrix3x3 &SetIdentity();
  CVector3 Apply( const CVector3 &v ) const;
};

class CMatrix4x4
{
  union
  {
    struct
    {
      TF32 _11, _12, _13, _14;
      TF32 _21, _22, _23, _24;
      TF32 _31, _32, _33, _34;
      TF32 _41, _42, _43, _44;
    };
    TF32 m[ 4 ][ 4 ];
  };

public:

  CMatrix4x4();;
  CMatrix4x4( const TF32 *f );
  CMatrix4x4( const CMatrix4x4 &mx );
  CMatrix4x4( TF32 f11, TF32 f12, TF32 f13, TF32 f14,
              TF32 f21, TF32 f22, TF32 f23, TF32 f24,
              TF32 f31, TF32 f32, TF32 f33, TF32 f34,
              TF32 f41, TF32 f42, TF32 f43, TF32 f44 );
  CMatrix4x4( const CQuaternion &q );

  TF32 &operator() ( TU32 Row, TU32 Col );
  TF32 operator() ( TU32 Row, TU32 Col ) const;
  operator TF32*( );
  operator const TF32*( ) const;
  CVector4 Row( TS32 x ) const;
  CVector4 Col( TS32 x ) const;
  CMatrix4x4 &operator*= ( const CMatrix4x4 &mat );
  CMatrix4x4 &operator+= ( const CMatrix4x4 &mat );
  CMatrix4x4 &operator-= ( const CMatrix4x4 &mat );
  CMatrix4x4 &operator*= ( const TF32 f );
  CMatrix4x4 &operator/= ( const TF32 f );
  CMatrix4x4 operator+ () const;
  CMatrix4x4 operator- () const;
  CMatrix4x4 operator *( const CMatrix4x4& mat ) const;
  CMatrix4x4 operator +( const CMatrix4x4& mat ) const;
  CMatrix4x4 operator -( const CMatrix4x4& mat ) const;
  CMatrix4x4 operator *( const TF32 f ) const;
  CMatrix4x4 operator /( const TF32 f ) const;
  friend CMatrix4x4 operator *( const TF32 f, const CMatrix4x4& mat );
  TBOOL operator ==( const CMatrix4x4& mat ) const;
  TBOOL operator !=( const CMatrix4x4& mat ) const;
  void Transpose();
  CMatrix4x4 Transposed() const;
  CMatrix4x4 &SetIdentity();
  CVector4 Apply( const CVector3 &v ) const;
  CVector4 Apply( const CVector4 &v ) const;
  void Decompose( CVector3 &Scale, CQuaternion &Rotation, CVector3 &Translation ) const;
  TF32 Determinant() const;
  void Invert();
  CMatrix4x4 Inverted() const;
  void SetLookAtLH( const CVector3 &Eye, const CVector3 &Target, const CVector3 &Up );
  void SetLookAtRH( const CVector3 &Eye, const CVector3 &Target, const CVector3 &Up );
  void SetOrthoLH( const TF32 w, const TF32 h, const TF32 zn, const TF32 zf );
  void SetOrthoRH( const TF32 w, const TF32 h, const TF32 zn, const TF32 zf );
  void SetPerspectiveFovLH( const TF32 fovy, const TF32 aspect, const TF32 zn, const TF32 zf );
  void SetPerspectiveFovRH( const TF32 fovy, const TF32 aspect, const TF32 zn, const TF32 zf );
  static CMatrix4x4 CMatrix4x4::Translation( const CVector3 &v );
  static CMatrix4x4 CMatrix4x4::Scaling( const CVector3 &v );
  static CMatrix4x4 CMatrix4x4::Rotation( const CQuaternion &q );
  CVector3 GetTranslation() const;
  CVector3 GetScaling() const;
  CQuaternion GetRotation() const;
  void SetTransformation( const CVector3 &scaling, const CQuaternion &rotation, const CVector3 &translation );
};