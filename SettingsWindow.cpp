#include "SettingsWindow.h"
#include "GW2TacOMenu.h"
#include "OverlayConfig.h"
#include "Language.h"
#include "GW2API.h"
#include "MarkerPack.h"

extern CDictionaryEnumerable<CString, GW2TacticalCategory*> CategoryMap;
extern CString CGAPaletteNames[];

static const TS32 tabWidth = 130;

enum SettingsTab
{
  Tab_Markers = 0,
  Tab_Trails,
  Tab_Display,
  Tab_Windows,
  Tab_Hotkeys,
  Tab_Menu,
  Tab_Account,
  Tab_General,
  Tab_Count
};

static const char* tabNames[ Tab_Count ][ 2 ] =
{
  { "settings_markers", "Markers" },
  { "settings_trails", "Trails" },
  { "settings_display", "Display" },
  { "settings_windows", "Windows" },
  { "settings_hotkeys", "Hotkeys" },
  { "settings_menu", "TacO menu" },
  { "apikeys", "API keys" },
  { "settings_general", "General" },
};

// main menu entries that can be hidden, see IsMenuItemShown() in GW2TacOMenu.cpp
static const char* hideableMenuItems[][ 2 ] =
{
  { "onlinemarkers", "onlinemarkers" },
  { "rangecircles", "togglerangecircles" },
  { "compass", "togglecompass" },
  { "loctimers", "toggleloctimers" },
  { "hpgrid", "togglehpgrid" },
  { "mousehighlight", "togglemousehighlight" },
  { "maptimer", "openmaptimer" },
  { "ts3", "opentswindow" },
  { "markereditor", "openmarkereditor" },
  { "notepad", "opennotepad" },
  { "raidprogress", "openraidprogress" },
  { "dungeonprogress", "opendungeonprogress" },
  { "tptracker", "opentptracker" },
  { "about", "abouttaco" },
  { "supporttaco", "supporttaco" },
};

static const CColor textColor = CColor::FromARGB( 0xffdddddd );
static const CColor dimTextColor = CColor::FromARGB( 0xff9a9a9a );
static const CColor accentColor = CColor::FromARGB( 0xffffcc77 );
static const CColor lineColor = CColor::FromARGB( 0xff606060 );
static const CColor hoverColor = CColor::FromARGB( 0x40ffffff );
static const CColor selectedColor = CColor::FromARGB( 0xff434346 );
static const CColor optionColor = CColor::FromARGB( 0x80000000 );
static const CColor optionSelectedColor = CColor::FromARGB( 0xc0806030 );

static SettingsRow::Option Opt( const CString& label, TS32 value )
{
  SettingsRow::Option o;
  o.label = label;
  o.value = value;
  return o;
}

static SettingsRow MakeRow( SettingsRow::Type type, const CString& label, TS32 indent = 0 )
{
  SettingsRow row;
  row.type = type;
  row.label = label;
  row.indent = indent;
  return row;
}

static SettingsRow Toggle( const CString& label, const CString& config, TS32 indent = 0 )
{
  SettingsRow row = MakeRow( SettingsRow::Toggle, label, indent );
  row.get = [ config ]() mutable { return Config::GetValue( config.GetPointer() ); };
  row.set = [ config ]( TS32 v ) mutable { Config::SetValue( config.GetPointer(), v ); };
  return row;
}

static SettingsRow Choice( const CString& label, const CString& config, std::vector<SettingsRow::Option> options, TS32 indent = 0 )
{
  SettingsRow row = Toggle( label, config, indent );
  row.type = SettingsRow::Choice;
  row.options = options;
  return row;
}

static SettingsRow Visibility( const CString& label, const CString& config )
{
  return Choice( label, config, { Opt( DICT( "defaultvisibility" ), 1 ), Opt( DICT( "forceonvisibility" ), 2 ), Opt( DICT( "forceoffvisibility" ), 0 ) } );
}

static SettingsRow Opacity( const CString& label, const CString& config )
{
  return Choice( label, config, { Opt( DICT( "opacitysolid" ), 0 ), Opt( DICT( "opacityfaded" ), 1 ), Opt( DICT( "opacitytransparent" ), 2 ) } );
}

static SettingsRow Buttons( const CString& label, std::vector<SettingsRow::Option> buttons, std::function<void( TS32 )> action, TS32 indent = 0 )
{
  SettingsRow row = MakeRow( SettingsRow::Buttons, label, indent );
  row.options = buttons;
  row.set = action;
  return row;
}

static CString CategoryName( GW2TacticalCategory* cat )
{
  return cat->displayName.Length() ? cat->displayName : cat->name;
}

std::vector<SettingsRow> GW2SettingsPanel::BuildRows()
{
  std::vector<SettingsRow> rows;
  GW2TacO* t = taco;

  switch ( tab )
  {
  case Tab_Markers:
  {
    rows.push_back( Toggle( DICT( "settings_showmarkers", "Show markers" ), "TacticalLayerVisible" ) );
    rows.push_back( Choice( DICT( "markerfadedistance", "Fade out beyond" ), "MarkerFadeDistance", { Opt( DICT( "fadeoff", "Off" ), 0 ), Opt( "25 m", 25 ), Opt( "50 m", 50 ), Opt( "100 m", 100 ) } ) );
    rows.push_back( Choice( DICT( "collectedmarkers", "Collected markers" ), "CollectedMarkers", { Opt( DICT( "collectedhide", "Hide" ), 0 ), Opt( DICT( "collectedfaded", "Faded" ), 1 ), Opt( DICT( "collectedshow", "Show" ), 2 ) } ) );

    rows.push_back( MakeRow( SettingsRow::Header, DICT( "markeropacity" ) ) );
    rows.push_back( Opacity( DICT( "ingameopacity" ), "OpacityIngame" ) );
    rows.push_back( Opacity( DICT( "mapopacity" ), "OpacityMap" ) );

    rows.push_back( MakeRow( SettingsRow::Header, DICT( "settings_markersshow", "Where markers show" ) ) );
    rows.push_back( Visibility( DICT( "ingamevisibility" ), "ShowInGameMarkers" ) );
    rows.push_back( Visibility( DICT( "minimapvisibility" ), "ShowMinimapMarkers" ) );
    rows.push_back( Visibility( DICT( "mapvisibility" ), "ShowBigmapMarkers" ) );

    rows.push_back( MakeRow( SettingsRow::Header, DICT( "settings_markerdisplay", "Display" ) ) );
    rows.push_back( Toggle( DICT( "togglepoidistance" ), "TacticalDrawDistance" ) );
    rows.push_back( Toggle( DICT( "togglemetricsystem" ), "UseMetricDisplay" ) );
    rows.push_back( Toggle( DICT( "toggleherdicons" ), "TacticalIconsOnEdge" ) );
    rows.push_back( Toggle( DICT( "toggletacticalinfotext" ), "TacticalInfoTextVisible" ) );
    rows.push_back( Toggle( DICT( "toggledrawwvwnames" ), "DrawWvWNames" ) );
    SettingsRow festivals = Toggle( DICT( "forcefestivals" ), "ForceFestivals" );
    festivals.set = []( TS32 v ) { Config::SetValue( "ForceFestivals", v ); CheckFestivalActive(); };
    rows.push_back( festivals );
    rows.push_back( Toggle( DICT( "nocategoryhiding" ), "NoCategoryHiding" ) );
    rows.push_back( Toggle( DICT( "toggleclipboardaccess" ), "CanWriteToClipboard" ) );

    rows.push_back( MakeRow( SettingsRow::Header, DICT( "tacticalutilities" ) ) );
    rows.push_back( Toggle( DICT( "downloadmarkers" ), "FetchMarkerPacks" ) );
    rows.push_back( Buttons( "", { Opt( DICT( "reloadmarkers" ), 0 ) }, [ t ]( TS32 ) { ImportPOIS( t->GetApplication() ); } ) );

    // removing needs a second click within a few seconds
    TBOOL confirm = globalTimer.GetTime() - removeMarkersClickTime < 3000;
    rows.push_back( Buttons( "", { Opt( confirm ? DICT( "reallyremovemarkers" ) : DICT( "removemymarkers" ), 0 ) }, [ this, t, confirm ]( TS32 )
    {
      removeMarkersClickTime = globalTimer.GetTime();
      if ( !confirm )
        return;
      removeMarkersClickTime = -10000;
      GW2TacticalDisplay* tactical = (GW2TacticalDisplay*)t->GetApplication()->GetRoot()->FindChildByID( "tactical", "gw2tactical" );
      if ( tactical )
        tactical->RemoveUserMarkersFromMap();
    } ) );
    break;
  }

  case Tab_Trails:
    rows.push_back( Choice( DICT( "trailwidth", "Trail width" ), "TrailWidth", { Opt( "50%", 50 ), Opt( "75%", 75 ), Opt( "100%", 100 ), Opt( "150%", 150 ), Opt( "200%", 200 ) } ) );
    rows.push_back( Toggle( DICT( "togglefadeoutbubble" ), "FadeoutBubble" ) );
    rows.push_back( MakeRow( SettingsRow::Header, DICT( "settings_trailsshow", "Where trails show" ) ) );
    rows.push_back( Visibility( DICT( "ingamevisibility" ), "ShowInGameTrails" ) );
    rows.push_back( Visibility( DICT( "minimapvisibility" ), "ShowMinimapTrails" ) );
    rows.push_back( Visibility( DICT( "mapvisibility" ), "ShowBigmapTrails" ) );
    break;

  case Tab_Display:
  {
    rows.push_back( Toggle( DICT( "togglerangecircles" ), "RangeCirclesVisible" ) );
    rows.push_back( Choice( DICT( "rangevisibility" ), "RangeCircleTransparency", { Opt( "40%", 40 ), Opt( "60%", 60 ), Opt( "100%", 100 ) }, 1 ) );
    SettingsRow ranges = MakeRow( SettingsRow::Chips, DICT( "toggleranges" ), 1 );
    const TS32 rangeValues[] = { 90, 120, 180, 240, 300, 400, 600, 900, 1200, 1500, 1600 };
    for ( TS32 range : rangeValues )
    {
      SettingsRow::Option o = Opt( CString::Format( "%d", range ), 0 );
      o.config = CString::Format( "RangeCircle%d", range );
      ranges.options.push_back( o );
    }
    rows.push_back( ranges );

    rows.push_back( Toggle( DICT( "togglecompass" ), "TacticalCompassVisible" ) );
    rows.push_back( Toggle( DICT( "toggleloctimers" ), "LocationalTimersVisible" ) );
    rows.push_back( Toggle( DICT( "togglehpgrid" ), "HPGridVisible" ) );
    rows.push_back( Toggle( DICT( "togglemousehighlight" ), "MouseHighlightVisible" ) );
    rows.push_back( Toggle( DICT( "togglemouseoutline" ), "MouseHighlightOutline", 1 ) );
    std::vector<SettingsRow::Option> colors;
    for ( TS32 x = 0; x < 16; x++ )
      colors.push_back( Opt( DICT( CGAPaletteNames[ x ] ), x ) );
    rows.push_back( Choice( DICT( "mousecolor" ), "MouseHighlightColor", colors, 1 ) );
    break;
  }

  case Tab_Windows:
  {
    auto window = [ t ]( const CString& label, const CString& name )
    {
      SettingsRow row = MakeRow( SettingsRow::Toggle, label );
      row.get = [ name ]() mutable { return (TS32)Config::IsWindowOpen( name.GetPointer() ); };
      row.set = [ t, name ]( TS32 ) { t->OpenWindow( name ); };
      return row;
    };

    rows.push_back( window( DICT( "openmaptimer" ), "MapTimer" ) );
    rows.push_back( Toggle( DICT( "compactmaptimer" ), "MapTimerCompact", 1 ) );
    rows.push_back( window( DICT( "opentswindow" ), "TS3Control" ) );
    rows.push_back( window( DICT( "openmarkereditor" ), "MarkerEditor" ) );
    rows.push_back( Toggle( DICT( "enableexternalediting" ), "EnableExternalEditing", 1 ) );
    rows.push_back( Toggle( DICT( "hideexternalmarkers" ), "HideExternalMarkers", 1 ) );
    rows.push_back( window( DICT( "opennotepad" ), "Notepad" ) );
    rows.push_back( window( DICT( "openraidprogress" ), "RaidProgress" ) );
    rows.push_back( Toggle( DICT( "raidwindow_compact" ), "CompactRaidWindow", 1 ) );
    rows.push_back( window( DICT( "opendungeonprogress" ), "DungeonProgress" ) );
    rows.push_back( window( DICT( "opentptracker" ), "TPTracker" ) );
    rows.push_back( Toggle( DICT( "tptracker_onlyoutbid" ), "TPTrackerOnlyShowOutbid", 1 ) );
    rows.push_back( Toggle( DICT( "tptracker_showbuys" ), "TPTrackerShowBuys", 1 ) );
    rows.push_back( Toggle( DICT( "tptracker_showsells" ), "TPTrackerShowSells", 1 ) );
    rows.push_back( Toggle( DICT( "tptracker_nextsellonly" ), "TPTrackerNextSellOnly", 1 ) );
    rows.push_back( MakeRow( SettingsRow::Header, DICT( "togglewindoweditmode" ) ) );
    rows.push_back( MakeRow( SettingsRow::Text, DICT( "settings_editmodehint", "Turn on edit mode to move and resize these windows." ) ) );
    rows.push_back( Toggle( DICT( "togglewindoweditmode" ), "EditMode" ) );
    break;
  }

  case Tab_Hotkeys:
  {
    rows.push_back( Toggle( DICT( "togglekeybinds" ), "KeybindsEnabled" ) );
    rows.push_back( MakeRow( SettingsRow::Text, DICT( "settings_hotkeyhint", "Click a key to change it. Esc while binding removes it." ) ) );

    auto keyName = [ t ]( TacOKeyAction action )
    {
      for ( TS32 y = 0; y < t->KeyBindings.NumItems(); y++ )
        if ( t->KeyBindings.GetByIndex( y ) == action )
          return GetTacOKeyString( t->KeyBindings.GetKDPair( y )->Key );
      return DICT( "action_no_key_bound" );
    };

    // editor actions are marked with a '*' in ActionNames
    for ( TS32 editor = 0; editor < 2; editor++ )
    {
      rows.push_back( MakeRow( SettingsRow::Header, editor ? DICT( "settings_editorkeys", "Marker editor" ) : DICT( "rebindkeys" ) ) );
      for ( TS32 x = 1; x < (TS32)TacOKeyAction::Count; x++ )
      {
        if ( ( ActionNames[ x ][ 0 ] == '*' ) != ( editor == 1 ) )
          continue;
        TacOKeyAction action = (TacOKeyAction)x;
        rows.push_back( Buttons( DICT( ActionNames[ x ] ), { Opt( keyName( action ), 0 ) }, [ t, action ]( TS32 ) { t->RebindAction( action ); } ) );
      }
    }

    rows.push_back( MakeRow( SettingsRow::Header, DICT( "settings_categorykeys", "Marker categories" ) ) );
    TBOOL anyCategoryKey = false;
    for ( TS32 x = 0; x < CategoryMap.NumItems(); x++ )
    {
      GW2TacticalCategory* cat = CategoryMap.GetByIndex( x );
      if ( !cat->hotkey )
        continue;
      anyCategoryKey = true;
      rows.push_back( Buttons( CategoryName( cat ), { Opt( GetTacOKeyString( cat->hotkey ), 0 ) }, [ t, cat ]( TS32 ) { t->RebindCategory( cat ); } ) );
    }
    if ( !anyCategoryKey )
      rows.push_back( MakeRow( SettingsRow::Text, DICT( "settings_nocategorykeys", "Bind one from a category in the marker filter menu." ) ) );
    break;
  }

  case Tab_Menu:
  {
    rows.push_back( MakeRow( SettingsRow::Header, DICT( "settings_tacobutton", "TacO button" ) ) );
    rows.push_back( MakeRow( SettingsRow::Text, DICT( "settings_tacobuttonhint", "With edit mode on, drag the TacO button to move it." ) ) );
    rows.push_back( Toggle( DICT( "togglewindoweditmode" ), "EditMode" ) );
    rows.push_back( Buttons( "", { Opt( DICT( "settings_resetbutton", "Reset button position" ), 0 ) }, [ t ]( TS32 ) { t->ResetTacOButtonPosition(); } ) );

    rows.push_back( MakeRow( SettingsRow::Header, DICT( "settings_showinmenu", "Show in the TacO menu" ) ) );
    for ( auto& item : hideableMenuItems )
    {
      CString config = CString( "MenuHidden_" ) + item[ 0 ];
      SettingsRow row = MakeRow( SettingsRow::Toggle, DICT( item[ 1 ] ) );
      row.get = [ config ]() mutable { return (TS32)!Config::GetValue( config.GetPointer() ); };
      row.set = [ config ]( TS32 v ) mutable { Config::SetValue( config.GetPointer(), !v ); };
      rows.push_back( row );
    }
    break;
  }

  case Tab_Account:
  {
    rows.push_back( MakeRow( SettingsRow::Header, DICT( "gw2apikey" ) ) );
    GW2::APIKey* keyInUse = GW2::apiKeyManager.GetIdentifiedAPIKey();
    for ( TS32 x = 0; x < GW2::apiKeyManager.keys.NumItems(); x++ )
    {
      auto key = GW2::apiKeyManager.keys[ x ];
      // the fetch thread writes accountName until it sets initialized
      CString name = key->initialized && key->accountName.Length() ? key->accountName : key->apiKey.Substring( 0, 12 ) + "...";
      rows.push_back( Buttons( name, { Opt( DICT( "settings_editkey", "Edit" ), 0 ), Opt( DICT( "deletekey" ), 1 ) }, [ t, x ]( TS32 button )
      {
        if ( button == 0 )
        {
          t->ApiKeyInputAction( APIKeys::GW2APIKey, x );
          return;
        }
        GW2::apiKeyManager.keys.FreeByIndex( x );
        GW2::apiKeyManager.RebuildConfigValues();
      } ) );
      rows.back().highlight = key == keyInUse;
    }
    rows.push_back( Buttons( "", { Opt( DICT( "addgw2apikey" ), 0 ) }, [ t ]( TS32 )
    {
      GW2::APIKey* newKey = new GW2::APIKey();
      GW2::apiKeyManager.keys += newKey;
      t->ApiKeyInputAction( APIKeys::GW2APIKey, GW2::apiKeyManager.keys.NumItems() - 1 );
    } ) );

    rows.push_back( MakeRow( SettingsRow::Header, DICT( "ts3controlplugin" ) ) );
    rows.push_back( Buttons( "", { Opt( DICT( "settings_setts3key", "Set API key" ), 0 ) }, [ t ]( TS32 ) { t->ApiKeyInputAction( APIKeys::TS3APIKey, 0 ); } ) );
    break;
  }

  case Tab_General:
  {
    CStringArray languages = Localization::GetLanguages();
    SettingsRow language = MakeRow( SettingsRow::Choice, DICT( "language" ) );
    for ( TS32 x = 0; x < languages.NumItems(); x++ )
      language.options.push_back( Opt( languages[ x ], x ) );
    language.get = []() { return Localization::GetActiveLanguageIndex(); };
    language.set = []( TS32 v )
    {
      CStringArray languages = Localization::GetLanguages();
      if ( v < languages.NumItems() )
        Localization::SetActiveLanguage( languages[ v ] );
    };
    rows.push_back( language );

    rows.push_back( Toggle( DICT( "togglewindoweditmode" ), "EditMode" ) );
    rows.push_back( Toggle( DICT( "toggleupdatecheck" ), "CheckForUpdates" ) );
    rows.push_back( Toggle( DICT( "hideonload" ), "HideOnLoadingScreens" ) );
    rows.push_back( Toggle( DICT( "closewithgw2" ), "CloseWithGW2" ) );
    rows.push_back( Toggle( DICT( "toggleinfoline" ), "InfoLineVisible" ) );
    rows.push_back( Toggle( DICT( "toggleforcedpiaware" ), "ForceDPIAware" ) );
    rows.push_back( Toggle( DICT( "enabletpnotificationicon" ), "EnableTPNotificationIcon" ) );
    rows.push_back( Toggle( DICT( "tacoontaskbar", "Show TacO on the taskbar" ), "TacOOnTaskBar" ) );
    rows.push_back( Toggle( DICT( "togglecrashoptout" ), "SendCrashDump" ) );
    rows.push_back( Buttons( "", { Opt( DICT( "abouttaco" ), 0 ) }, [ t ]( TS32 ) { t->OpenAboutWindow(); } ) );
    break;
  }
  }

  return rows;
}

GW2SettingsPanel::GW2SettingsPanel( CWBItem* Parent, CRect Position, GW2TacO* taco ) : CWBItem( Parent, Position ), taco( taco )
{
  tab = max( 0, min( Tab_Count - 1, Config::GetValue( "SettingsTab" ) ) );
}

GW2SettingsPanel::~GW2SettingsPanel()
{
}

TS32 GW2SettingsPanel::GetRowHeight( CWBFont* font )
{
  return font->GetLineHeight() + 8;
}

CRect GW2SettingsPanel::GetContentRect( TBOOL scrollbar )
{
  CRect client = GetClientRect();
  return CRect( tabWidth + 12, 4, client.x2 - ( scrollbar ? 24 : 6 ), client.y2 - 4 );
}

// places the option boxes of a row, wrapping them onto more lines when they don't fit; returns the row height
TS32 GW2SettingsPanel::LayoutRow( CWBFont* font, SettingsRow& row, TS32 y, const CRect& content, std::vector<CRect>& optionRects )
{
  TS32 rowHeight = GetRowHeight( font );
  optionRects.clear();

  if ( row.type == SettingsRow::Header )
    return rowHeight + 6;

  if ( row.options.empty() )
    return rowHeight;

  TS32 x1 = content.x1 + row.indent * 16;
  TS32 left = x1;
  if ( row.label.Length() )
    left = content.x1 + max( labelColumn, row.indent * 16 + font->GetWidth( row.label ) + 12 );

  TS32 line = 0;
  TS32 x = left;
  for ( auto& option : row.options )
  {
    TS32 width = font->GetWidth( option.label ) + 14;
    if ( x + width > content.x2 && ( x > left || left > x1 ) )
    {
      if ( x == left )
        left = x1 + 16; // the label left no room, the options go on the lines below it
      x = left;
      line++;
    }
    optionRects.push_back( CRect( x, y + line * rowHeight + 2, x + width, y + ( line + 1 ) * rowHeight - 2 ) );
    x += width + 4;
  }

  return ( line + 1 ) * rowHeight;
}

// works out the content area (narrower when it needs scrolling) and the option column of this tab
CRect GW2SettingsPanel::PrepareLayout( CWBFont* font, std::vector<SettingsRow>& rows, TS32& maxScroll )
{
  CRect content = GetContentRect( false );

  for ( TS32 scrollbar = 0; scrollbar < 2; scrollbar++ )
  {
    content = GetContentRect( scrollbar );
    labelColumn = content.Width() * 2 / 5;
    for ( auto& row : rows )
      if ( !row.options.empty() && row.label.Length() )
        labelColumn = max( labelColumn, min( content.Width() * 3 / 5, row.indent * 16 + font->GetWidth( row.label ) + 12 ) );

    maxScroll = max( 0, GetContentHeight( font, rows, content ) - content.Height() );
    if ( !maxScroll )
      break;
  }

  return content;
}

TS32 GW2SettingsPanel::GetContentHeight( CWBFont* font, std::vector<SettingsRow>& rows, const CRect& content )
{
  std::vector<CRect> optionRects;
  TS32 height = 0;
  for ( auto& row : rows )
    height += LayoutRow( font, row, 0, content, optionRects );
  return height;
}

static void WriteCentered( CWBDrawAPI* API, CWBFont* font, CString& text, const CRect& r, CColor color )
{
  font->Write( API, text, r.x1 + ( r.Width() - font->GetWidth( text ) ) / 2, r.y1 + ( r.Height() - font->GetLineHeight() ) / 2, color );
}

void GW2SettingsPanel::OnDraw( CWBDrawAPI* API )
{
  CWBFont* font = GetFont( GetState() );
  if ( !font )
    return;

  CRect client = GetClientRect();
  CPoint mouse = ScreenToClient( App->GetMousePos() );
  TBOOL mouseOver = MouseOver();
  TS32 rowHeight = GetRowHeight( font );

  for ( TS32 x = 0; x < Tab_Count; x++ )
  {
    CRect r( 0, x * ( rowHeight + 2 ), tabWidth, x * ( rowHeight + 2 ) + rowHeight );
    if ( x == tab )
      API->DrawRect( r, selectedColor );
    else if ( mouseOver && r.Contains( mouse ) )
      API->DrawRect( r, hoverColor );
    CString name = DICT( tabNames[ x ][ 0 ], tabNames[ x ][ 1 ] );
    font->Write( API, name, r.x1 + 8, r.y1 + ( r.Height() - font->GetLineHeight() ) / 2, x == tab ? accentColor : textColor );
  }
  API->DrawRect( CRect( tabWidth + 4, 0, tabWidth + 5, client.y2 ), lineColor );

  auto rows = BuildRows();
  TS32 maxScroll = 0;
  CRect content = PrepareLayout( font, rows, maxScroll );
  scroll = max( 0, min( maxScroll, scroll ) );

  std::vector<CRect> optionRects;
  TS32 y = content.y1 - scroll;
  for ( auto& row : rows )
  {
    TS32 height = LayoutRow( font, row, y, content, optionRects );
    TS32 rowY = y;
    y += height;
    if ( rowY < content.y1 || rowY + height > content.y2 )
      continue; // only whole rows are drawn

    TS32 x1 = content.x1 + row.indent * 16;
    CRect rowRect( x1, rowY, content.x2, rowY + height );
    TS32 textY = rowY + ( rowHeight - font->GetLineHeight() ) / 2;

    switch ( row.type )
    {
    case SettingsRow::Header:
      font->Write( API, row.label, x1, textY + 6, accentColor );
      API->DrawRect( CRect( x1, rowY + height - 2, content.x2, rowY + height - 1 ), lineColor );
      break;

    case SettingsRow::Text:
      font->Write( API, row.label, x1, textY, dimTextColor );
      break;

    case SettingsRow::Toggle:
    {
      if ( mouseOver && rowRect.Contains( mouse ) )
        API->DrawRect( rowRect, hoverColor );
      TS32 cy = rowY + rowHeight / 2;
      CRect box( x1 + 2, cy - 5, x1 + 12, cy + 5 );
      API->DrawRect( box, optionColor );
      API->DrawRectBorder( box, textColor );
      if ( row.get() )
        API->DrawRect( CRect( box.x1 + 2, box.y1 + 2, box.x2 - 2, box.y2 - 2 ), accentColor );
      font->Write( API, row.label, x1 + 18, textY, textColor );
      break;
    }

    case SettingsRow::Choice:
    case SettingsRow::Chips:
    case SettingsRow::Buttons:
    {
      font->Write( API, row.label, x1, textY, row.highlight ? accentColor : textColor );
      TS32 current = row.type == SettingsRow::Choice ? row.get() : 0;
      for ( size_t x = 0; x < row.options.size(); x++ )
      {
        auto& option = row.options[ x ];
        CRect& r = optionRects[ x ];
        TBOOL selected = row.type == SettingsRow::Choice ? option.value == current : row.type == SettingsRow::Chips ? Config::GetValue( option.config.GetPointer() ) != 0 : false;
        API->DrawRect( r, selected ? optionSelectedColor : optionColor );
        if ( mouseOver && r.Contains( mouse ) )
          API->DrawRect( r, hoverColor );
        API->DrawRectBorder( r, selected ? accentColor : lineColor );
        WriteCentered( API, font, option.label, r, selected ? accentColor : textColor );
      }
      break;
    }
    }
  }

  if ( maxScroll )
  {
    CRect up( client.x2 - 18, content.y1, client.x2 - 2, content.y1 + 16 );
    CRect down( client.x2 - 18, content.y2 - 16, client.x2 - 2, content.y2 );
    CString upText = "^";
    CString downText = "v";
    API->DrawRect( up, mouseOver && up.Contains( mouse ) ? selectedColor : optionColor );
    API->DrawRect( down, mouseOver && down.Contains( mouse ) ? selectedColor : optionColor );
    WriteCentered( API, font, upText, up, textColor );
    WriteCentered( API, font, downText, down, textColor );

    TS32 track = down.y1 - up.y2 - 4;
    TS32 thumb = max( 12, track * content.Height() / ( content.Height() + maxScroll ) );
    TS32 thumbY = up.y2 + 2 + ( track - thumb ) * scroll / maxScroll;
    API->DrawRect( CRect( up.x1 + 5, thumbY, up.x2 - 5, thumbY + thumb ), lineColor );
  }
}

void GW2SettingsPanel::OnClick( CPoint pos )
{
  CWBFont* font = GetFont( GetState() );
  if ( !font )
    return;

  TS32 rowHeight = GetRowHeight( font );

  for ( TS32 x = 0; x < Tab_Count; x++ )
  {
    if ( CRect( 0, x * ( rowHeight + 2 ), tabWidth, x * ( rowHeight + 2 ) + rowHeight ).Contains( pos ) )
    {
      tab = x;
      scroll = 0;
      Config::SetValue( "SettingsTab", tab );
      return;
    }
  }

  auto rows = BuildRows();
  TS32 maxScroll = 0;
  CRect content = PrepareLayout( font, rows, maxScroll );

  if ( maxScroll && pos.x >= content.x2 )
  {
    scroll += pos.y < content.Center().y ? -3 * rowHeight : 3 * rowHeight; // clamped when drawn
    return;
  }

  std::vector<CRect> optionRects;
  TS32 y = content.y1 - scroll;
  for ( auto& row : rows )
  {
    TS32 height = LayoutRow( font, row, y, content, optionRects );
    TS32 rowY = y;
    y += height;
    if ( rowY < content.y1 || rowY + height > content.y2 )
      continue;

    if ( row.type == SettingsRow::Toggle && CRect( content.x1 + row.indent * 16, rowY, content.x2, rowY + height ).Contains( pos ) )
    {
      row.set( !row.get() );
      return;
    }

    for ( size_t x = 0; x < optionRects.size(); x++ )
    {
      if ( !optionRects[ x ].Contains( pos ) )
        continue;
      if ( row.type == SettingsRow::Chips )
        Config::ToggleValue( row.options[ x ].config );
      else
        row.set( row.options[ x ].value );
      return;
    }
  }
}

TBOOL GW2SettingsPanel::MessageProc( CWBMessage& Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
    if ( App->GetMouseItem() == this )
    {
      OnClick( ScreenToClient( Message.GetPosition() ) );
      return true;
    }
    break;
  case WBM_MOUSEWHEEL:
    if ( MouseOver() )
    {
      scroll -= Message.Data * 40;
      return true;
    }
    break;
  }

  return CWBItem::MessageProc( Message );
}

GW2SettingsWindow::GW2SettingsWindow( CWBItem* Parent, CRect Position, GW2TacO* taco ) : OverlayWindow( Parent, Position )
{
  Style = WB_WINDOW_CLOSEABLE | WB_WINDOW_MOVEABLE | WB_WINDOW_TITLE;
  SetID( "Settings" );
  SetTitle( DICT( "tacosettings" ) );
  ReapplyStyles();
  ApplyStyleDeclarations( "font-family:trebuchet_normal;text-transform:none;opacity:0.9;" );

  auto panel = new GW2SettingsPanel( this, GetClientRect(), taco );
  panel->ReapplyStyles();
  panel->ApplyStyleDeclarations( "font-family:trebuchet_normal;text-transform:none;" );
}

GW2SettingsWindow::~GW2SettingsWindow()
{
}

// unlike the other overlay windows this one is always interactive and framed
void GW2SettingsWindow::OnDraw( CWBDrawAPI* API )
{
  SetTitle( DICT( "tacosettings" ) ); // follows a language switch made in the General tab
  CWBWindow::OnDraw( API );
}

TBOOL GW2SettingsWindow::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return false;
}
