#pragma once
#include "Bedrock/BaseLib/BaseLib.h"
#include "GW2TacO.h"

void LoadConfig();
void SaveConfig();
void ToggleConfigValue( TCHAR *value );
void ToggleConfigValue( CString &value );
TS32 GetConfigValue( TCHAR *value );
void SetConfigValue( TCHAR *value, TS32 val );
TBOOL HasConfigValue( TCHAR *value );
TBOOL HasConfigString( TCHAR *value );
void SetConfigString( TCHAR *value, const CString& val );
CString GetConfigString( TCHAR *value );

TBOOL HasWindowData( TCHAR *windowname );
TBOOL IsWindowOpen( TCHAR *windowname );
void SetWindowOpenState( TCHAR *windowname, TBOOL Open );
CRect GetWindowPosition( TCHAR *windowname );
void SetWindowPosition( TCHAR *windowname, CRect Pos );

void GetKeyBindings( CDictionary<TS32, TacOKeyAction> &KeyBindings );
void DeleteKeyBinding( TS32 key );
void SetKeyBinding( TacOKeyAction action, TS32 key );

void GetScriptKeyBindings( CDictionary<TS32, CString> &ScriptKeyBindings );
void SetScriptKeyBinding( const CString& scriptEvent, TS32 key );
void DeleteScriptKeyBinding( const CString& scriptEvent );

void LoadMarkerCategoryVisibilityInfo();

void AutoSaveConfig();
void RemoveConfigEntry(TCHAR* value);
