#include "OverlayConfig.h"
#include "Bedrock/UtilLib/XMLDocument.h"

CDictionaryEnumerable<CString, TS32> Config::configValues;
CDictionaryEnumerable<CString, CString> Config::configStrings;

void Config::SetDefaultValue( TCHAR* cfg, TS32 val )
{
  if ( !HasValue( cfg ) )
    SetValue( cfg, val );
}

bool configChanged = false;
auto lastConfigChangeTime = globalTimer.GetTime();

void Config::InitDefaults()
{
  SetDefaultValue( "FrameThrottling", 1 );
  SetDefaultValue( "Vsync", 1 );
  SetDefaultValue( "SmoothCharacterPos", 1 );
  SetDefaultValue( "CheckForUpdates", 1 );
  SetDefaultValue( "HideOnLoadingScreens", 1 );
  SetDefaultValue( "KeybindsEnabled", 1 );
  SetDefaultValue( "EnableCrashMenu", 0 );
  SetDefaultValue( "SendCrashDump", 1 );
  SetDefaultValue( "CanWriteToClipboard", 1 );
  SetDefaultValue( "LogTrails", 0 );
  SetDefaultValue( "CloseWithGW2", 1 );
  SetDefaultValue( "InfoLineVisible", 0 );
  SetDefaultValue( "EnableTPNotificationIcon", 1 );
  SetDefaultValue( "TacticalIconsOnEdge", 1 );
  SetDefaultValue( "TacticalLayerVisible", 1 );
  SetDefaultValue( "DrawWvWNames", 1 );
  SetDefaultValue( "TacticalDrawDistance", 0 );
  SetDefaultValue( "UseMetricDisplay", 0 );
  SetDefaultValue( "OpacityIngame", 0 );
  SetDefaultValue( "OpacityMap", 0 );
  SetDefaultValue( "TacticalInfoTextVisible", 1 );
  SetDefaultValue( "HPGridVisible", 1 );
  SetDefaultValue( "LocationalTimersVisible", 1 );
  SetDefaultValue( "MapTimerVisible", 1 );
  SetDefaultValue( "MapTimerCompact", 1 );
  SetDefaultValue( "AutoHideMarkerEditor", 1 );
  SetDefaultValue( "TacticalLayerVisible", 1 );
  SetDefaultValue( "MouseHighlightVisible", 0 );
  SetDefaultValue( "MouseHighlightColor", 0 );
  SetDefaultValue( "MouseHighlightOutline", 0 );
  SetDefaultValue( "CompactRaidWindow", 0 );
  SetDefaultValue( "RangeCirclesVisible", 0 );
  SetDefaultValue( "RangeCircleTransparency", 100 );
  SetDefaultValue( "RangeCircle90", 0 );
  SetDefaultValue( "RangeCircle120", 1 );
  SetDefaultValue( "RangeCircle180", 0 );
  SetDefaultValue( "RangeCircle240", 0 );
  SetDefaultValue( "RangeCircle300", 1 );
  SetDefaultValue( "RangeCircle400", 1 );
  SetDefaultValue( "RangeCircle600", 1 );
  SetDefaultValue( "RangeCircle900", 1 );
  SetDefaultValue( "RangeCircle1200", 1 );
  SetDefaultValue( "RangeCircle1500", 0 );
  SetDefaultValue( "RangeCircle1600", 0 );
  SetDefaultValue( "TacticalCompassVisible", 0 );
  SetDefaultValue( "TPTrackerOnlyShowOutbid", 0 );
  SetDefaultValue( "TPTrackerShowBuys", 1 );
  SetDefaultValue( "TPTrackerShowSells", 1 );
  SetDefaultValue( "TPTrackerNextSellOnly", 0 );
  SetDefaultValue( "TrailLayerVisible", 1 );
  SetDefaultValue( "FadeoutBubble", 1 );
  SetDefaultValue( "TacticalLayerVisible", 1 );
  SetDefaultValue( "ShowMinimapMarkers", 1 );
  SetDefaultValue( "ShowBigmapMarkers", 1 );
  SetDefaultValue( "ShowInGameMarkers", 1 );
  SetDefaultValue( "ShowMinimapTrails", 1 );
  SetDefaultValue( "ShowBigmapTrails", 1 );
  SetDefaultValue( "ShowInGameTrails", 1 );
  SetDefaultValue( "FetchMarkerPacks", 1 );
  SetDefaultValue( "ForceFestivals", 0 );
  SetDefaultValue( "NoCategoryHiding", 0 );
  SetDefaultValue( "EnableExternalEditing", 1 );
  SetDefaultValue( "HideExternalMarkers", 0 );
}

void Config::Load()
{
  CXMLDocument d;
  if ( !d.LoadFromFile( "TacOConfig.xml" ) )
  {
    SetValue( "EditMode", 0 );
    SetValue( "InterfaceSize", 1 );
    SetValue( "CloseWithGW2", 1 );
    SetWindowOpenState( "MapTimer", true );
    SetWindowPosition( "MapTimer", CRect( 6, 97, 491, 813 ) );
    return;
  }
  configValues.Flush();
  configStrings.Flush();

  if ( !d.GetDocumentNode().GetChildCount( "TacOConfig" ) ) return;
  CXMLNode root = d.GetDocumentNode().GetChild( "TacOConfig" );

  for ( TS32 x = 0; x < root.GetChildCount(); x++ )
  {
    auto item = root.GetChild( x );
    if ( item.HasAttribute( "Data" ) )
    {
      TS32 data = 0;
      item.GetAttributeAsInteger( "Data", &data );
      configValues[ item.GetNodeName() ] = data;
    }

    if ( item.HasAttribute( "String" ) )
    {
      configStrings[ item.GetNodeName() ] = item.GetAttributeAsString( "String" );
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

void Config::Save()
{
  CXMLDocument doc;
  CXMLNode& root = doc.GetDocumentNode();
  root = root.AddChild( "TacOConfig" );
  configValues.SortByKey( StringSorter );
  configStrings.SortByKey( StringSorter );
  for ( TS32 x = 0; x < configValues.NumItems(); x++ )
  {
    auto kdp = configValues.GetKDPairByIndex( x );
    root.AddChild( kdp->Key.GetPointer() ).SetAttributeFromInteger( "Data", kdp->Data );
  }
  for ( TS32 x = 0; x < configStrings.NumItems(); x++ )
  {
    auto kdp = configStrings.GetKDPairByIndex( x );
    root.AddChild( kdp->Key.GetPointer() ).SetAttribute( "String", kdp->Data.GetPointer() );
  }
  doc.SaveToFile( "TacOConfig.xml" );
}

void Config::ToggleValue( const CString& value )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  if ( configValues.HasKey( value ) )
  {
    TS32 v = configValues[ value ];
    configValues[ value ] = !v;
  }
  else
    configValues[ value ] = 0;
}

void Config::ToggleValue( const TCHAR* value )
{
  ToggleValue( CString( value ) );
}

TS32 Config::GetValue( const TCHAR* value )
{
  if ( configValues.HasKey( value ) )
    return configValues[ value ];
  return 0;
}

void Config::SetValue( TCHAR* value, TS32 val )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  configValues[ value ] = val;
}

TBOOL Config::HasValue( TCHAR* value )
{
  return configValues.HasKey( value );
}

TBOOL Config::HasString( TCHAR* value )
{
  return configStrings.HasKey( value );
}

void Config::SetString( TCHAR* value, const CString& val )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  configStrings[ value ] = val;
}

CString Config::GetString( TCHAR* value )
{
  if ( configStrings.HasKey( value ) )
    return configStrings[ value ];
  return "";
}

TBOOL Config::IsWindowOpen( TCHAR* windowname )
{
  CString s( windowname );
  return GetValue( GetWindowOpenConfigValue( windowname ).GetPointer() );
}

CString Config::GetWindowOpenConfigValue( TCHAR* windowname )
{
  return CString( windowname ) + L"_open";
}

void Config::SetWindowOpenState( TCHAR* windowname, TBOOL Open )
{
  CString s( windowname );
  SetValue( ( s + L"_open" ).GetPointer(), (int)Open );
}

CRect Config::GetWindowPosition( TCHAR* windowname )
{
  CRect r;
  CString s( windowname );
  r.x1 = GetValue( ( s + L"_x1" ).GetPointer() );
  r.y1 = GetValue( ( s + L"_y1" ).GetPointer() );
  r.x2 = GetValue( ( s + L"_x2" ).GetPointer() );
  r.y2 = GetValue( ( s + L"_y2" ).GetPointer() );
  return r;
}

void Config::SetWindowPosition( TCHAR* windowname, CRect Pos )
{
  CString s( windowname );
  SetValue( ( s + L"_x1" ).GetPointer(), Pos.x1 );
  SetValue( ( s + L"_y1" ).GetPointer(), Pos.y1 );
  SetValue( ( s + L"_x2" ).GetPointer(), Pos.x2 );
  SetValue( ( s + L"_y2" ).GetPointer(), Pos.y2 );
}

TBOOL Config::HasWindowData( TCHAR* windowname )
{
  CString s( windowname );
  return HasValue( ( s + L"_open" ).GetPointer() ) &&
    HasValue( ( s + L"_x1" ).GetPointer() ) &&
    HasValue( ( s + L"_y1" ).GetPointer() ) &&
    HasValue( ( s + L"_x2" ).GetPointer() ) &&
    HasValue( ( s + L"_y2" ).GetPointer() );
}

void Config::GetKeyBindings( CDictionary<TS32, TacOKeyAction>& KeyBindings )
{
  KeyBindings.Flush();

  for ( TS32 x = 0; x < configValues.NumItems(); x++ )
  {
    auto kdp = configValues.GetKDPair( x );
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

void Config::SetKeyBinding( TacOKeyAction action, TS32 key )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  configValues[ CString::Format( "KeyboardKey_%d", key ) ] = (TS32)action;
}

void Config::DeleteKeyBinding( TS32 keyToDelete )
{
  for ( TS32 x = 0; x < configValues.NumItems(); x++ )
  {
    auto kdp = configValues.GetKDPair( x );
    if ( kdp->Key.Find( "KeyboardKey_" ) != 0 )
      continue;
    TS32 key;
    if ( kdp->Key.Scan( "KeyboardKey_%d", &key ) != 1 )
      continue;

    if ( key == keyToDelete )
    {
      configValues.Delete( kdp->Key );
      x--;
    }
  }
}

void Config::GetScriptKeyBindings( CDictionary<TS32, CString>& ScriptKeyBindings )
{
  ScriptKeyBindings.Flush();

  for ( TS32 x = 0; x < configValues.NumItems(); x++ )
  {
    auto kdp = configValues.GetKDPair( x );
    if ( kdp->Key.Find( "ScriptKey_" ) != 0 )
      continue;
    CString key = kdp->Key.Substring( 10 );
    if ( key.Length() <= 0 )
      continue;
    ScriptKeyBindings[ kdp->Data ] = key;
  }
}

void Config::SetScriptKeyBinding( const CString& scriptEvent, TS32 key )
{
  configChanged = true;
  lastConfigChangeTime = globalTimer.GetTime();
  configValues[ CString::Format( "ScriptKey_%s", scriptEvent.GetPointer() ) ] = (TS32)key;
}

void Config::DeleteScriptKeyBinding( const CString& scriptEvent )
{
  for ( TS32 x = 0; x < configValues.NumItems(); x++ )
  {
    auto kdp = configValues.GetKDPair( x );
    if ( kdp->Key.Find( "ScriptKey_" ) != 0 )
      continue;
    CString key = kdp->Key.Substring( 10 );
    if ( key != scriptEvent )
      continue;

    if ( key == scriptEvent )
    {
      configValues.DeleteByIndex( x );
      x--;
    }
  }
}

GW2TacticalCategory* GetCategory( CString s );

void Config::LoadMarkerCategoryVisibilityInfo()
{
  for ( TS32 x = 0; x < configValues.NumItems(); x++ )
  {
    auto kdp = configValues.GetKDPair( x );
    if ( kdp->Key.Find( "CategoryVisible_" ) != 0 )
      continue;

    CString str = kdp->Key.Substring( 16 );
    auto cat = GetCategory( str );
    if ( cat )
      cat->isDisplayed = kdp->Data;
  }

  CategoryRoot.CalculateVisibilityCache();
}

void Config::AutoSaveConfig()
{
  if ( !configChanged )
    return;

  if ( globalTimer.GetTime() - lastConfigChangeTime > 10000 )
  {
    configChanged = false;
    Save();
  }
}

void Config::RemoveValue( TCHAR* name )
{
  configStrings.Delete( name );
  configValues.Delete( name );
}
