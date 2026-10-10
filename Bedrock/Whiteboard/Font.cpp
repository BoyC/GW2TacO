#include "BasePCH.h"
#include "Font.h"
#include "DrawAPI.h"

INLINE TU32 DictionaryHash( const CWBKerningPair &i )
{
  return i.First + ( i.Second << 16 );
}

INLINE TBOOL CWBKerningPair::operator==( const CWBKerningPair &k )
{
  return First == k.First && Second == k.Second;
}

CWBKerningPair::CWBKerningPair()
{
  First = Second = 0;
}

CWBKerningPair::CWBKerningPair( TU16 a, TU16 b )
{
  First = a;
  Second = b;
}

CWBFontDescription::CWBFontDescription()
{
  Image = NULL;
}

CWBFontDescription::~CWBFontDescription()
{
  SAFEDELETE( Image );
  for ( TS32 x = 0; x < ExtraGlyphs.NumItems(); x++ )
    SAFEDELETEA( ExtraGlyphs[ x ].Image );
}

TBOOL CWBFontDescription::HasChar( TU16 Char )
{
  for ( TS32 x = 0; x < Alphabet.NumItems(); x++ )
    if ( Alphabet[ x ].Char == Char )
      return true;
  for ( TS32 x = 0; x < ExtraGlyphs.NumItems(); x++ )
    if ( ExtraGlyphs[ x ].Char == Char )
      return true;
  return false;
}

TBOOL CWBFontDescription::AddSystemFontGlyphs( const wchar_t *FaceName, TU16 FirstChar, TU16 LastChar, TS32 MinBodyHeight )
{
  if ( !Image ) return false;

  // measure the body height of 'x' and whether the glyphs carry a dark outline (all TacO fonts do)
  TS32 xHeight = 0;
  TBOOL outlined = false;
  for ( TS32 x = 0; x < Alphabet.NumItems(); x++ )
  {
    if ( Alphabet[ x ].Char != 'x' ) continue;

    CRect uv = Alphabet[ x ].UV;
    TS32 top = uv.y2, bottom = uv.y1, opaque = 0, dark = 0;
    for ( TS32 j = uv.y1; j < uv.y2; j++ )
      for ( TS32 i = uv.x1; i < uv.x2; i++ )
      {
        TU8 *c = Image + ( j*XRes + i ) * 4;
        if ( c[ 3 ] <= 10 ) continue;
        top = min( top, j );
        bottom = max( bottom, j + 1 );
        if ( c[ 3 ] > 200 )
        {
          opaque++;
          if ( c[ 0 ] + c[ 1 ] + c[ 2 ] < 300 ) dark++;
        }
      }

    outlined = dark * 100 > opaque * 15;
    xHeight = bottom - top - ( outlined ? 2 : 0 );
    break;
  }

  HDC dc = CreateCompatibleDC( NULL );
  if ( !dc ) return false;

  MAT2 identity = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
  GLYPHMETRICS gm = {};

  // measure FirstChar at a reference size, then pick the size at which it is as tall as our 'x'
  HFONT font = CreateFontW( -100, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, FaceName );
  HGDIOBJ oldFont = SelectObject( dc, font );

  wchar_t usedFace[ LF_FACESIZE ] = {};
  GetTextFaceW( dc, LF_FACESIZE, usedFace );
  TBOOL found = !wcscmp( usedFace, FaceName ) && GetGlyphOutlineW( dc, FirstChar, GGO_METRICS, &gm, 0, NULL, &identity ) != GDI_ERROR && gm.gmBlackBoxY > 0;
  TS32 referenceHeight = gm.gmBlackBoxY;

  SelectObject( dc, oldFont );
  DeleteObject( font );

  if ( !found )
  {
    LOG( LOG_WARNING, _T( "[gui] System font '%S' not found, can't add glyphs %x-%x to font" ), FaceName, FirstChar, LastChar );
    DeleteDC( dc );
    return false;
  }

  TS32 emSize = max( 1, (TS32)( 100.0f * max( xHeight, MinBodyHeight ) / referenceHeight + 0.5f ) );
  font = CreateFontW( -emSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, FaceName );
  oldFont = SelectObject( dc, font );

  TS32 border = outlined ? 1 : 0;

  for ( TS32 c = FirstChar; c <= LastChar; c++ )
  {
    if ( HasChar( c ) ) continue;

    wchar_t wc = (wchar_t)c;
    WORD glyphIndex = 0xffff;
    if ( GetGlyphIndicesW( dc, &wc, 1, &glyphIndex, GGI_MARK_NONEXISTING_GLYPHS ) == GDI_ERROR || glyphIndex == 0xffff )
      continue; // unassigned code point

    DWORD size = GetGlyphOutlineW( dc, c, GGO_GRAY8_BITMAP, &gm, 0, NULL, &identity );
    if ( size == GDI_ERROR || !size ) continue;

    TU8 *coverage = new TU8[ size ];
    if ( GetGlyphOutlineW( dc, c, GGO_GRAY8_BITMAP, &gm, size, coverage, &identity ) == GDI_ERROR )
    {
      SAFEDELETEA( coverage );
      continue;
    }

    TS32 pitch = ( gm.gmBlackBoxX + 3 ) & ~3;
    auto alphaAt = [&]( TS32 x, TS32 y ) -> TS32 // coverage 0..255 at output pixel x,y
    {
      x -= border;
      y -= border;
      if ( x < 0 || y < 0 || x >= (TS32)gm.gmBlackBoxX || y >= (TS32)gm.gmBlackBoxY ) return 0;
      return min( 255, coverage[ y*pitch + x ] * 255 / 64 );
    };

    WBEXTRAGLYPH g;
    g.Char = c;
    g.XRes = gm.gmBlackBoxX + border * 2;
    g.YRes = gm.gmBlackBoxY + border * 2;
    g.Image = new TU8[ g.XRes*g.YRes * 4 ];
    g.Offset = CPoint( gm.gmptGlyphOrigin.x - border, Base - gm.gmptGlyphOrigin.y - border );
    g.Advance = gm.gmCellIncX;

    for ( TS32 y = 0; y < g.YRes; y++ )
      for ( TS32 x = 0; x < g.XRes; x++ )
      {
        TS32 a = alphaAt( x, y );
        TS32 outline = 0;
        for ( TS32 j = -border; j <= border && border; j++ )
          for ( TS32 i = -border; i <= border; i++ )
            outline = max( outline, alphaAt( x + i, y + j ) );

        // white glyph over a black outline
        TS32 alpha = a + outline * ( 255 - a ) / 255;
        TU8 *p = g.Image + ( y*g.XRes + x ) * 4;
        p[ 0 ] = p[ 1 ] = p[ 2 ] = alpha ? a * 255 / alpha : 0;
        p[ 3 ] = alpha;
      }

    SAFEDELETEA( coverage );
    ExtraGlyphs += g;
  }

  SelectObject( dc, oldFont );
  DeleteObject( font );
  DeleteDC( dc );

  return true;
}

TBOOL CWBFontDescription::LoadBMFontBinary( TU8 *Binary, TS32 BinarySize, TU8 *img, TS32 xr, TS32 yr, CArray<int>& enabledGlyphs )
{
  if ( !img || !Binary || BinarySize <= 0 || xr <= 0 || yr <= 0 ) return false;

  XRes = xr;
  YRes = yr;
  Image = new TU8[ xr*yr * 4 ];
  memcpy( Image, img, xr*yr * 4 );

#pragma pack(push,1)

  struct BMCOMMON
  {
    TU16 lineHeight;
    TU16 base;
    TU16 scaleW;
    TU16 scaleH;
    TU16 pages;
    TU8 bitField;
    TU8 alphaChnl;
    TU8 redChnl;
    TU8 greenChnl;
    TU8 blueChnl;
  };

  struct BMCHAR
  {
    TU32 id;
    TU16 x;
    TU16 y;
    TU16 width;
    TU16 height;
    TS16 xoffset;
    TS16 yoffset;
    TS16 xadvance;
    TU8 page;
    TU8 chnl;
  };

  struct BMKERNINGDATA
  {
    TU32 first;
    TU32 second;
    TS16 amount;
  };

#pragma pack(pop)

  CStreamReaderMemory m;
  m.Open( Binary, BinarySize );

  if ( m.ReadByte() != 'B' || m.ReadByte() != 'M' || m.ReadByte() != 'F' || m.ReadByte() != 3 )
  {
    LOG( LOG_ERROR, _T( "[gui] Error loading font data: bad fileformat or unsupported version" ) );
    return false;
  }

  while ( !m.eof() )
  {
    TS32 BlockType = m.ReadByte();
    TS32 BlockSize = m.ReadDWord();
    TU8 *BlockData = new TU8[ BlockSize ];

    if ( m.Read( BlockData, BlockSize ) != BlockSize )
    {
      LOG( LOG_ERROR, _T( "[gui] Error loading font data: unable to read block data" ) );
      return false;
    }

    switch ( BlockType )
    {
    case 1: //info block, ignored
      break;
    case 2: //common data
    {
      if ( BlockSize != sizeof( BMCOMMON ) )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: common block size doesn't match" ) );
        return false;
      }
      BMCOMMON *cmn = (BMCOMMON*)BlockData;
      if ( cmn->pages > 1 )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: only single page fonts are supported" ) );
        return false;
      }
      LineHeight = cmn->lineHeight;
      Base = cmn->base;
    }
    break;
    case 3: //page name list
      break;
    case 4: //characters
    {
      if ( BlockSize % sizeof( BMCHAR ) != 0 )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: character block size doesn't match" ) );
        return false;
      }

      BMCHAR *c = (BMCHAR*)BlockData;

      for ( TU32 x = 0; x < BlockSize / sizeof( BMCHAR ); x++ )
      {
        if ( !enabledGlyphs.NumItems() || enabledGlyphs.Find( c[ x ].id ) >= 0 )
        {
          WBSYMBOLINPUT s;
          s.Char = c[ x ].id;
          s.Advance = c[ x ].xadvance;
          s.Offset = CPoint( c[ x ].xoffset, c[ x ].yoffset );
          s.UV = CRect( c[ x ].x, c[ x ].y, c[ x ].x + c[ x ].width, c[ x ].y + c[ x ].height );

          if ( c[ x ].id >= 0 && c[ x ].id <= 0xffff )
            Alphabet += s;
        }
      }
    }
    break;
    case 5: //kerning data
    {
      if ( BlockSize % sizeof( BMKERNINGDATA ) != 0 )
      {
        LOG( LOG_ERROR, _T( "[gui] Error loading font data: kerning block size doesn't match" ) );
        return false;
      }

      BMKERNINGDATA *k = (BMKERNINGDATA*)BlockData;

      for ( TU32 x = 0; x < BlockSize / sizeof( BMKERNINGDATA ); x++ )
      {
        WBKERNINGDATA d;
        d.First = k[ x ].first;
        d.Second = k[ x ].second;
        d.Amount = k[ x ].amount;
        KerningData += d;
      }
    }
    break;

    default:
      LOG( LOG_ERROR, _T( "[gui] Error loading font data: unknown block type" ) );
      return false;
    }


    SAFEDELETEA( BlockData );
  }

  return true;
}

bool ReadInt( const CString& s, const CString& val, TS32& result )
{
  int p = s.Find( val + "=" );
  if ( p < 0 )
    return false;
  return s.Substring( p ).Scan( ( val + "=%d" ).GetPointer(), &result ) == 1;
}

TBOOL CWBFontDescription::LoadBMFontText( TU8 *Binary, TS32 BinarySize, TU8 *img, TS32 xr, TS32 yr, CArray<int>& enabledGlyphs )
{
  if ( !img || !Binary || BinarySize <= 0 || xr <= 0 || yr <= 0 ) return false;

  XRes = xr;
  YRes = yr;
  Image = new TU8[ xr*yr * 4 ];
  memcpy( Image, img, xr*yr * 4 );

  CStreamReaderMemory m;
  m.Open( Binary, BinarySize );

  if ( m.ReadByte() != 'i' || m.ReadByte() != 'n' || m.ReadByte() != 'f' || m.ReadByte() != 'o' )
  {
    LOG( LOG_ERROR, _T( "[gui] Error loading font data: bad fileformat or unsupported version" ) );
    return false;
  }

  CString s;
  do
  {
    s = m.ReadLine();

    if ( s.Find( "info" ) == 0 )
      continue;

    if ( s.Find( "common" ) == 0 )
    {
      if ( !ReadInt( s, "lineHeight", LineHeight ) )
        return false;
      if ( !ReadInt( s, "base", Base ) )
        return false;
      continue;
    }

    if ( s.Find( "page" ) == 0 )
    {
      continue;
    }

    if ( s.Find( "char" ) == 0 )
    {
      if ( s.Find( "chars" ) == 0 )
        continue;

      WBSYMBOLINPUT r;
      TS32 v;

      if ( !ReadInt( s, "id", v ) )
        return false;
      r.Char = v;

      if ( !ReadInt( s, "xadvance", v ) )
        return false;
      r.Advance = v;

      if ( !ReadInt( s, "xoffset", v ) )
        return false;
      r.Offset.x = v;

      if ( !ReadInt( s, "yoffset", v ) )
        return false;
      r.Offset.y = v;

      int x, y, width, height;

      if ( !ReadInt( s, "x", x ) )
        return false;
      if ( !ReadInt( s, "y", y ) )
        return false;
      if ( !ReadInt( s, "width", width ) )
        return false;
      if ( !ReadInt( s, "height", height ) )
        return false;

      r.UV = CRect( x, y, x + width, y + height );

      if ( !enabledGlyphs.NumItems() || enabledGlyphs.Find( r.Char ) >= 0 )
      {
        if ( r.Char >= 0 && r.Char <= 0xffff )
          Alphabet += r;
      }

      continue;
    }

    if ( s.Find( "kerning" ) == 0 )
    {
      if ( s.Find( "kernings" ) == 0 )
        continue;

      WBKERNINGDATA d;
      TS32 v;

      if ( !ReadInt( s, "first", v ) )
        return false;
      d.First = v;

      if ( !ReadInt( s, "second", v ) )
        return false;
      d.Second = v;

      if ( !ReadInt( s, "amount", v ) )
        return false;
      d.Amount = v;

      KerningData += d;
      continue;
    }

  } while ( s.Length() > 0 );

  return true;
}

CWBFont::CWBFont( CAtlas *atlas )
{
  Atlas = atlas;
  LineHeight = 0;
  Base = 0;
  Offset_X_Char = 0;
}

CWBFont::~CWBFont()
{
  for ( TS32 x = 0; x < AlphabetSize; x++ )
    if ( Alphabet[ x ].Char == x )
      Atlas->DeleteImage( Alphabet[ x ].Handle );
  SAFEDELETEA( Alphabet );
}

void CWBFont::AddSymbol( TU16 Char, WBATLASHANDLE Handle, CSize &Size, CPoint &Offset, TS32 Advance, CRect contentRect )
{
  if ( Char >= AlphabetSize )
    return;

  WBSYMBOL s;
  s.Char = Char;
  s.Handle = Handle;
  s.OffsetX = Offset.x;
  s.OffsetY = Offset.y;
  s.Advance = Advance;
  s.SizeX = Size.x;
  s.SizeY = Size.y;
  s.calculatedContentRect = contentRect;

  Alphabet[ Char ] = s;
  //Alphabet.Add(Char, s);

  if ( Char == 'x' )
  {
    Offset_X_Char = Offset.y + contentRect.y1;
    Height_X_Char = contentRect.x2 - contentRect.x1;
  }
}

TU32 ReadUTF8Char( _TUCHAR* &Text )
{
  TU32 Char = *Text;
  Text++;

  if ( (Char & 0x80) ) // decode utf-8
  {
    if ( (Char & 0xe0) == 0xc0 )
    {
      Char = Char & ( ( 1 << 5 ) - 1 );
      for ( int z = 0; z < 1; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xf0) == 0xe0 )
    {
      Char = Char & ( ( 1 << 4 ) - 1 );
      for ( int z = 0; z < 2; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xf8) == 0xf0 )
    {
      Char = Char & ( ( 1 << 3 ) - 1 );
      for ( int z = 0; z < 3; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xfc) == 0xf8 )
    {
      Char = Char & ( ( 1 << 2 ) - 1 );
      for ( int z = 0; z < 4; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
    else
    if ( (Char & 0xfe) == 0xfc )
    {
      Char = Char & ( ( 1 << 1 ) - 1 );
      for ( int z = 0; z < 5; z++ )
      {
        if ( *( Text ) )
        {
          Char = ( Char << 6 ) + ( ( *( Text ) ) & 0x3f );
          Text++;
        }
      }
    }
  }

  return Char;
}

TS32 CWBFont::WriteChar(CWBDrawAPI *DrawApi, int Char, TS32 x, TS32 y, CColor Color)
{
  // Char is negative for bytes >= 0x80 of a signed char string (the textbox passes raw bytes)
  if ( Char < 0 || Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) //missing character replaced by a simple rectangle
  {
    if ( Alphabet[ (TU16)MissingChar ].Char != MissingChar )
    {
      LOG( LOG_WARNING, _T( "[font] Used character %d and fallback character %d also missing from font." ), Char, MissingChar );
      return 0;
    }

    TS32 width = GetWidth( MissingChar );

    if ( Char != ' ' )
    {
      WBSYMBOL &mc = Alphabet[ (TU16)MissingChar ];
      DrawApi->DrawRectBorder( CRect( x + mc.OffsetX, y + mc.OffsetY, x + mc.OffsetX + mc.SizeX, y + mc.OffsetY + mc.SizeY ), Color );
    }
    return width;
  }

  // zero advance glyphs (combining marks) are drawn too, over the previous character
  WBSYMBOL &Symbol = Alphabet[ (TU16)Char ];
  //DrawApi->DrawRect(CRect(x + Symbol.OffsetX, y + Symbol.OffsetY, x + Symbol.OffsetX + Symbol.SizeX, y + Symbol.OffsetY + Symbol.SizeY), 0x80808080);
  DrawApi->DrawAtlasElement( Symbol.Handle, x + Symbol.OffsetX, y + Symbol.OffsetY, Color );
  return Symbol.Advance;
}

TS32 CWBFont::Write( CWBDrawAPI *DrawApi, TCHAR *String, TS32 x, TS32 y, CColor Color, WBTEXTTRANSFORM Transform, TBOOL DoKerning )
{
  _TUCHAR *Text = (_TUCHAR*)String;
  if ( !Text ) return 0;
  TS32 xp = x;
  TS32 yp = y;
  TU16 Prev = 0;
  TU16 Base = 0; // last character that advanced the pen

  while ( *Text )
  {
    TU16 Char = ApplyTextTransformUtf8( (_TUCHAR*)String, Text, Transform );

    if ( Char == '\n' )
    {
      xp = x;
      yp += GetLineHeight();
      Prev = Base = 0;
      continue;
    }

    _TUCHAR* next = Text;
    TU16 NextChar = *Text ? ApplyTextTransformUtf8( (_TUCHAR*)String, next, Transform ) : 0;

    CPoint markOffset = GetThaiMarkOffset( Prev, Char, NextChar, Base );
    TS32 advance = WriteChar( DrawApi, Char, xp + markOffset.x, yp + markOffset.y, Color );
    xp += advance;
    if ( advance )
      Base = Char;
    Prev = Char;

    if ( DoKerning && NextChar && Kerning.NumItems() )
    {
      CWBKerningPair k = CWBKerningPair( Char, NextChar );
      if ( Kerning.HasKey( k ) )
        xp += Kerning[ k ];
    }
  }

  return xp - x;
}

TBOOL CWBFont::HasSymbol( TU16 Char )
{
  return Char < AlphabetSize && Alphabet[ Char ].Char == Char;
}

static TBOOL IsThaiAboveMark( TU16 c ) // upper vowels, tone marks and other diacritics drawn above the consonant
{
  return c == 0x0E31 || ( c >= 0x0E34 && c <= 0x0E37 ) || ( c >= 0x0E47 && c <= 0x0E4E );
}

// Thai vowels and tone marks above the line are zero advance glyphs drawn over the preceding consonant. Without
// OpenType shaping a tone mark collides with an upper vowel (or the nikhahit of sara am) below it, and marks cover
// the ascender of the tall consonants, so nudge them roughly the way a shaping engine would.
CPoint CWBFont::GetThaiMarkOffset( TU16 Prev, TU16 Char, TU16 Next, TU16 Base )
{
  CPoint offset( 0, 0 );

  if ( !IsThaiAboveMark( Char ) )
    return offset;

  TBOOL toneMark = Char >= 0x0E48 && Char <= 0x0E4C;
  TBOOL upperVowelBelow = Prev == 0x0E31 || ( Prev >= 0x0E34 && Prev <= 0x0E37 ) || Prev == 0x0E47 || Prev == 0x0E4D || Next == 0x0E33;
  if ( toneMark && upperVowelBelow )
    offset.y = ThaiToneRaise;

  if ( Base == 0x0E1B || Base == 0x0E1D || Base == 0x0E1F || Base == 0x0E2C ) // po pla, fo fa, fo fan, lo chula
    offset.x = -ThaiTallShift;

  return offset;
}

TS32 CWBFont::Write( CWBDrawAPI *DrawApi, CString &String, TS32 x, TS32 y, CColor Color, WBTEXTTRANSFORM Transform, TBOOL DoKerning )
{
  return Write( DrawApi, String.GetPointer(), x, y, Color, Transform, DoKerning );
}

TS32 CWBFont::WriteChar(CWBDrawAPI *DrawApi, int Char, CPoint &p, CColor Color)
{
  return WriteChar( DrawApi, Char, p.x, p.y, Color );
}

TS32 CWBFont::Write( CWBDrawAPI *DrawApi, TCHAR *String, CPoint &p, CColor Color, WBTEXTTRANSFORM Transform, TBOOL DoKerning )
{
  return Write( DrawApi, String, p.x, p.y, Color, Transform, DoKerning );
}

TS32 CWBFont::Write( CWBDrawAPI *DrawApi, CString &String, CPoint &p, CColor Color, WBTEXTTRANSFORM Transform, TBOOL DoKerning )
{
  return Write( DrawApi, String, p.x, p.y, Color, Transform, DoKerning );
}

TS32 CWBFont::GetWidth( TU16 Char, TBOOL Advance )
{
  if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) //missing character
  {
    if ( Alphabet[ (TU16)MissingChar ].Char != MissingChar )
      return 0;
    return Alphabet[ (TU16)MissingChar ].Advance;
  }
  return Advance ? Alphabet[ (TU16)Char ].Advance : ( Alphabet[ (TU16)Char ].OffsetX + Alphabet[ (TU16)Char ].calculatedContentRect.x2 );
}

TS32 CWBFont::GetWidth( TCHAR *String, TBOOL AdvanceLastChar, WBTEXTTRANSFORM Transform, TBOOL DoKerning, TBOOL firstCharHack )
{
  _TUCHAR *Text = (_TUCHAR*)String;
  if ( !Text ) return 0;
  TS32 xp = 0;
  TS32 maxXp = 0;

  bool firstChar = true;

  while ( *Text )
  {
    TU16 Char = ApplyTextTransformUtf8( (_TUCHAR*)String, Text, Transform );

    if ( Char == '\n' )
    {
      maxXp = max( xp, maxXp );
      xp = 0;
      continue;
    }

    if ( firstCharHack && firstChar && !AdvanceLastChar )
    {
      if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char )
        xp -= Alphabet[ (TU16)MissingChar ].calculatedContentRect.x1;
      else
        xp -= Alphabet[ (TU16)Char ].calculatedContentRect.x1;
    }

    if ( AdvanceLastChar )
      xp += GetWidth( Char );
    else
    {
      if ( *Text )
        xp += GetWidth( Char );
      else
        xp += GetWidth( Char, false );
    }

    if ( DoKerning && *Text && Kerning.NumItems() )
    {
      _TUCHAR* next = Text;
      TU16 NextChar = ApplyTextTransformUtf8( (_TUCHAR*)String, next, Transform );
      CWBKerningPair k = CWBKerningPair( Char, NextChar );
      if ( Kerning.HasKey( k ) )
        xp += Kerning[ k ];
    }
  }

  return max( maxXp, xp );
}

TS32 CWBFont::GetWidth( CString &String, TBOOL AdvanceLastChar, WBTEXTTRANSFORM Transform, TBOOL DoKerning, TBOOL firstCharHack )
{
  return GetWidth( String.GetPointer(), AdvanceLastChar, Transform, DoKerning, firstCharHack );
}

void CWBFont::AddKerningPair( TU16 First, TU16 Second, TS16 Amount )
{
  Kerning[ CWBKerningPair( First, Second ) ] = Amount;
}

// bounding box of the visible pixels of the UV area of an image, relative to the UV area
static CRect GetGlyphContentRect( TU8 *Image, TS32 XRes, const CRect &UV )
{
  CRect content = CRect( UV.x2, UV.y2, UV.x1, UV.y1 );
  bool hadContent = false;

  for ( int j = UV.y1; j < UV.y2; j++ )
    for ( int i = UV.x1; i < UV.x2; i++ )
    {
      TU8* c = Image + ( j*XRes + i ) * 4;

      if ( c[ 3 ] > 10 )
      {
        hadContent = true;
        content.x1 = min( content.x1, i );
        content.x2 = max( content.x2, i );
        content.y1 = min( content.y1, j );
        content.y2 = max( content.y2, j );
      }
    }

  if ( hadContent )
    return content - UV.TopLeft();

  return CRect( 0, 0, UV.Width(), UV.Height() );
}

TBOOL CWBFont::Initialize( CWBFontDescription *Description, TCHAR mc )
{
  MissingChar = mc;
  if ( !Description ) return false;
  if ( !Atlas ) return false;
  if ( !Description->Image ) return false;

  AlphabetSize = 0;
  for ( int x = 0; x < Description->Alphabet.NumItems(); x++ )
    AlphabetSize = max( AlphabetSize, Description->Alphabet[ x ].Char ) + 1;
  for ( int x = 0; x < Description->ExtraGlyphs.NumItems(); x++ )
    AlphabetSize = max( AlphabetSize, Description->ExtraGlyphs[ x ].Char + 1 );

  Alphabet = new WBSYMBOL[ AlphabetSize ];
  memset( Alphabet, 0, AlphabetSize * sizeof( WBSYMBOL ) );
  for ( TS32 x = 0; x < AlphabetSize; x++ )
    Alphabet[ x ].Char = (TS16)( x - 1 );

  for ( TS32 x = 0; x < Description->Alphabet.NumItems(); x++ )
    if ( Description->Alphabet[ x ].UV.Area() > 0 )
    {
      WBATLASHANDLE h = Atlas->AddImage( Description->Image, Description->XRes, Description->YRes, Description->Alphabet[ x ].UV );

      if ( !h )
      {
        LOG( LOG_ERROR, _T( "[gui] Atlas Error while creating font!" ) );
        return false;
      };

      CRect content = GetGlyphContentRect( Description->Image, Description->XRes, Description->Alphabet[ x ].UV );
      AddSymbol( Description->Alphabet[ x ].Char, h, Description->Alphabet[ x ].UV.Size(), Description->Alphabet[ x ].Offset, Description->Alphabet[ x ].Advance, content );
    }

  for ( TS32 x = 0; x < Description->ExtraGlyphs.NumItems(); x++ )
  {
    WBEXTRAGLYPH &g = Description->ExtraGlyphs[ x ];
    CRect uv( 0, 0, g.XRes, g.YRes );
    WBATLASHANDLE h = Atlas->AddImage( g.Image, g.XRes, g.YRes, uv );
    if ( !h )
      continue;

    AddSymbol( g.Char, h, uv.Size(), g.Offset, g.Advance, GetGlyphContentRect( g.Image, g.XRes, uv ) );
  }

  // see GetThaiMarkOffset()
  if ( HasSymbol( 0x0E34 ) && HasSymbol( 0x0E48 ) )
  {
    WBSYMBOL &upperVowel = Alphabet[ 0x0E34 ]; // sara i
    WBSYMBOL &toneMark = Alphabet[ 0x0E48 ]; // mai ek
    ThaiToneRaise = min( 0, ( upperVowel.OffsetY + upperVowel.calculatedContentRect.y1 ) - ( toneMark.OffsetY + toneMark.calculatedContentRect.y2 ) );
  }
  for ( TS32 x = 0; x < Description->ExtraGlyphs.NumItems(); x++ )
  {
    // shift marks left until they clear the ascender of po pla: its left edge is the first pixel of its top row
    WBEXTRAGLYPH &g = Description->ExtraGlyphs[ x ];
    if ( g.Char != 0x0E1B || !HasSymbol( g.Char ) )
      continue;

    TS32 ascenderLeft = g.Offset.x;
    for ( TS32 i = 0; i < g.XRes * g.YRes; i++ )
      if ( g.Image[ i * 4 + 3 ] > 10 )
      {
        ascenderLeft += i % g.XRes;
        break;
      }

    for ( TU16 m = 0x0E31; m <= 0x0E4E; m++ )
      if ( IsThaiAboveMark( m ) && HasSymbol( m ) )
        ThaiTallShift = max( ThaiTallShift, g.Advance + Alphabet[ m ].OffsetX + Alphabet[ m ].calculatedContentRect.x2 - ascenderLeft );
  }

  for ( TS32 x = 0; x < Description->KerningData.NumItems(); x++ )
  {
    AddKerningPair( Description->KerningData[ x ].First, Description->KerningData[ x ].Second, Description->KerningData[ x ].Amount );
  }

  LineHeight = Description->LineHeight;
  Base = Description->Base;

  return true;
}

TS32 CWBFont::GetLineHeight()
{
  return LineHeight;
}

TS32 CWBFont::GetBase()
{
  return Base;
}

TS32 CWBFont::GetOffsetX( TCHAR Char )
{
  if ( Char < 0 || Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) Char = MissingChar;
  if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) return 0;

  return Alphabet[ (TU16)Char ].OffsetX;
}

TS32 CWBFont::GetOffsetY( TCHAR Char )
{
  if ( Char < 0 || Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) Char = MissingChar;
  if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) return 0;

  return Alphabet[ (TU16)Char ].OffsetY;
}

TS32 CWBFont::GetCenterWidth( TS32 x1, TS32 x2, TCHAR *Text, WBTEXTTRANSFORM Transform )
{
  //_TUCHAR Char = 0;
  //if (Text && *Text)
  //  Char = ApplyTextTransform((_TUCHAR*)Text, (_TUCHAR*)Text, Transform);

  return ( x1 + x2 - GetWidth( Text, false, Transform, true ) ) / 2;// -GetOffsetX(Char);
}

TS32 CWBFont::GetCenterWidth( TS32 x1, TS32 x2, CString &Text, WBTEXTTRANSFORM Transform )
{
  return GetCenterWidth( x1, x2, Text.GetPointer(), Transform );
}

TS32 CWBFont::GetCenterHeight( TS32 y1, TS32 y2 )
{
  return y1 + ( y2 - y1 ) / 2 - GetMedian();
}

CPoint CWBFont::GetCenter( TCHAR *Text, CRect Rect, WBTEXTTRANSFORM Transform )
{
  return CPoint( GetCenterWidth( Rect.x1, Rect.x2, Text, Transform ), GetCenterHeight( Rect.y1, Rect.y2 ) );
}

CPoint CWBFont::GetCenter( CString &Text, CRect Rect, WBTEXTTRANSFORM Transform )
{
  return GetCenter( Text.GetPointer(), Rect, Transform );
}

TS32 CWBFont::GetMedian()
{
  return Offset_X_Char + Height_X_Char / 2;// (Base - Offset_X_Char) / 2;// +1;// Height_X_Char / 2;
}

void CWBFont::ConvertToUppercase()
{
  for ( TS32 x = 0; x < AlphabetSize; x++ )
    if ( Alphabet[ x ].Char == x )
    {
      WBSYMBOL &c = Alphabet[ x ];
      if ( c.Char != towupper( c.Char ) )
      {
        if ( Alphabet[ towupper( c.Char ) ].Char == towupper( c.Char ) )
        {
          WBSYMBOL &C = Alphabet[ towupper( c.Char ) ];

          c.Advance = C.Advance;
          Atlas->DeleteImage( c.Handle );
          c.Handle = C.Handle;
          c.OffsetX = C.OffsetX;
          c.OffsetY = C.OffsetY;
          c.SizeX = C.SizeX;
          c.SizeY = C.SizeY;
        }
      }
    }
}

INLINE _TUCHAR CWBFont::ApplyTextTransform( _TUCHAR *Text, _TUCHAR *CurrPos, WBTEXTTRANSFORM Transform )
{
  if ( Transform <= 0 ) return *CurrPos;
  switch ( Transform )
  {
  case WBTT_NONE:
    return *CurrPos;
    break;
  case WBTT_CAPITALIZE:
    if ( Text == CurrPos || ( CurrPos > Text && _istspace( *( CurrPos - 1 ) ) ) ) return _totupper( *CurrPos );
    return _totlower( *CurrPos );
    break;
  case WBTT_UPPERCASE:
    return _totupper( *CurrPos );
    break;
  case WBTT_LOWERCASE:
    return _totlower( *CurrPos );
    break;
  default:
    break;
  }
  return 0;
}

INLINE TU16 CWBFont::ApplyTextTransformUtf8( _TUCHAR *Text, _TUCHAR *&CurrPos, WBTEXTTRANSFORM Transform )
{
  TU32 decoded = ReadUTF8Char( CurrPos );

  if ( Transform <= 0 ) 
    return decoded;

  switch ( Transform )
  {
  case WBTT_NONE:
    return decoded;
    break;
  case WBTT_CAPITALIZE:
    if ( Text == CurrPos || ( CurrPos > Text && _istspace( *( CurrPos - 1 ) ) ) ) return _totupper( decoded );
    return _totlower( decoded );
    break;
  case WBTT_UPPERCASE:
    return _totupper( decoded );
    break;
  case WBTT_LOWERCASE:
    return _totlower( decoded );
    break;
  default:
    break;
  }
  return 0;
}

TS32 CWBFont::GetHeight( TU16 Char )
{
  return GetLineHeight();
}

TS32 CWBFont::GetHeight( TCHAR* String )
{
  TS32 lineCount = 1;
  while ( *String )
  {
    if ( *String == '\n' )
      lineCount++;
    String++;
  }

  return LineHeight * lineCount;
}

TS32 CWBFont::GetHeight( CString& String )
{
  return GetHeight( String.GetPointer() );
}

CPoint CWBFont::GetTextPosition( CString &String, CRect &Container, WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign, WBTEXTTRANSFORM Transform /*= WBTT_NONE*/, TBOOL DoKerning /*= true*/ )
{
  CPoint p = Container.TopLeft();

  TS32 Width = GetWidth( String, false, Transform, DoKerning );
  TS32 Height = GetLineHeight();

  if ( XAlign == WBTA_CENTERX ) p.x = GetCenterWidth( Container.x1, Container.x2, String, Transform );
  if ( XAlign == WBTA_RIGHT ) p.x = Container.x2 - Width;
  if ( YAlign == WBTA_CENTERY ) p.y = GetCenterHeight( Container.y1, Container.y2 );
  if ( YAlign == WBTA_BOTTOM ) p.y = Container.y2 - Height;

  return p;
}

CPoint CWBFont::GetTextPosition( TCHAR *String, CRect &Container, WBTEXTALIGNMENTX XAlign, WBTEXTALIGNMENTY YAlign, WBTEXTTRANSFORM Transform /*= WBTT_NONE*/, TBOOL DoKerning /*= true*/ )
{
  return GetTextPosition( CString( String ), Container, XAlign, YAlign, Transform, DoKerning );
}

