#pragma once

struct EnumNamePair
{
  TS32 Value;
  const TCHAR *Name;
};

TBOOL FindEnumByName( EnumNamePair *Pairs, TCHAR *Name, TS32 &Result );
TBOOL FindEnumByName( EnumNamePair *Pairs, CString &Name, TS32 &Result );
const TCHAR *FindNameByEnum( EnumNamePair *Pairs, TS32 Enum );