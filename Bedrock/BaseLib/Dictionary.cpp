#include "BaseLib.h"

TU32 DictionaryHash( const TS32 &i )
{
  return i;
}

TU32 DictionaryHash( const void *i ) //hash for a pointer
{
  TS64 v = (TS64)i;

  TS32 c;

  //djb2 hash
  TU32 Hash = 5381;
  while ( c = v & 0xff )
  {
    Hash = ( ( Hash << 5 ) + Hash ) + c; // hash * 33 + c
    v = v >> 8;
  }
  return Hash;

}