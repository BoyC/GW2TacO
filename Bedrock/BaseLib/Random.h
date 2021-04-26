#pragma once

#define RAND_MAX 0x7fff

class CRandom
{
  TU32 Seed;

public:

  CRandom();
  CRandom( TU32 seed );
  void srand( TU32 seed );
  TU32 rand();
  TF32 frand();
  TF32 frand( TF32 min, TF32 max );
};