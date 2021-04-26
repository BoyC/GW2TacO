#pragma once
#include "Atlas.h"

enum WBTEXTALIGNMENTX
{
  WBTA_CENTERX,
  WBTA_LEFT,
  WBTA_RIGHT,
};

enum WBTEXTALIGNMENTY
{
  WBTA_CENTERY,
  WBTA_TOP,
  WBTA_BOTTOM,
};

enum WBTEXTTRANSFORM
{
  WBTT_NONE = 0,
  WBTT_CAPITALIZE,
  WBTT_UPPERCASE,
  WBTT_LOWERCASE,
};

struct WBSYMBOLINPUT
{
  TU16 Char;
  CRect UV;
  CPoint Offset;
  TS32 Advance;
};

struct WBSYMBOL
{
  WBATLASHANDLE Handle;
  TS16 OffsetX, OffsetY;
  TU16 SizeX, SizeY;
  TS16 Advance;
  TU16 Char;

  CRect calculatedContentRect;
};

struct WBKERNINGDATA
{
  TU16 First, Second;
  TS16 Amount;
};

class CWBKerningPair
{
public:

  TU32 First, Second;

  INLINE TBOOL operator==( const CWBKerningPair &k );
  CWBKerningPair();
  CWBKerningPair( TU16 a, TU16 b );
};

INLINE TU32 DictionaryHash( const CWBKerningPair &i );

class CWBDrawAPI;

class CWBFontDescription
{
  friend class CWBFont;

  TU8 *Image;
  TS32 XRes, YRes;

  CArray<WBSYMBOLINPUT> Alphabet;
  CArray<WBKERNINGDATA> KerningData;

  TS32 LineHeight;
  TS32 Base;

public:

  CWBFontDescription();
  ~CWBFontDescription();

  TBOOL LoadBMFontBinary( TU8 *Binary, TS32 BinarySize, TU8 *Image, TS32 XRes, TS32 YRes, CArray<int>& enabledGlyphs = CArray<int>() ); //32 bit raw image data
  TBOOL LoadBMFontText( TU8 *Binary, TS32 BinarySize, TU8 *Image, TS32 XRes, TS32 YRes, CArray<int>& enabledGlyphs = CArray<int>() ); //32 bit raw image data
};

class CWBFont
{
  CAtlas *Atlas;
  //CDictionary<TU16,WBSYMBOL> Alphabet;
  TS32 AlphabetSize = 0;
  WBSYMBOL *Alphabet;
  CDictionary<CWBKerningPair, TS16> Kerning;

  TS32 LineHeight;
  TS32 Base;

  TS32 Offset_X_Char;
  TS32 Height_X_Char;

  TCHAR MissingChar;

  void AddSymbol( TU16 Char, WBATLASHANDLE Handle, CSize &Size, CPoint &Offset, TS32 Advance, CRect contentRect );
  void AddKerningPair( TU16 First, TU16 Second, TS16 Amount );

public:

  CWBFont( CAtlas *Atlas );
  virtual ~CWBFont();
  TBOOL Initialize( CWBFontDescription *Description, TCHAR MissingChar = _T( 'o' ) );

  TS32 GetLineHeight();
  TS32 GetBase();
  TS32 GetOffsetX( TCHAR Char );
  TS32 GetOffsetY( TCHAR Char );
  TS32 GetCenterWidth( TS32 x1, TS32 x2, TCHAR *Text, WBTEXTTRANSFORM Transform = WBTT_NONE );
  TS32 GetCenterWidth( TS32 x1, TS32 x2, CString &Text, WBTEXTTRANSFORM Transform = WBTT_NONE );
  TS32 GetCenterHeight( TS32 y1, TS32 y2 );
  CPoint GetCenter( TCHAR *Text, CRect Rect, WBTEXTTRANSFORM Transform = WBTT_NONE );
  CPoint GetCenter( CString &Text, CRect Rect, WBTEXTTRANSFORM Transform = WBTT_NONE );
  TS32 GetMedian();

	TS32 WriteChar(CWBDrawAPI *DrawApi, int Char, TS32 x, TS32 y, CColor Color = 0xffffffff);
  TS32 Write( CWBDrawAPI *DrawApi, TCHAR *String, TS32 x, TS32 y, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
  TS32 Write( CWBDrawAPI *DrawApi, CString &String, TS32 x, TS32 y, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
	TS32 WriteChar(CWBDrawAPI *DrawApi, int Char, CPoint &p, CColor Color = 0xffffffff);
  TS32 Write( CWBDrawAPI *DrawApi, TCHAR *String, CPoint &p, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
  TS32 Write( CWBDrawAPI *DrawApi, CString &String, CPoint &p, CColor Color = 0xffffffff, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true );
  TS32 GetWidth( TU16 Char, TBOOL Advance = true ); //if Advance is set to false this returns the width of the image in pixels
  TS32 GetWidth( TCHAR *String, TBOOL AdvanceLastChar = true, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true, TBOOL firstCharHack = false );
  TS32 GetWidth( CString &String, TBOOL AdvanceLastChar = true, WBTEXTTRANSFORM Transform = WBTT_NONE, TBOOL DoKerning = true, TBOOL firstCharHack = false );

  TS32 GetHeight( TU16 Char );
  TS32 GetHeight( TCHAR* String );
  TS32 GetHeight( CString& String );

  CPoint GetTextPosition( CString &String, CRect &Container, WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign, WBTEXTTRANSFORM Transform, TBOOL DoKerning = true );
  CPoint GetTextPosition( TCHAR *String, CRect &Container, WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign, WBTEXTTRANSFORM Transform, TBOOL DoKerning = true );

  INLINE TU16 ApplyTextTransformUtf8( _TUCHAR *Text, _TUCHAR *&CurrPos, WBTEXTTRANSFORM Transform );
  INLINE _TUCHAR ApplyTextTransform( _TUCHAR *Text, _TUCHAR *CurrPos, WBTEXTTRANSFORM Transform );

  void ConvertToUppercase();

};