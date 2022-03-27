#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class Language
{
public:

  CString name;
  CDictionary<CString, CString> dict;
};

class Localization
{
  static int activeLanguageIdx;
  static CArray< Language > languages;

  static void ImportFile( const CString& file );
  static void ImportLanguage( CXMLDocument& d );

  static CArray<int> usedGlyphs;

public:

  static void SetActiveLanguage( const CString& language );
  static CStringArray GetLanguages();

  static void Import();
  static CString Localize( const char* token, const CString& fallback = CString( "" ) );
  static CString Localize( const CString& token, const CString& fallback = CString( "" ) );

  static int GetActiveLanguageIndex();
  static CArray<int>& GetUsedGlyphs();
  static void ProcessStringForUsedGlyphs( CString& string );
};

#define DICT( token, ... ) Localization::Localize( token, __VA_ARGS__ )