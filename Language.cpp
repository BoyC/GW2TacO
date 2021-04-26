#include "Language.h"
#include "OverlayConfig.h"

Localization* localization = nullptr;

void Localization::ImportFile( const CString& s )
{
  CXMLDocument d;
  if ( !d.LoadFromFile( s.GetPointer() ) ) return;
  ImportLanguage( d );
}

void Localization::ImportLanguage( CXMLDocument& d )
{
  if ( !d.GetDocumentNode().GetChildCount( "TacOLanguageData" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "TacOLanguageData" );

  CString language;

  if ( root.HasAttribute( "language" ) ) 
    language = root.GetAttributeAsString( "language" );
  else
  {
    LOG_ERR( "[GW2TacO] Language data file didn't specify the language." );
    return;
  }

  language.ToLower();

  int langIdx = -1;

  for ( int x = 0; x < languages.NumItems(); x++ )
  {
    if ( languages[ x ].name == language )
      langIdx = x;
  }

  if ( langIdx < 0 )
  {
    Language lang;
    lang.name = language;
    langIdx = languages.NumItems();
    languages += lang;
  }
  
  auto& lang = languages[ langIdx ].dict;

  int tokenCount = root.GetChildCount( "token" );

  for ( int x = 0; x < tokenCount; x++ )
  {
    auto& token = root.GetChild( "token", x );
    if ( !token.HasAttribute( "key" ) || !token.HasAttribute( "value" ) )
      continue;

    CString key = token.GetAttributeAsString( "key" );
    key.ToLower();
    CString value = token.GetAttributeAsString( "value" );     

    lang[ key ] = value;

    ProcessStringForUsedGlyphs( value );
  }

  ProcessStringForUsedGlyphs( language );
}

Localization::Localization()
{
  for ( int x = 0; x <= 0x460; x++ )
    usedGlyphs.AddUnique( x );
}

void Localization::SetActiveLanguage( const CString& language )
{
  auto lang = language;
  lang.ToLower();

  for ( int x = 0; x < languages.NumItems(); x++ )
  {
    if ( languages[ x ].name == lang )
    {
      activeLanguageIdx = x;
      SetConfigString( "language", lang );
      LOG_NFO( "[GW2TacO] Setting TacO language to %s", language.GetPointer() );
      return;
    }
  }

  activeLanguageIdx = 0;
}

CStringArray Localization::GetLanguages()
{
  CStringArray langs;

  for ( int x = 0; x < languages.NumItems(); x++ )
    langs += languages[ x ].name;

  for ( int x = 0; x < langs.NumItems(); x++ )
    langs[ x ][ 0 ] = toupper( langs[ x ][ 0 ] );

  return langs;
}

void Localization::Import()
{
  ImportFile( "TacO_Language_en.xml" );

  CFileList list;
  list.ExpandSearch( "TacO_Language_*.xml", ".", false );
  for ( TS32 x = 0; x < list.Files.NumItems(); x++ )
    if ( CString::CompareNoCase( list.Files[ x ].FileName, CString( "TacO_Language_en.xml" ) ) != 0 )
    {
      ImportFile( list.Files[ x ].Path + list.Files[ x ].FileName );
    }

  if ( HasConfigString( "language" ) )
  {
    SetActiveLanguage( GetConfigString( "language" ) );
  }
}

CString Localization::Localize( const char* token, const CString& fallback )
{
  CString tokenString( token );
  tokenString.ToLower();
  CString rawToken;
  if ( fallback.Length() )
    rawToken = fallback;
  else
    rawToken = CString( "[" ) + tokenString + "]";

  if ( activeLanguageIdx >= languages.NumItems() || activeLanguageIdx < 0 )
    return rawToken;

  auto& lang = languages[ activeLanguageIdx ].dict;

  if ( !lang.HasKey( tokenString ) )
  {
    static CArray<CString> dumpedStrings;

    if ( dumpedStrings.Find( tokenString ) < 0 )
    {
      LOG_WARN( "[GW2TacO] Translation for token '%s' is not available in the %s language.", tokenString.GetPointer(), languages[ activeLanguageIdx ].name.GetPointer() );
      dumpedStrings += tokenString;
    }

    return rawToken;
  }

  return lang[ tokenString ];
}

CString Localization::Localize( const CString& token, const CString& fallback )
{
  return Localize( token.GetPointer(), fallback );
}

int Localization::GetActiveLanguageIndex()
{
  return activeLanguageIdx;
}

CArray<int>& Localization::GetUsedGlyphs()
{
  return usedGlyphs;
}

void Localization::ProcessStringForUsedGlyphs( CString& string )
{
  string.DecodeUtf8( [ & ]( TU32 Char )->TBOOL
  {
    usedGlyphs.AddUnique( Char );
    return true;
  } );
}
