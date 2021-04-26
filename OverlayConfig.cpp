#include "OverlayConfig.h"
#include "Bedrock/UtilLib/XMLDocument.h"

CDictionaryEnumerable<CString, TS32> ConfigNums;
CDictionaryEnumerable<CString, CString> ConfigStrings;
bool configChanged = false;
auto lastConfigChangeTime = globalTimer.GetTime();

void LoadConfig()
{
  FORCEDDEBUGLOG( "Loading config." );
  CXMLDocument d;
  if ( !d.LoadFromFile( "TacOConfig.xml" ) )
  {
    FORCEDDEBUGLOG( "Config failed to load, setting defaults." );
    SetConfigValue( "EditMode", 0 );
    SetConfigValue( "InterfaceSize", 1 );
    SetConfigValue( "CloseWithGW2", 1 );
    SetWindowOpenState( "MapTimer", true );
    SetWindowPosition( "MapTimer", CRect( 6, 97, 491, 813 ) );
    return;
  }
  ConfigNums.Flush();
  ConfigStrings.Flush();
  FORCEDDEBUGLOG( "Config flushed." );

  if ( !d.GetDocumentNode().GetChildCount( "TacOConfig" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "TacOConfig" );

  FORCEDDEBUGLOG( "Config root found." );

  for ( TS32 x = 0; x < root.GetChildCount(); x++ )
  {
    FORCEDDEBUGLOG( "Loading config value %d/%d.", x, root.GetChildCount() );

    auto item = root.GetChild( x );
    if ( item.HasAttribute( "Data" ) )
    {
      TS32 data = 0;
      item.GetAttributeAsInteger( "Data", &data );
      ConfigNums[ item.GetNodeName() ] = data;
    }

    if ( item.HasAttribute( "String" ) )
    {
      ConfigStrings[ item.GetNodeName() ] = item.GetAttributeAsString( "String" );
    }
  }

  configChanged = false;
}

int StringSorter( const CString& a, const CString& b )
{
  int len = min( a.Length(), b.Length() );

  for ( int x = 0; x < len; x++ )
  {
    int dif = a[ x ] - b[ x ];
    if ( dif != 0 )
      return dif;
  }
  return 0;
}

void SaveConfig()
{
  CXMLDocument doc;
  CXMLNode& root = doc.GetDocumentNode();
  root = root.AddChild( "TacOConfig" );
  ConfigNums.SortByKey( StringSorter );
  ConfigStrings.SortByKey( StringSorter );
  for ( TS32 x = 0; x < ConfigNums.NumItems(); x++ )
  {
    auto kdp = ConfigNums.GetKDPairByIndex( x );
    root.AddChild( kdp->Key.GetPointer() ).SetAttributeFromInteger( "Data", kdp->Data );
  }
  for ( TS32 x = 0; x < ConfigStrings.NumItems(); x++ )
  {
    auto kdp = ConfigStrings.GetKDPairByIndex( x );
    root.AddChild( kdp->Key.GetPointer() ).SetAttribute( "String", kdp->Data.GetPointer() );
  }
  doc.SaveToFile( "TacOConfig.xml" );
}

void ToggleConfigValue( CString &value )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  if ( ConfigNums.HasKey( value ) )
  {
    TS32 v = ConfigNums[ value ];
    ConfigNums[ value ] = !v;
  }
  else
    ConfigNums[ value ] = 0;
}

void ToggleConfigValue( TCHAR *value )
{
  ToggleConfigValue( CString( value ) );
}

TS32 GetConfigValue( TCHAR *value )
{
  if ( ConfigNums.HasKey( value ) )
    return ConfigNums[ value ];
  return 0;
}

void SetConfigValue( TCHAR *value, TS32 val )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[ value ] = val;
}

TBOOL HasConfigValue( TCHAR *value )
{
  return ConfigNums.HasKey( value );
}

TBOOL HasConfigString( TCHAR *value )
{
  return ConfigStrings.HasKey( value );
}

void SetConfigString( TCHAR *value, const CString& val )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigStrings[ value ] = val;
}

CString GetConfigString( TCHAR *value )
{
  if ( ConfigStrings.HasKey( value ) )
    return ConfigStrings[ value ];
  return "";
}

TBOOL IsWindowOpen( TCHAR *windowname )
{
  CString s( windowname );
  return GetConfigValue( ( s + L"_open" ).GetPointer() );
}

void SetWindowOpenState( TCHAR *windowname, TBOOL Open )
{
  CString s( windowname );
  SetConfigValue( ( s + L"_open" ).GetPointer(), (int)Open );
}

CRect GetWindowPosition( TCHAR *windowname )
{
  CRect r;
  CString s( windowname );
  r.x1 = GetConfigValue( ( s + L"_x1" ).GetPointer() );
  r.y1 = GetConfigValue( ( s + L"_y1" ).GetPointer() );
  r.x2 = GetConfigValue( ( s + L"_x2" ).GetPointer() );
  r.y2 = GetConfigValue( ( s + L"_y2" ).GetPointer() );
  return r;
}

void SetWindowPosition( TCHAR *windowname, CRect Pos )
{
  CString s( windowname );
  SetConfigValue( ( s + L"_x1" ).GetPointer(), Pos.x1 );
  SetConfigValue( ( s + L"_y1" ).GetPointer(), Pos.y1 );
  SetConfigValue( ( s + L"_x2" ).GetPointer(), Pos.x2 );
  SetConfigValue( ( s + L"_y2" ).GetPointer(), Pos.y2 );
}

TBOOL HasWindowData( TCHAR *windowname )
{
  CString s( windowname );
  return HasConfigValue( ( s + L"_open" ).GetPointer() ) &&
    HasConfigValue( ( s + L"_x1" ).GetPointer() ) &&
    HasConfigValue( ( s + L"_y1" ).GetPointer() ) &&
    HasConfigValue( ( s + L"_x2" ).GetPointer() ) &&
    HasConfigValue( ( s + L"_y2" ).GetPointer() );
}

void GetKeyBindings( CDictionary<TS32, TacOKeyAction> &KeyBindings )
{
  KeyBindings.Flush();

  for ( TS32 x = 0; x < ConfigNums.NumItems(); x++ )
  {
    auto kdp = ConfigNums.GetKDPair( x );
    if ( kdp->Key.Find( "KeyboardKey_" ) != 0 )
      continue;
    TS32 key;
    if ( kdp->Key.Scan( "KeyboardKey_%d", &key ) != 1 )
      continue;
    if ( kdp->Data == (TS32)TacOKeyAction::NoAction )
      continue;
    KeyBindings[ key ] = (TacOKeyAction)kdp->Data;
  }

  if ( !KeyBindings.NumItems() )
  {
    SetKeyBinding( TacOKeyAction::AddPOI, '+' );
    SetKeyBinding( TacOKeyAction::RemovePOI, '-' );
    SetKeyBinding( TacOKeyAction::ActivatePOI, 'F' );
    SetKeyBinding( TacOKeyAction::ActivatePOI, 'f' );
    SetKeyBinding( TacOKeyAction::EditNotepad, ']' );

    KeyBindings[ '+' ] = TacOKeyAction::AddPOI;
    KeyBindings[ '-' ] = TacOKeyAction::RemovePOI;
    KeyBindings[ 'F' ] = TacOKeyAction::ActivatePOI;
    KeyBindings[ 'f' ] = TacOKeyAction::ActivatePOI;
    KeyBindings[ ']' ] = TacOKeyAction::EditNotepad;
  }
}

void SetKeyBinding( TacOKeyAction action, TS32 key )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[ CString::Format( "KeyboardKey_%d", key ) ] = (TS32)action;
}

void DeleteKeyBinding( TS32 keyToDelete )
{
  for ( TS32 x = 0; x < ConfigNums.NumItems(); x++ )
  {
    auto kdp = ConfigNums.GetKDPair( x );
    if ( kdp->Key.Find( "KeyboardKey_" ) != 0 )
      continue;
    TS32 key;
    if ( kdp->Key.Scan( "KeyboardKey_%d", &key ) != 1 )
      continue;

    if ( key == keyToDelete )
    {
      ConfigNums.Delete( kdp->Key );
      x--;
    }
  }
}

void GetScriptKeyBindings( CDictionary<TS32, CString> &ScriptKeyBindings )
{
  ScriptKeyBindings.Flush();

  for ( TS32 x = 0; x < ConfigNums.NumItems(); x++ )
  {
    auto kdp = ConfigNums.GetKDPair( x );
    if ( kdp->Key.Find( "ScriptKey_" ) != 0 )
      continue;
    CString key = kdp->Key.Substring( 10 );
    if ( key.Length() <= 0 )
      continue;
    ScriptKeyBindings[ kdp->Data ] = key;
  }
}

void SetScriptKeyBinding( const CString& scriptEvent, TS32 key )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  ConfigNums[ CString::Format( "ScriptKey_%s", scriptEvent.GetPointer() ) ] = (TS32)key;
}

void DeleteScriptKeyBinding( const CString& scriptEvent )
{
  for ( TS32 x = 0; x < ConfigNums.NumItems(); x++ )
  {
    auto kdp = ConfigNums.GetKDPair( x );
    if ( kdp->Key.Find( "ScriptKey_" ) != 0 )
      continue;
    CString key = kdp->Key.Substring( 10 );
    if ( key != scriptEvent )
      continue;

    if ( key == scriptEvent )
    {
      ConfigNums.DeleteByIndex( x );
      x--;
    }
  }
}

GW2TacticalCategory *GetCategory( CString s );

void LoadMarkerCategoryVisibilityInfo()
{
  for ( TS32 x = 0; x < ConfigNums.NumItems(); x++ )
  {
    auto kdp = ConfigNums.GetKDPair( x );
    if ( kdp->Key.Find( "CategoryVisible_" ) != 0 )
      continue;

    CString str = kdp->Key.Substring( 16 );
    auto cat = GetCategory( str );
    if ( cat )
      cat->IsDisplayed = kdp->Data;
  }
}

void AutoSaveConfig()
{
  if ( !configChanged )
    return;

  if ( globalTimer.GetTime() - lastConfigChangeTime > 10000 )
  {
    configChanged = false;
    SaveConfig();
  }
}

void RemoveConfigEntry(TCHAR* name)
{
  ConfigStrings.Delete(name);
  ConfigNums.Delete(name);
}