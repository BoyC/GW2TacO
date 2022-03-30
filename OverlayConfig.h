#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "GW2TacOMenu.h"

class Config
{

  static CDictionaryEnumerable<CString, TS32> configValues;
  static CDictionaryEnumerable<CString, CString> configStrings;

public:

  static void InitDefaults();

  static void Load();
  static void Save();
  static void ToggleValue( TCHAR* value );
  static void ToggleValue( CString& value );
  static TS32 GetValue( const TCHAR* value );
  static void SetValue( TCHAR* value, TS32 val );
  static TBOOL HasValue( TCHAR* value );
  static TBOOL HasString( TCHAR* value );
  static void SetString( TCHAR* value, const CString& val );
  static CString GetString( TCHAR* value );

  static TBOOL HasWindowData( TCHAR* windowname );
  static TBOOL IsWindowOpen( TCHAR* windowname );
  static void SetWindowOpenState( TCHAR* windowname, TBOOL Open );
  static CRect GetWindowPosition( TCHAR* windowname );
  static void SetWindowPosition( TCHAR* windowname, CRect Pos );

  static void GetKeyBindings( CDictionary<TS32, TacOKeyAction>& KeyBindings );
  static void DeleteKeyBinding( TS32 key );
  static void SetKeyBinding( TacOKeyAction action, TS32 key );

  static void GetScriptKeyBindings( CDictionary<TS32, CString>& ScriptKeyBindings );
  static void SetScriptKeyBinding( const CString& scriptEvent, TS32 key );
  static void DeleteScriptKeyBinding( const CString& scriptEvent );

  static void LoadMarkerCategoryVisibilityInfo();

  static void AutoSaveConfig();
  static void RemoveValue( TCHAR* value );

  static void SetDefaultValue( TCHAR* cfg, TS32 val );
};
