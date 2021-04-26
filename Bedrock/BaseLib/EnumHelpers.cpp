#include "BaseLib.h"


TBOOL FindEnumByName( EnumNamePair *Pairs, TCHAR *Name, TS32 &Result )
{
  TS32 r = -1;
  CString s = Name;
  for ( TS32 x = 0; Pairs[ x ].Name; x++ )
  {
    if ( Name == Pairs[ x ].Name )
    {
      Result = Pairs[ x ].Value;
      return true;
    }
  }
  return false;
}

TBOOL FindEnumByName( EnumNamePair *Pairs, CString &Name, TS32 &Result )
{
  return FindEnumByName( Pairs, Name.GetPointer(), Result );
}

const TCHAR *FindNameByEnum( EnumNamePair *Pairs, TS32 Enum )
{
  TCHAR *Name = NULL;
  for ( TS32 x = 0; Pairs[ x ].Name; x++ )
  {
    if ( Enum == Pairs[ x ].Value )
      return Pairs[ x ].Name;
  }
  return Name;
}