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
  int activeLanguageIdx = 0;
  CArray< Language > languages;

  void ImportFile( const CString& file );
  void ImportLanguage( CXMLDocument& d );

  CArray<int> usedGlyphs;

public:

  Localization();

  void SetActiveLanguage( const CString& language );
  CStringArray GetLanguages();

  void Import();
  CString Localize( const char* token, const CString& fallback = CString( "" ) );
  CString Localize( const CString& token, const CString& fallback = CString( "" ) );

  int GetActiveLanguageIndex();
  CArray<int>& GetUsedGlyphs();
  void ProcessStringForUsedGlyphs( CString& string );
};

extern Localization* localization;
#define DICT( token, ... ) localization->Localize( token, __VA_ARGS__ )