#include "BaseLib.h"


TF32 CRandom::frand( TF32 min, TF32 max )
{
  return ( max - min )*frand() + min;
}

TF32 CRandom::frand()
{
  TS32 r = rand();
  return r / (TF32)RAND_MAX;
}

TU32 CRandom::rand()
{
  Seed = Seed * 214013L + 2531011L;
  return ( Seed >> 16 )&RAND_MAX;
}

void CRandom::srand( TU32 seed )
{
  Seed = seed;
}

CRandom::CRandom( TU32 seed )
{
  srand( seed );
}

CRandom::CRandom()
{
  srand( 0 );
}
