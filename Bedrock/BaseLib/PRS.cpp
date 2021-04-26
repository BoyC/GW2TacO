#include "BaseLib.h"


CVector3 CPRS::Apply( const CVector3 &v ) const
{
  return CVector3( v.x*Scale.x, v.y*Scale.y, v.z*Scale.z )*Rotation + Translation;
}

void CPRS::FromMatrix( const CMatrix4x4 &m )
{
  m.Decompose( Scale, Rotation, Translation );
}

CMatrix4x4 CPRS::ToMatrix()
{
  CMatrix4x4 m;
  m.SetTransformation( Scale, Rotation, Translation );
  return m;
}

CPRS::~CPRS()
{

}

CPRS::CPRS( const CMatrix4x4 &m )
{
  FromMatrix( m );
}

CPRS::CPRS( const CVector3 &scale, const CQuaternion &rotation, const CVector3 &translation )
{
  Scale = scale;
  Rotation = rotation;
  Translation = translation;
}

CPRS::CPRS()
{

}

CPRS &CPRS::operator+= ( const CPRS &v )
{
  Scale.x *= v.Scale.x;
  Scale.y *= v.Scale.y;
  Scale.z *= v.Scale.z;
  Rotation *= v.Rotation;
  Translation += v.Translation;
  return *this;
}

CPRS CPRS::operator+ ( const CPRS &v ) const
{
  return CPRS( CVector3( Scale.x*v.Scale.x, Scale.y*v.Scale.y, Scale.z*v.Scale.z ),
               Rotation*v.Rotation,
               Translation + v.Translation );
}

CPRS &CPRS::operator-= ( const CPRS &v )
{
  Scale.x /= v.Scale.x;
  Scale.y /= v.Scale.y;
  Scale.z /= v.Scale.z;
  Rotation *= v.Rotation.Inverted();
  Translation -= v.Translation;
  return *this;
}

CPRS CPRS::operator- ( const CPRS &v ) const
{
  return CPRS( CVector3( Scale.x / v.Scale.x, Scale.y / v.Scale.y, Scale.z / v.Scale.z ),
               Rotation*v.Rotation.Inverted(),
               Translation - v.Translation );
}


CPRS Lerp( const CPRS &v1, const CPRS &v2, TF32 t )
{
  return CPRS( Lerp( v1.Scale, v2.Scale, t ),
               Lerp( v1.Rotation, v2.Rotation, t ),
               Lerp( v1.Translation, v2.Translation, t ) );
}
