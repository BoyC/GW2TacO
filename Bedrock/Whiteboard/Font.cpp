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
  if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) //missing character replaced by a simple rectangle
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

  WBSYMBOL &Symbol = Alphabet[ (TU16)Char ];
  TS32 width = Symbol.Advance;
  if ( !width ) return 0;
  //DrawApi->DrawRect(CRect(x + Symbol.OffsetX, y + Symbol.OffsetY, x + Symbol.OffsetX + Symbol.SizeX, y + Symbol.OffsetY + Symbol.SizeY), 0x80808080);
  DrawApi->DrawAtlasElement( Symbol.Handle, x + Symbol.OffsetX, y + Symbol.OffsetY, Color );
  return width;
}

TS32 CWBFont::Write( CWBDrawAPI *DrawApi, TCHAR *String, TS32 x, TS32 y, CColor Color, WBTEXTTRANSFORM Transform, TBOOL DoKerning )
{
  _TUCHAR *Text = (_TUCHAR*)String;
  if ( !Text ) return 0;
  TS32 xp = x;
  TS32 yp = y;

  while ( *Text )
  {
    TU16 Char = ApplyTextTransformUtf8( (_TUCHAR*)String, Text, Transform );

    if ( Char == '\n' )
    {
      xp = x;
      yp += GetLineHeight();
      continue;
    }

    xp += WriteChar( DrawApi, Char, xp, yp, Color );

    if ( DoKerning && *Text && Kerning.NumItems() )
    {
      _TUCHAR* next = Text;
      TU16 NextChar = ApplyTextTransformUtf8( (_TUCHAR*)String, next, Transform );
      CWBKerningPair k = CWBKerningPair( Char, NextChar );
      if ( Kerning.HasKey( k ) )
        xp += Kerning[ k ];
    }
  }

  return xp - x;
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

TBOOL CWBFont::Initialize( CWBFontDescription *Description, TCHAR mc )
{
  MissingChar = mc;
  if ( !Description ) return false;
  if ( !Atlas ) return false;
  if ( !Description->Image ) return false;

  AlphabetSize = 0;
  for ( int x = 0; x < Description->Alphabet.NumItems(); x++ )
    AlphabetSize = max( AlphabetSize, Description->Alphabet[ x ].Char ) + 1;

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

      CRect content = CRect( Description->Alphabet[ x ].UV.x2, Description->Alphabet[ x ].UV.y2, Description->Alphabet[ x ].UV.x1, Description->Alphabet[ x ].UV.y1 );
      bool hadContent = false;

      for ( int j = Description->Alphabet[ x ].UV.y1; j < Description->Alphabet[ x ].UV.y2; j++ )
        for ( int i = Description->Alphabet[ x ].UV.x1; i < Description->Alphabet[ x ].UV.x2; i++ )
        {
          TU8* c = Description->Image + ( j*Description->XRes + i ) * 4;

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
        content -= Description->Alphabet[ x ].UV.TopLeft();
      else
        content = CRect( 0, 0, Description->Alphabet[ x ].UV.Width(), Description->Alphabet[ x ].UV.Height() );

      AddSymbol( Description->Alphabet[ x ].Char, h, Description->Alphabet[ x ].UV.Size(), Description->Alphabet[ x ].Offset, Description->Alphabet[ x ].Advance, content );
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
  if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) Char = MissingChar;
  if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) return 0;

  return Alphabet[ (TU16)Char ].OffsetX;
}

TS32 CWBFont::GetOffsetY( TCHAR Char )
{
  if ( Char >= AlphabetSize || Alphabet[ (TU16)Char ].Char != Char ) Char = MissingChar;
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

