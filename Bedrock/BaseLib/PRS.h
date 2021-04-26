#pragma once

//translation rotation scale class

class CPRS
{
public:
  CVector3 Translation;
  CQuaternion Rotation;
  CVector3 Scale;

  CPRS();
  CPRS( const CVector3 &scale, const CQuaternion &rotation, const CVector3 &translation );
  CPRS( const CMatrix4x4 &m );
  virtual ~CPRS();

  CMatrix4x4 ToMatrix();
  void FromMatrix( const CMatrix4x4 &m );
  CPRS &operator+= ( const CPRS &v );
  CPRS operator+ ( const CPRS &v ) const;
  CPRS &operator-= ( const CPRS &v );
  CPRS operator- ( const CPRS &v ) const;
  CVector3 Apply( const CVector3 &v ) const;
};

CPRS Lerp( const CPRS &v1, const CPRS &v2, TF32 t );