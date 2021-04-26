#include "BaseLib.h"

TU32 Hash_DJB2(TU8 *Data, TS32 Size)
{
	if (!Data || !Size) return 0;

	TU32 Hash = 5381;

	for (TS32 x=0; x<Size; x++)
		Hash = ((Hash << 5) + Hash) + *Data++;

	return Hash;
}