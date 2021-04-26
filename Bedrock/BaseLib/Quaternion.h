#pragma once

class CQuaternion
{
public:

  TF32 x, y, z, s;

  CQuaternion();
  CQuaternion( const TF32 _x, const TF32 _y, const TF32 _z, const TF32 _s );
  CQuaternion( const TF32 _s, const CVector3 &v );
  CQuaternion( const TF32* v );
  CQuaternion( const CQuaternion &v );
  CQuaternion( const TF32 _x, const TF32 _y, const TF32 _z ); //from euler angles
  CQuaternion( const CVector3 &v ); //from euler angles

  void FromEuler( const TF32 _x, const TF32 _y, const TF32 _z );
  CVector3 ToEuler() const;
  static CQuaternion FromAxisAngle( const CVector3 &Axis, const TF32 Angle );
  void ToAxisAngle( CVector3 &Axis, TF32 &Angle ) const;
  void FromRotationMatrix( const CMatrix4x4 &m );

  TF32 const operator[]( TS32 idx ) const;
  TF32 &operator[]( TS32 idx );
  operator TF32* ( );
  operator const TF32* ( ) const;
  CQuaternion &operator= ( const CQuaternion &q );
  CQuaternion &operator+= ( const CQuaternion &v );
  CQuaternion &operator-= ( const CQuaternion &v );
  const CQuaternion &operator*= ( const CQuaternion &v );
  CQuaternion &operator*= ( const TF32 f );
  CQuaternion &operator/= ( const TF32 f );
  CQuaternion operator+ () const;
  CQuaternion operator- () const;
  CQuaternion operator+ ( const CQuaternion &v ) const;
  CQuaternion operator- ( const CQuaternion &v ) const;
  CQuaternion operator* ( const TF32 f ) const;
  CQuaternion operator/ ( const TF32 f ) const;
  TBOOL operator== ( const CQuaternion &v ) const;
  TBOOL operator!= ( const CQuaternion &v ) const;
  CQuaternion operator* ( const CQuaternion &q ) const;
  TF32 Length() const;
  TF32 LengthSquared() const;
  CQuaternion Normalized() const;
  void Conjugate();
  void Invert();
  CQuaternion Inverted() const;
  void Normalize();
  CQuaternion Log() const;
  CQuaternion Exp() const;
  static TF32 Dot( const CQuaternion &v1, const CQuaternion &v2 );
};

CQuaternion Lerp( const CQuaternion &v1, const CQuaternion &v2, const TF32 t );
CQuaternion Slerp( const CQuaternion &v1, const CQuaternion &v2, const TF32 t );
CQuaternion SlerpFast( const CQuaternion &v1, const CQuaternion &v2, const TF32 t );
CQuaternion Squad( const CQuaternion &q1, const CQuaternion &S1, const CQuaternion &S2, const CQuaternion &q2, const TF32 t );
void SquadSetup( CQuaternion &OutA, CQuaternion &OutB, CQuaternion &OutC, const CQuaternion &Q0, const CQuaternion &Q1, const CQuaternion &Q2, const CQuaternion &Q3 );
