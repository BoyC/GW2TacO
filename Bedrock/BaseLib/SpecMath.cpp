#include "BaseLib.h"

#ifdef FAST_INVSQRT
TF32 InvSqrt( TF32 x )
{
  TF32 xhalf = 0.5f*x;
  TS32 i = *(TS32*)&x; // get bits for TF32ing value
  i = 0x5f3759df - ( i >> 1 ); // gives initial guess y0
  x = *(TF32*)&i; // convert bits back to TF32
  x = x*( 1.5f - xhalf*x*x ); // Newton step, repeating increases accuracy
  return x;
}
#else
TF32 InvSqrt( TF32 x )
{
  return 1 / sqrtf( x );
}

TF32 DeGamma( TF32 c )
{
  TF32 cs;
  if ( c < 0.04045f ) cs = c / 12.92f;
  else cs = powf( ( c + 0.055f ) / 1.055f, 2.4f );
  return cs;
}

#endif