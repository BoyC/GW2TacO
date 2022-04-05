#include "GW2TacOMenu.h"
#include "OverlayConfig.h"
#include "gw2tactical.h"
#include "MumbleLink.h"
#include <Shellapi.h>
#include "OverlayWindow.h"
#include "MapTimer.h"
#include "BuildInfo.h"
#include "TS3Control.h"
#include "TS3Connection.h"
#include "MarkerEditor.h"
#include "Notepad.h"
#include "RaidProgress.h"
#include "DungeonProgress.h"
#include "TPTracker.h"
#include "GW2API.h"
#include "Bedrock/UtilLib/jsonxx.h"
#include "TrailLogger.h"
#include "Language.h"
#include "BuildCount.h"
#include "ThirdParty/BugSplat/inc/BugSplat.h"
#include "MarkerPack.h"
using namespace jsonxx;

CString UIFileNames[] =
{
  "UI_small.css",
  "UI_normal.css",
  "UI_large.css",
  "UI_larger.css",
};

CString ActionNames[] =
{
  // '*' at start means this will be placed under the marker editor menu!

  "no_action",//"No Action",
  "add_marker",//"Add New Marker",
  "remove_marker",//"Remove Marker",
  "action_key",//"Action Key",
  "edit_notepad",//"Edit notepad",
  "*toggle_trail_recording",//"*Start/Stop Trail Recording",
  "*pause_trail_recording",//"*Pause/Resume Trail Recording",
  "*remove_last_trail",//"*Remove Last Trail Segment",
  "*resume_trail",//"*Resume Trail By Creating New Section"
  "toggle_tactical_layer",
  "toggle_range_circles",
  "toggle_tactical_compass",
  "toggle_locational_timers",
  "toggle_hp_grids",
  "toggle_mouse_highlight",
  "toggle_map_timer",
  "toggle_ts3_window",
  "toggle_marker_editor",
  "toggle_notepad",
  "toggle_raid_progress",
  "toggle_dungeon_progress",
  "toggle_tp_tracker",
  "toggle_window_edit_mode",
  "*add_default_marker_2",
  "*add_default_marker_3",
  "*add_default_marker_4",
  "*add_default_marker_5",
  "*delete_selected_marker",
  "*delete_selected_segment",
  "*copy_selected_marker",
};

CString APIKeyNames[] =
{
  "no_action",//"No Action",
  "ts3_clientquery_plugin",//"TS3 ClientQuery Plugin",
  "guild_wars_2",//"Guild Wars 2",
};

enum MainMenuItems
{
  Menu_Exit = 0x25472,
  Menu_About,
  Menu_ToggleHighLight,
  Menu_ToggleTactical,
  Menu_ToggleMapTimer,
  Menu_ToggleMarkerEditor,
  Menu_ToggleNotepad,
  Menu_ToggleTS3Control,
  Menu_ToggleRaidProgress,
  Menu_ToggleDungeonProgress,
  Menu_ToggleTPTracker,
  Menu_ToggleTPTracker_OnlyOutbid,
  Menu_ToggleTPTracker_ShowBuys,
  Menu_ToggleTPTracker_ShowSells,
  Menu_ToggleTPTracker_OnlyNextFulfilled,
  Menu_ToggleEditMode,
  Menu_ToggleTacticalsOnEdge,
  Menu_ToggleDrawDistance,
  Menu_DrawWvWNames,
  Menu_ToggleInfoLine,
  Menu_ToggleLocationalTimers,
  Menu_ToggleGW2ExitMode,
  Menu_Interface_Small,
  Menu_Interface_Normal,
  Menu_Interface_Large,
  Menu_Interface_Larger,
  Menu_ToggleVersionCheck,
  Menu_DownloadNewBuild,
  Menu_SupportTacO,
  Menu_ToggleRangeCircles,
  Menu_RangeCircleTransparency40,
  Menu_RangeCircleTransparency60,
  Menu_RangeCircleTransparency100,
  Menu_ToggleRangeCircle90,
  Menu_ToggleRangeCircle120,
  Menu_ToggleRangeCircle180,
  Menu_ToggleRangeCircle240,
  Menu_ToggleRangeCircle300,
  Menu_ToggleRangeCircle400,
  Menu_ToggleRangeCircle600,
  Menu_ToggleRangeCircle900,
  Menu_ToggleRangeCircle1200,
  Menu_ToggleRangeCircle1500,
  Menu_ToggleRangeCircle1600,
  Menu_ToggleTacticalCompass,
  Menu_ToggleVsync,
  Menu_ToggleHPGrid,
  Menu_ToggleCompactMapTimer,
  Menu_ToggleMouseHighlightOutline,
  Menu_MouseHighlightColor0,
  Menu_MouseHighlightColor1,
  Menu_MouseHighlightColor2,
  Menu_MouseHighlightColor3,
  Menu_MouseHighlightColor4,
  Menu_MouseHighlightColor5,
  Menu_MouseHighlightColor6,
  Menu_MouseHighlightColor7,
  Menu_MouseHighlightColor8,
  Menu_MouseHighlightColor9,
  Menu_MouseHighlightColora,
  Menu_MouseHighlightColorb,
  Menu_MouseHighlightColorc,
  Menu_MouseHighlightColord,
  Menu_MouseHighlightColore,
  Menu_MouseHighlightColorf,
  Menu_TS3APIKey,
  Menu_GW2APIKey,
  Menu_ToggleAutoHideMarkerEditor,
  Menu_HideOnLoadingScreens,
  Menu_ToggleFadeoutBubble,
  Menu_TacOSettings,
  Menu_Language,
  Menu_ToggleCompactRaids,
  Menu_MarkerVisibility_MiniMap_Off,
  Menu_MarkerVisibility_MiniMap_Default,
  Menu_MarkerVisibility_MiniMap_Force,
  Menu_MarkerVisibility_BigMap_Off,
  Menu_MarkerVisibility_BigMap_Default,
  Menu_MarkerVisibility_BigMap_Force,
  Menu_MarkerVisibility_InGameMap_Off,
  Menu_MarkerVisibility_InGameMap_Default,
  Menu_MarkerVisibility_InGameMap_Force,
  Menu_TrailVisibility_MiniMap_Off,
  Menu_TrailVisibility_MiniMap_Default,
  Menu_TrailVisibility_MiniMap_Force,
  Menu_TrailVisibility_BigMap_Off,
  Menu_TrailVisibility_BigMap_Default,
  Menu_TrailVisibility_BigMap_Force,
  Menu_TrailVisibility_InGameMap_Off,
  Menu_TrailVisibility_InGameMap_Default,
  Menu_TrailVisibility_InGameMap_Force,
  Menu_ToggleMetricSystem,
  Menu_ToggleForceDPIAware,
  Menu_ToggleShowNotificationIcon,
  Menu_ReloadMarkers,
  Menu_DeleteMyMarkers,
  Menu_AddGW2ApiKey,
  Menu_TogglePOIInfoText,
  Menu_Crash,
  Menu_OptOutFromCrashReports,
  Menu_ToggleClipboardAccess,
  Menu_ToggleAutomaticMarkerUpdates,
  Menu_ForceFestivals,
  Menu_NoCategoryHiding,
  Menu_ToggleExternalEditing,
  Menu_HideExternalMarkers,
  Menu_ExportMyMapMarkers,
  Menu_TacOOnTaskBar,

  Menu_OpacityIngame_Solid,
  Menu_OpacityIngame_Transparent,
  Menu_OpacityIngame_Faded,
  Menu_OpacityMap_Solid,
  Menu_OpacityMap_Transparent,
  Menu_OpacityMap_Faded,
  Menu_KeyBindsEnabled,

  Menu_RaidToggles = 0x1000,
  Menu_RaidToggles_End = 0x2000,
  Menu_GW2APIKey_Base = 0x3000,
  Menu_GW2APIKey_End = 0x4000,
  Menu_DeleteGW2APIKey_Base = 0x5000,
  Menu_DeleteGW2APIKey_End = 0x6000,

  Menu_ToggleMapTimerMap = 0x30000,

  Menu_RebindKey_Base = 0x31337,

  Menu_Language_Base = 0x40000,

  Menu_MarkerPacks_Base = 0x40100,

  Menu_MarkerFilter_Base = 0x65535,
};

//extern CArray<ScriptKeyBind> scriptKeyBinds;

TBOOL GW2TacO::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( editor && !editor->IsHidden() )
      return editor->GetMouseTransparency( ClientSpacePoint, MessageType );
  }


  return true;
}

GW2TacO::GW2TacO( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{
  Config::GetKeyBindings( KeyBindings );
  Config::GetScriptKeyBindings( ScriptKeyBindings );
}

GW2TacO::~GW2TacO()
{
  if ( pickupFetcherThread.joinable() )
    pickupFetcherThread.join();

  //scriptEngines.FreeArray();
}

CWBItem* GW2TacO::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  auto ret = new GW2TacO( Root, Pos );
  ret->SetFocus();

  return ret;
}

bool iconSizesStored = false;
CRect tacoIconRect;
CRect menuHoverRect;
CRect tpButtonRect;
CRect tpHighlightRect;
int scaleCountDownHack = 0;

void ChangeUIScale( int size )
{
  if ( size < 0 || size > 3 )
  {
    LOG_ERR( "Someone wants to set the UI size to %d! Are you multi-clienting perhaps?", size );
    return;
  }

  extern CWBApplication* App;
  if ( !App )
    return;

  if ( App->LoadCSSFromFile( UIFileNames[ size ], true ) )
    Config::SetValue( "InterfaceSize", size );
  App->ReApplyStyle();

  scaleCountDownHack = 2;
  iconSizesStored = false;
}

CString buildText2( "WW91IGNhbiBzdXBwb3J0IGRldmVsb3BtZW50IGJ5IGRvbmF0aW5nIGluLWdhbWUgdG8gQm95Qy4yNjUzIDop" ); // You can support development by donating in-game to BoyC.2653 :)

CString GW2TacO::GetKeybindString( TacOKeyAction action )
{
  for ( TS32 x = 0; x < KeyBindings.NumItems(); x++ )
    if ( KeyBindings.GetByIndex( x ) == action )
    {
      return CString::Format( "[%c] ", KeyBindings.GetKDPair( x )->Key );
      break;
    }
  return "";
}

CString GetConfigActiveString( const char* cfg, int active = -1 )
{
  if ( active == -1 )
    return Config::GetValue( cfg ) ? "[x] " : "[ ] ";
  return Config::GetValue( cfg ) == active ? "[x] " : "[ ] ";
}

struct ToggleOption
{
  CString text;
  CString configOption;
  CString activeText;
  int menuItem;
  bool toggle = true;
  int value = -1;
  bool doHighlight = false;
  bool rebuildMenu = false;
};

CDictionary<int, ToggleOption> toggleOptions;

CWBContextItem* AddToggleOption( CWBContextMenu* root, const CString config, const CString& text, int menuItem, bool highlighted = false, bool rebuildMenu = false )
{
  ToggleOption option;
  option.text = text;
  option.configOption = config;
  option.menuItem = menuItem;
  option.rebuildMenu = rebuildMenu;
  toggleOptions[ menuItem ] = option;

  return root->AddItem( ( GetConfigActiveString( config.GetPointer() ) + text ).GetPointer(), menuItem, highlighted, false );
}

CWBContextItem* AddSetValueOption( CWBContextItem* root, const CString config, const CString& text, int menuItem, int value, bool highlighted = false )
{
  ToggleOption option;
  option.text = text;
  option.configOption = config;
  option.menuItem = menuItem;
  option.toggle = false;
  option.value = value;
  option.doHighlight = highlighted;
  toggleOptions[ menuItem ] = option;

  return root->AddItem( ( GetConfigActiveString( config.GetPointer(), value ) + text ).GetPointer(), menuItem, highlighted ? ( value == Config::GetValue( config.GetPointer() ) ) : false, false );
}

CWBContextItem* AddToggleOption( CWBContextItem* root, const CString config, const CString& text, int menuItem, bool highlighted = false, bool rebuildMenu = false )
{
  ToggleOption option;
  option.text = text;
  option.configOption = config;
  option.menuItem = menuItem;
  option.rebuildMenu = rebuildMenu;
  toggleOptions[ menuItem ] = option;

  return root->AddItem( ( GetConfigActiveString( config.GetPointer() ) + text ).GetPointer(), menuItem, highlighted, false );
}

CWBContextItem* AddTwoTextOption( CWBContextMenu* root, const CString config, const CString& text, const CString& textActive, int menuItem, bool highlighted = false )
{
  ToggleOption option;
  option.text = text;
  option.configOption = config;
  option.menuItem = menuItem;
  option.activeText = textActive;
  option.rebuildMenu = true;
  toggleOptions[ menuItem ] = option;

  return root->AddItem( Config::GetValue( config.GetPointer() ) ? textActive : text, menuItem, highlighted, false );
}

void GW2TacO::OpenMainMenu( CWBContextMenu* ctx )
{
  ctx->SetID( "TacOMenu" );

  if ( Config::GetValue( "TacticalLayerVisible" ) )
  {
    auto flt = ctx->AddItem( DICT( "filtermarkers" ), 0, false, false );
    {
      CLightweightCriticalSection cs( &Achievements::critSec );
      OpenTypeContextMenu( flt, CategoryList, true, Menu_MarkerFilter_Base, false, Achievements::achievements );
    }
    auto options = ctx->AddItem( DICT( "tacticalsettings" ), 0, false, false );

    AddToggleOption( options, "TacticalDrawDistance", DICT( "togglepoidistance" ), Menu_ToggleDrawDistance );
    AddToggleOption( options, "TacticalIconsOnEdge", DICT( "toggleherdicons" ), Menu_ToggleTacticalsOnEdge );
    AddToggleOption( options, "DrawWvWNames", DICT( "toggledrawwvwnames" ), Menu_DrawWvWNames );
    AddToggleOption( options, "FadeoutBubble", DICT( "togglefadeoutbubble" ), Menu_ToggleFadeoutBubble );
    AddToggleOption( options, "UseMetricDisplay", DICT( "togglemetricsystem" ), Menu_ToggleMetricSystem );
    AddToggleOption( options, "TacticalInfoTextVisible", DICT( "toggletacticalinfotext" ), Menu_TogglePOIInfoText );
    AddToggleOption( options, "CanWriteToClipboard", DICT( "toggleclipboardaccess" ), Menu_ToggleClipboardAccess );
    AddToggleOption( options, "ForceFestivals", DICT( "forcefestivals" ), Menu_ForceFestivals );
    AddToggleOption( options, "NoCategoryHiding", DICT( "nocategoryhiding" ), Menu_NoCategoryHiding, false, true );

    auto opacityMenu = options->AddItem( DICT( "markeropacity" ), 0, false, false );
    auto opacityInGame = opacityMenu->AddItem( DICT( "ingameopacity" ), 0, false, false );
    AddSetValueOption( opacityInGame, "OpacityIngame", DICT( "opacitysolid" ), Menu_OpacityIngame_Solid, 0 );
    AddSetValueOption( opacityInGame, "OpacityIngame", DICT( "opacityfaded" ), Menu_OpacityIngame_Faded, 1 );
    AddSetValueOption( opacityInGame, "OpacityIngame", DICT( "opacitytransparent" ), Menu_OpacityIngame_Transparent, 2 );

    auto opacityMiniMap = opacityMenu->AddItem( DICT( "mapopacity" ), 0, false, false );
    AddSetValueOption( opacityMiniMap, "OpacityMap", DICT( "opacitysolid" ), Menu_OpacityMap_Solid, 0 );
    AddSetValueOption( opacityMiniMap, "OpacityMap", DICT( "opacityfaded" ), Menu_OpacityMap_Faded, 1 );
    AddSetValueOption( opacityMiniMap, "OpacityMap", DICT( "opacitytransparent" ), Menu_OpacityMap_Transparent, 2 );

    auto visibilityMenu = options->AddItem( DICT( "visibilitymenu" ), 0, false, false );
    auto markerSubMenu = visibilityMenu->AddItem( DICT( "markervisibilitymenu" ), 0, false, false );
    auto markerInGameSubMenu = markerSubMenu->AddItem( DICT( "ingamevisibility" ), 0, false, false );
    AddSetValueOption( markerInGameSubMenu, "ShowInGameMarkers", DICT( "defaultvisibility" ), Menu_MarkerVisibility_InGameMap_Default, 1 );
    AddSetValueOption( markerInGameSubMenu, "ShowInGameMarkers", DICT( "forceonvisibility" ), Menu_MarkerVisibility_InGameMap_Force, 2 );
    AddSetValueOption( markerInGameSubMenu, "ShowInGameMarkers", DICT( "forceoffvisibility" ), Menu_MarkerVisibility_InGameMap_Off, 0 );
    auto markerMiniMapSubMenu = markerSubMenu->AddItem( DICT( "minimapvisibility" ), 0, false, false );
    AddSetValueOption( markerMiniMapSubMenu, "ShowMinimapMarkers", DICT( "defaultvisibility" ), Menu_MarkerVisibility_MiniMap_Default, 1 );
    AddSetValueOption( markerMiniMapSubMenu, "ShowMinimapMarkers", DICT( "forceonvisibility" ), Menu_MarkerVisibility_MiniMap_Force, 2 );
    AddSetValueOption( markerMiniMapSubMenu, "ShowMinimapMarkers", DICT( "forceoffvisibility" ), Menu_MarkerVisibility_MiniMap_Off, 0 );
    auto markerMapSubMenu = markerSubMenu->AddItem( DICT( "mapvisibility" ), 0, false, false );
    AddSetValueOption( markerMapSubMenu, "ShowBigmapMarkers", DICT( "defaultvisibility" ), Menu_MarkerVisibility_BigMap_Default, 1 );
    AddSetValueOption( markerMapSubMenu, "ShowBigmapMarkers", DICT( "forceonvisibility" ), Menu_MarkerVisibility_BigMap_Force, 2 );
    AddSetValueOption( markerMapSubMenu, "ShowBigmapMarkers", DICT( "forceoffvisibility" ), Menu_MarkerVisibility_BigMap_Off, 0 );
    auto trailSubMenu = visibilityMenu->AddItem( DICT( "trailvisibilitymenu" ), 0, false, false );
    auto trailInGameSubMenu = trailSubMenu->AddItem( DICT( "ingamevisibility" ), 0, false, false );
    AddSetValueOption( trailInGameSubMenu, "ShowInGameTrails", DICT( "defaultvisibility" ), Menu_TrailVisibility_InGameMap_Default, 1 );
    AddSetValueOption( trailInGameSubMenu, "ShowInGameTrails", DICT( "forceonvisibility" ), Menu_TrailVisibility_InGameMap_Force, 2 );
    AddSetValueOption( trailInGameSubMenu, "ShowInGameTrails", DICT( "forceoffvisibility" ), Menu_TrailVisibility_InGameMap_Off, 0 );
    auto trailMiniMapSubMenu = trailSubMenu->AddItem( DICT( "minimapvisibility" ), 0, false, false );
    AddSetValueOption( trailMiniMapSubMenu, "ShowMinimapTrails", DICT( "defaultvisibility" ), Menu_TrailVisibility_MiniMap_Default, 1 );
    AddSetValueOption( trailMiniMapSubMenu, "ShowMinimapTrails", DICT( "forceonvisibility" ), Menu_TrailVisibility_MiniMap_Force, 2 );
    AddSetValueOption( trailMiniMapSubMenu, "ShowMinimapTrails", DICT( "forceoffvisibility" ), Menu_TrailVisibility_MiniMap_Off, 0 );
    auto trailMapSubMenu = trailSubMenu->AddItem( DICT( "mapvisibility" ), 0, false, false );
    AddSetValueOption( trailMapSubMenu, "ShowBigmapTrails", DICT( "defaultvisibility" ), Menu_TrailVisibility_BigMap_Default, 1 );
    AddSetValueOption( trailMapSubMenu, "ShowBigmapTrails", DICT( "forceonvisibility" ), Menu_TrailVisibility_BigMap_Force, 2 );
    AddSetValueOption( trailMapSubMenu, "ShowBigmapTrails", DICT( "forceoffvisibility" ), Menu_TrailVisibility_BigMap_Off, 0 );

    auto utils = ctx->AddItem( DICT( "tacticalutilities" ), 0, false, false );
    utils->AddItem( DICT( "reloadmarkers" ), Menu_ReloadMarkers, false, false );
    utils->AddItem( DICT( "removemymarkers" ), 0, false, false )->AddItem( DICT( "reallyremovemarkers" ), Menu_DeleteMyMarkers );

    auto onlineMarkers = ctx->AddItem( DICT( "onlinemarkers" ), 0, false, false );
    AddToggleOption( onlineMarkers, "FetchMarkerPacks", DICT( "downloadmarkers" ), Menu_ToggleAutomaticMarkerUpdates );
    onlineMarkers->AddSeparator();

    {
      CLightweightCriticalSection cs( &dlTextCritSec );
      for ( int x = 0; x < markerPacks.NumItems(); x++ )
      {
        CString state = "";

        if ( markerPacks[ x ].beingDownloaded )
          state = DICT( "downloadingpack" );

        if ( markerPacks[ x ].versionCheckDone && !markerPacks[ x ].versionCheckOk )
          state = DICT( "packversioncheckfail" );

        if ( markerPacks[ x ].downloadFinished )
          state = DICT( "packdownloaded" );

        if ( markerPacks[ x ].failed )
          state = DICT( "downloadpackfail" );

        AddToggleOption( onlineMarkers, "MarkerPack_" + markerPacks[ x ].id + "_autoupdate", state + markerPacks[ x ].name, Menu_MarkerPacks_Base + x, false );
      }
    }

  }
  AddToggleOption( ctx, "TacticalLayerVisible", GetKeybindString( TacOKeyAction::Toggle_tactical_layer ) + DICT( "toggletactical" ), Menu_ToggleTactical, false, true );

  ctx->AddSeparator();

  if ( mumbleLink.isPvp )
  {
    ctx->AddItem( DICT( "rangecirclesnotavailable" ), 0, false, false );
  }
  else
  {
    AddToggleOption( ctx, "RangeCirclesVisible", GetKeybindString( TacOKeyAction::Toggle_range_circles ) + DICT( "togglerangecircles" ), Menu_ToggleRangeCircles );
    if ( Config::GetValue( "RangeCirclesVisible" ) )
    {
      auto trns = ctx->AddItem( DICT( "rangevisibility" ), 0, false, false );
      AddSetValueOption( trns, "RangeCircleTransparency", "40%", Menu_RangeCircleTransparency40, 40 );
      AddSetValueOption( trns, "RangeCircleTransparency", "60%", Menu_RangeCircleTransparency60, 60 );
      AddSetValueOption( trns, "RangeCircleTransparency", "100%", Menu_RangeCircleTransparency100, 100 );

      auto ranges = ctx->AddItem( DICT( "toggleranges" ), 0, false, false );
      AddToggleOption( ranges, "RangeCircle90", "90", Menu_ToggleRangeCircle90 );
      AddToggleOption( ranges, "RangeCircle120", "120", Menu_ToggleRangeCircle120 );
      AddToggleOption( ranges, "RangeCircle180", "180", Menu_ToggleRangeCircle180 );
      AddToggleOption( ranges, "RangeCircle240", "240", Menu_ToggleRangeCircle240 );
      AddToggleOption( ranges, "RangeCircle300", "300", Menu_ToggleRangeCircle300 );
      AddToggleOption( ranges, "RangeCircle400", "400", Menu_ToggleRangeCircle400 );
      AddToggleOption( ranges, "RangeCircle600", "600", Menu_ToggleRangeCircle600 );
      AddToggleOption( ranges, "RangeCircle900", "900", Menu_ToggleRangeCircle900 );
      AddToggleOption( ranges, "RangeCircle1200", "1200", Menu_ToggleRangeCircle1200 );
      AddToggleOption( ranges, "RangeCircle1500", "1500", Menu_ToggleRangeCircle1500 );
      AddToggleOption( ranges, "RangeCircle1600", "1600", Menu_ToggleRangeCircle1600 );
    }
  }

  ctx->AddSeparator();

  AddToggleOption( ctx, "TacticalCompassVisible", GetKeybindString( TacOKeyAction::Toggle_tactical_compass ) + DICT( "togglecompass" ), Menu_ToggleTacticalCompass );
  AddToggleOption( ctx, "LocationalTimersVisible", GetKeybindString( TacOKeyAction::Toggle_locational_timers ) + DICT( "toggleloctimers" ), Menu_ToggleLocationalTimers );
  AddToggleOption( ctx, "HPGridVisible", GetKeybindString( TacOKeyAction::Toggle_hp_grids ) + DICT( "togglehpgrid" ), Menu_ToggleHPGrid );

  //ctx->AddItem( GetConfigValue( "Vsync" ) ? "Toggle TacO Vsync [x]" : "Toggle TacO Vsync [ ]", Menu_ToggleVsync );

  ctx->AddSeparator();
  AddToggleOption( ctx, "MouseHighlightVisible", GetKeybindString( TacOKeyAction::Toggle_mouse_highlight ) + DICT( "togglemousehighlight" ), Menu_ToggleHighLight );
  if ( Config::GetValue( "MouseHighlightVisible" ) )
  {
    AddToggleOption( ctx, "MouseHighlightOutline", DICT( "togglemouseoutline" ), Menu_ToggleMouseHighlightOutline );

    extern CString CGAPaletteNames[];
    auto cols = ctx->AddItem( DICT( "mousecolor" ), 0, false, false );

    for ( int x = 0; x < 16; x++ )
      AddSetValueOption( cols, "MouseHighlightColor", DICT( CGAPaletteNames[ x ] ), Menu_MouseHighlightColor0 + x, x, true );
  }
  ctx->AddSeparator();

  AddTwoTextOption( ctx, Config::GetWindowOpenConfigValue( "MapTimer" ), GetKeybindString( TacOKeyAction::Toggle_map_timer ) + DICT( "openmaptimer" ), GetKeybindString( TacOKeyAction::Toggle_map_timer ) + DICT( "closemaptimer" ), Menu_ToggleMapTimer );
  if ( Config::IsWindowOpen( "MapTimer" ) )
  {
    AddToggleOption( ctx, "MapTimerCompact", DICT( "compactmaptimer" ), Menu_ToggleCompactMapTimer );

    GW2MapTimer* timer = (GW2MapTimer*)App->GetRoot()->FindChildByID( "MapTimer", "maptimer" );

    if ( timer )
    {
      auto itm = ctx->AddItem( DICT( "configmaptimer" ), 0, false, false );

      for ( TS32 x = 0; x < timer->maps.NumItems(); x++ )
        AddToggleOption( itm, CString( "maptimer_mapopen_" ) + timer->maps[ x ].id, timer->maps[ x ].name, Menu_ToggleMapTimerMap + x );
    }

  }
  ctx->AddSeparator();
  AddTwoTextOption( ctx, Config::GetWindowOpenConfigValue( "TS3Control" ), GetKeybindString( TacOKeyAction::Toggle_ts3_window ) + DICT( "opentswindow" ), GetKeybindString( TacOKeyAction::Toggle_ts3_window ) + DICT( "closetswindow" ), Menu_ToggleTS3Control );
  auto markerEditor = AddTwoTextOption( ctx, Config::GetWindowOpenConfigValue( "MarkerEditor" ), GetKeybindString( TacOKeyAction::Toggle_marker_editor ) + DICT( "openmarkereditor" ), GetKeybindString( TacOKeyAction::Toggle_marker_editor ) + DICT( "closemarkereditor" ), Menu_ToggleMarkerEditor );
  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    markerEditor->AddItem( DICT( "exportmymapmaprkers" ), Menu_ExportMyMapMarkers );
    markerEditor->AddSeparator();

    //AddToggleOption( markerEditor, "AutoHideMarkerEditor", DICT( "autohidemarkereditor" ), Menu_ToggleAutoHideMarkerEditor );
    AddToggleOption( markerEditor, "EnableExternalEditing", DICT( "enableexternalediting" ), Menu_ToggleExternalEditing );
    AddToggleOption( markerEditor, "HideExternalMarkers", DICT( "hideexternalmarkers" ), Menu_HideExternalMarkers );
    // 
    // Menu_ToggleExternalEditing
    markerEditor->AddSeparator();
    int cnt = 1;
    auto rebinds = markerEditor->AddItem( DICT( "rebindkeys" ), 0, false, false );

    for ( TS32 x = 1; x < sizeof( ActionNames ) / sizeof( CString ); x++ )
    {
      CString str = DICT( ActionNames[ x ] ) + " " + DICT( "action_no_key_bound" );
      for ( TS32 y = 0; y < KeyBindings.NumItems(); y++ )
        if ( (TS32)KeyBindings.GetByIndex( y ) == x )
        {
          str = CString::Format( "[%c] ", KeyBindings.GetKDPair( y )->Key ) + DICT( ActionNames[ x ] );
          break;
        }

      if ( ActionNames[ x ][ 0 ] == '*' )
        rebinds->AddItem( str.GetPointer(), Menu_RebindKey_Base + x );
      cnt++;
    }
  }

  AddTwoTextOption( ctx, Config::GetWindowOpenConfigValue( "Notepad" ), GetKeybindString( TacOKeyAction::Toggle_notepad ) + DICT( "opennotepad" ), GetKeybindString( TacOKeyAction::Toggle_notepad ) + DICT( "closenotepad" ), Menu_ToggleNotepad );
  ctx->AddSeparator();

  //if (teamSpeakConnection.IsConnected() && teamSpeakConnection.handlers.NumItems())
  //{
  //	TS32 connectednum = 0;
  //	for (TS32 x = 0; x < teamSpeakConnection.handlers.NumItems(); x++)
  //		if (teamSpeakConnection.handlers[x].Connected)
  //			connectednum++;

  //	if (connectednum)
  //	{
  //		auto chn = ctx->AddItem("Switch Teamspeak channel", 0);
  //		for (TS32 x = 0; x < teamSpeakConnection.handlers.NumItems(); x++)
  //			if (teamSpeakConnection.handlers[x].Connected)
  //			{
  //				auto hndlr = chn->AddItem(teamSpeakConnection.handlers[x].name.GetPointer(), 0);
  //				BuildChannelTree(teamSpeakConnection.handlers[x], hndlr, 0);
  //			}

  //		ctx->AddSeparator();
  //	}
  //}

  auto raid = AddTwoTextOption( ctx, Config::GetWindowOpenConfigValue( "RaidProgress" ), GetKeybindString( TacOKeyAction::Toggle_raid_progress ) + DICT( "openraidprogress" ), GetKeybindString( TacOKeyAction::Toggle_raid_progress ) + DICT( "closeraidprogress" ), Menu_ToggleRaidProgress );

  if ( Config::IsWindowOpen( "RaidProgress" ) )
  {
    AddToggleOption( raid, "CompactRaidWindow", DICT( "raidwindow_compact" ), Menu_ToggleCompactRaids );
    auto* rp = FindChildByID<RaidProgress>( "RaidProgressView" );
    if ( rp )
    {
      auto& raids = rp->GetRaids();
      if ( raids.NumItems() )
        raid->AddSeparator();
      for ( TS32 x = 0; x < raids.NumItems(); x++ )
      {
        if ( !Config::HasValue( raids[ x ].configName.GetPointer() ) )
          Config::SetValue( raids[ x ].configName.GetPointer(), 1 );

        AddToggleOption( raid, raids[ x ].configName, DICT( raids[ x ].configName, raids[ x ].name ), Menu_RaidToggles + x, false );
      }
    }
  }

  AddTwoTextOption( ctx, Config::GetWindowOpenConfigValue( "DungeonProgress" ), GetKeybindString( TacOKeyAction::Toggle_dungeon_progress ) + DICT( "opendungeonprogress" ), GetKeybindString( TacOKeyAction::Toggle_dungeon_progress ) + DICT( "closedungeonprogress" ), Menu_ToggleDungeonProgress );
  auto tpTracker = AddTwoTextOption( ctx, Config::GetWindowOpenConfigValue( "TPTracker" ), GetKeybindString( TacOKeyAction::Toggle_tp_tracker ) + DICT( "opentptracker" ), GetKeybindString( TacOKeyAction::Toggle_tp_tracker ) + DICT( "closetptracker" ), Menu_ToggleTPTracker );
  if ( Config::IsWindowOpen( "TPTracker" ) )
  {
    AddToggleOption( tpTracker, "TPTrackerOnlyShowOutbid", DICT( "tptracker_onlyoutbid" ), Menu_ToggleTPTracker_OnlyOutbid );
    AddToggleOption( tpTracker, "TPTrackerShowBuys", DICT( "tptracker_showbuys" ), Menu_ToggleTPTracker_ShowBuys );
    AddToggleOption( tpTracker, "TPTrackerShowSells", DICT( "tptracker_showsells" ), Menu_ToggleTPTracker_ShowSells );
    AddToggleOption( tpTracker, "TPTrackerNextSellOnly", DICT( "tptracker_nextsellonly" ), Menu_ToggleTPTracker_OnlyNextFulfilled );
  }

  ctx->AddSeparator();
  extern TBOOL isTacOUptoDate;

  auto settings = ctx->AddItem( DICT( "tacosettings" ), Menu_TacOSettings, false, false );
  AddToggleOption( settings, "EditMode", GetKeybindString( TacOKeyAction::Toggle_window_edit_mode ) + DICT( "togglewindoweditmode" ), Menu_ToggleEditMode );
  settings->AddSeparator();

  AddToggleOption( settings, "CheckForUpdates", DICT( "toggleupdatecheck" ), Menu_ToggleVersionCheck );
  AddToggleOption( settings, "HideOnLoadingScreens", DICT( "hideonload" ), Menu_HideOnLoadingScreens );
  AddToggleOption( settings, "CloseWithGW2", DICT( "closewithgw2" ), Menu_ToggleGW2ExitMode );
  AddToggleOption( settings, "InfoLineVisible", DICT( "toggleinfoline" ), Menu_ToggleInfoLine );
  AddToggleOption( settings, "ForceDPIAware", DICT( "toggleforcedpiaware" ), Menu_ToggleForceDPIAware );
  AddToggleOption( settings, "EnableTPNotificationIcon", DICT( "enabletpnotificationicon" ), Menu_ToggleShowNotificationIcon );
  AddToggleOption( settings, "TacOOnTaskBar", DICT( "tacoontaskbar" ), Menu_TacOOnTaskBar );
  AddToggleOption( settings, "SendCrashDump", DICT( "togglecrashoptout" ), Menu_OptOutFromCrashReports );

  settings->AddSeparator();
  AddToggleOption( settings, "KeybindsEnabled", DICT( "togglekeybinds" ), Menu_KeyBindsEnabled );
  auto bind = settings->AddItem( DICT( "rebindkeys" ), 0, false, false );
  int cnt = 1;
  for ( TS32 x = 1; x < sizeof( ActionNames ) / sizeof( CString ); x++ )
  {
    CString str = DICT( ActionNames[ x ] ) + " " + DICT( "action_no_key_bound" );
    for ( TS32 y = 0; y < KeyBindings.NumItems(); y++ )
      if ( (TS32)KeyBindings.GetByIndex( y ) == x )
      {
        str = DICT( ActionNames[ x ] ) + CString::Format( " [%c]", KeyBindings.GetKDPair( y )->Key );
        break;
      }

    if ( ActionNames[ x ][ 0 ] != '*' )
      bind->AddItem( str.GetPointer(), Menu_RebindKey_Base + x );
    cnt++;
  }
  /*
  if ( scriptKeyBinds.NumItems() )
  bind->AddSeparator();
  for ( TS32 x = 0; x < scriptKeyBinds.NumItems(); x++ )
  {
  CString str = scriptKeyBinds[ x ].eventDescription + " [no key bound]";
  for ( TS32 y = 0; y < ScriptKeyBindings.NumItems(); y++ )
  if ( ScriptKeyBindings.GetKDPair( y )->Data == scriptKeyBinds[ x ].eventName )
  {
  str = scriptKeyBinds[ x ].eventDescription + CString::Format( " [%c]", ScriptKeyBindings.GetKDPair( y )->Key );
  break;
  }

  bind->AddItem( str.GetPointer(), Menu_RebindKey_Base + x + cnt );
  }
  */
  settings->AddSeparator();

  //auto interfaceSize = ctx->AddItem( "Interface Size", 0 );
  //interfaceSize->AddItem( "Small", Menu_Interface_Small );
  //interfaceSize->AddItem( "Normal", Menu_Interface_Normal );
  //interfaceSize->AddItem( "Large", Menu_Interface_Large );
  //interfaceSize->AddItem( "Larger", Menu_Interface_Larger );
  //ctx->AddSeparator();
  auto apiKeys = settings->AddItem( DICT( "apikeys" ), 0, false, false );
  auto gw2keys = apiKeys->AddItem( DICT( "gw2apikey" ), 0, false, false );

  auto currKey = GW2::apiKeyManager.GetIdentifiedAPIKey();

  for ( TS32 x = 0; x < GW2::apiKeyManager.keys.NumItems(); x++ )
  {
    auto key = GW2::apiKeyManager.keys[ x ];
    auto keyMenu = gw2keys->AddItem( key->accountName.Length() ? key->accountName : key->apiKey, Menu_GW2APIKey_Base + x, key == currKey );
    keyMenu->AddItem( DICT( "deletekey" ), Menu_DeleteGW2APIKey_Base + x );
  }

  if ( GW2::apiKeyManager.keys.NumItems() )
    gw2keys->AddSeparator();

  gw2keys->AddItem( DICT( "addgw2apikey" ), Menu_AddGW2ApiKey );

  apiKeys->AddItem( DICT( "ts3controlplugin" ), Menu_TS3APIKey );

  settings->AddSeparator();

  auto languages = Localization::GetLanguages();
  auto langs = settings->AddItem( DICT( "language" ), Menu_Language, false, false );
  for ( int x = 0; x < languages.NumItems(); x++ )
    langs->AddItem( ( x == Localization::GetActiveLanguageIndex() ? CString( "[x] " ) : CString( "[ ] " ) ) + languages[ x ], Menu_Language_Base + x, x == Localization::GetActiveLanguageIndex() );


  ctx->AddSeparator();
  ctx->AddItem( DICT( "abouttaco" ), Menu_About );
  if ( !isTacOUptoDate )
    ctx->AddItem( DICT( "getnewbuild" ), Menu_DownloadNewBuild );
  ctx->AddItem( DICT( "supporttaco" ), Menu_SupportTacO, true );
  ctx->AddSeparator();
  ctx->AddSeparator();
  ctx->AddItem( DICT( "exittaco" ), Menu_Exit );

  ctx->AddSeparator();
  if ( Config::GetValue( "EnableCrashMenu" ) )
    ctx->AddItem( "CRASH", Menu_Crash );
}

void GW2TacO::RebuildMainMenu( CWBContextMenu* ctx )
{
  ctx->FlushItems();
  OpenMainMenu( ctx );
}

TBOOL GW2TacO::MessageProc( CWBMessage& Message )
{
  if ( Config::IsWindowOpen( "MarkerEditor" ) )
  {
    auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
    if ( editor && !editor->IsHidden() )
      if ( editor->HandleUberToolMessages( Message ) )
        return true;
  }

  switch ( Message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    CWBButton* cb = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "clickthroughbutton" ) );
    if ( cb && cb->GetID() == _T( "TPButton" ) )
    {
      TurnOffTPLight();
      break;
    }

    CWBButton* b = (CWBButton*)App->FindItemByGuid( Message.GetTarget(), _T( "button" ) );
    if ( !b )
      break;

    if ( b->GetID() == _T( "MenuButton" ) )
    {
      toggleOptions.Flush();

      auto ctx = b->OpenContextMenu( App->GetMousePos() );
      OpenMainMenu( ctx );

      return true;
    }
    if ( b->GetID() == _T( "GoToWebsite" ) )
    {
      ShellExecute( (HWND)App->GetHandle(), "open", "http://www.gw2taco.com", NULL, NULL, SW_SHOW );
      return true;
    }
    if ( b->GetID() == _T( "SendEmail" ) )
    {
      ShellExecute( (HWND)App->GetHandle(), "open", "mailto:boyc@scene.hu", NULL, NULL, SW_SHOW );
      return true;
    }
  }
  break;
  case WBM_REBUILDCONTEXTITEM:

    if ( toggleOptions.HasKey( (MainMenuItems)Message.Data ) )
    {
      CWBContextMenu* ctxMenu = (CWBContextMenu*)App->FindItemByGuid( Message.Position[ 1 ] );
      if ( ctxMenu )
      {
        auto& option = toggleOptions[ (MainMenuItems)Message.Data ];
        if ( option.toggle )
        {
          auto itm = ctxMenu->GetItem( Message.Data );
          if ( itm )
          {
            if ( option.activeText.Length() )
              itm->SetText( Config::GetValue( option.configOption.GetPointer() ) ? option.activeText : option.text );
            else
              itm->SetText( GetConfigActiveString( option.configOption.GetPointer() ) + option.text );
          }
        }
        else
        {
          for ( int x = 0; x < toggleOptions.NumItems(); x++ )
          {
            auto& other = toggleOptions.GetByIndex( x );
            if ( other.configOption == option.configOption )
            {
              auto itm = ctxMenu->GetItem( other.menuItem );
              if ( itm )
              {
                itm->SetText( ( GetConfigActiveString( option.configOption.GetPointer(), other.value ) + other.text ).GetPointer() );
                if ( option.doHighlight )
                  itm->SetHighlight( Config::GetValue( option.configOption.GetPointer() ) == other.value );
              }
            }
          }
        }
      }
      break;
    }


    {
      CWBContextMenu* ctxMenu = (CWBContextMenu*)App->FindItemByGuid( Message.Position[ 1 ] );
      auto category = FindInCategoryTree( (GW2TacticalCategory*)Message.Data );
      if ( category )
      {
        SetAllCategoriesToVisibleInContext( category );
        ctxMenu->FlushItems();
        AddTypeContextMenu( ctxMenu, CategoryList, category, true, Menu_MarkerFilter_Base, false );
        break;
      }
    }

    if ( Message.Data >= Menu_RaidToggles && Message.Data < Menu_RaidToggles_End )
    {
      TS32 raidToggle = Message.Data - Menu_RaidToggles;

      auto* rp = FindChildByID<RaidProgress>( "RaidProgressView" );
      if ( rp )
      {
        auto& raids = rp->GetRaids();
        if ( raidToggle < raids.NumItems() )
        {
          CWBContextMenu* ctxMenu = (CWBContextMenu*)App->FindItemByGuid( Message.Position[ 1 ] );
          auto itm = ctxMenu->GetItem( Message.Data );

          itm->SetText( ( ( Config::HasValue( raids[ raidToggle ].configName.GetPointer() ) && !Config::GetValue( raids[ raidToggle ].configName.GetPointer() ) ) ? "[ ] " : "[x] " ) + DICT( raids[ raidToggle ].configName, raids[ raidToggle ].name ) );
        }
      }
    }

    if ( Message.Data >= Menu_MarkerFilter_Base && Message.Data < Menu_MarkerFilter_Base + CategoryList.NumItems() )
    {
      CWBContextMenu* ctxMenu = (CWBContextMenu*)App->FindItemByGuid( Message.Position[ 1 ] );
      if ( ctxMenu ) // possible fix for a really weird crash reported through bugsplat
      {
        auto itm = ctxMenu->GetItem( Message.Data );

        auto& dta = CategoryList[ Message.Data - Menu_MarkerFilter_Base ];

        if ( !dta->isOnlySeparator )
        {
          CString txt = "[" + CString( dta->isDisplayed ? "x" : " " ) + "] ";
          if ( dta->displayName.Length() )
            txt += dta->displayName;
          else
            txt += dta->name;

          itm->SetText( txt );
          itm->SetHighlight( dta->isDisplayed );
        }

        //TBOOL displayed = !CategoryList[ Message.Data - Menu_MarkerFilter_Base ]->IsDisplayed;
        //CategoryList[ Message.Data - Menu_MarkerFilter_Base ]->IsDisplayed = displayed;
        //SetConfigValue( ( CString( "CategoryVisible_" ) + CategoryList[ Message.Data - Menu_MarkerFilter_Base ]->GetFullTypeName() ).GetPointer(), displayed );
      }
      break;
    }

    break;

  case WBM_CONTEXTMESSAGE:
  {
    if ( Message.GetTargetID() != "MenuButton" )
      break;

    if ( FindInCategoryTree( (GW2TacticalCategory*)Message.Data ) )
      break;

    if ( Message.Data >= Menu_GW2APIKey_Base && Message.Data < Menu_GW2APIKey_End )
    {
      TS32 idx = Message.Data - Menu_GW2APIKey_Base;
      ApiKeyInputAction( APIKeys::GW2APIKey, idx );
      return true;
    }

    if ( Message.Data >= Menu_DeleteGW2APIKey_Base && Message.Data < Menu_DeleteGW2APIKey_End )
    {
      TS32 idx = Message.Data - Menu_DeleteGW2APIKey_Base;
      GW2::apiKeyManager.keys.FreeByIndex( idx );
      GW2::apiKeyManager.RebuildConfigValues();
      return true;
    }

    if ( Message.Data >= Menu_RaidToggles && Message.Data < Menu_RaidToggles_End )
    {
      TS32 raidToggle = Message.Data - Menu_RaidToggles;

      auto* rp = FindChildByID<RaidProgress>( "RaidProgressView" );
      if ( rp )
      {
        auto& raids = rp->GetRaids();
        if ( raidToggle < raids.NumItems() )
        {
          if ( !Config::HasValue( raids[ raidToggle ].configName.GetPointer() ) )
            Config::SetValue( raids[ raidToggle ].configName.GetPointer(), 0 );
          else
            Config::ToggleValue( raids[ raidToggle ].configName.GetPointer() );
        }
      }
      break;
    }

    if ( Message.Data >= Menu_RebindKey_Base && Message.Data < Menu_RebindKey_Base + sizeof( ActionNames ) / sizeof( CString ) )
    {
      RebindAction( (TacOKeyAction)( Message.Data - Menu_RebindKey_Base ) );
      break;
    }

    /*
        if ( Message.Data >= Menu_RebindKey_Base + sizeof( ActionNames ) / sizeof( CString ) && TU32( Message.Data ) < Menu_RebindKey_Base + sizeof( ActionNames ) / sizeof( CString ) + scriptKeyBinds.NumItems() )
        {
          RebindScriptKey( Message.Data - Menu_RebindKey_Base - sizeof( ActionNames ) / sizeof( CString ) );
          break;
        }
    */

    {
      auto languages = Localization::GetLanguages();

      if ( Message.Data >= Menu_Language_Base && Message.Data < Menu_Language_Base + languages.NumItems() )
      {
        Localization::SetActiveLanguage( languages[ Message.Data - Menu_Language_Base ] );
        break;
      }
    }

    {
      if ( Message.Data >= Menu_MarkerPacks_Base && Message.Data < Menu_MarkerPacks_Base + markerPacks.NumItems() )
      {
        CString id = markerPacks[ Message.Data - Menu_MarkerPacks_Base ].id;
        CString cfgValue = "MarkerPack_" + id + "_autoupdate";
        Config::ToggleValue( cfgValue.GetPointer() );
        break;
      }
    }

    if ( Message.Data >= Menu_MarkerFilter_Base && Message.Data < Menu_MarkerFilter_Base + CategoryList.NumItems() )
    {
      TBOOL displayed = !CategoryList[ Message.Data - Menu_MarkerFilter_Base ]->isDisplayed;
      CategoryList[ Message.Data - Menu_MarkerFilter_Base ]->isDisplayed = displayed;
      Config::SetValue( ( CString( "CategoryVisible_" ) + CategoryList[ Message.Data - Menu_MarkerFilter_Base ]->GetFullTypeName() ).GetPointer(), displayed );
      CategoryRoot.CalculateVisibilityCache();
      break;
    }

    if ( Message.Data >= Menu_ToggleMapTimerMap )
    {
      GW2MapTimer* timer = (GW2MapTimer*)App->GetRoot()->FindChildByID( "MapTimer", "maptimer" );

      if ( timer )
      {
        if ( Message.Data < Menu_ToggleMapTimerMap + timer->maps.NumItems() )
        {
          TS32 mapIdx = Message.Data - Menu_ToggleMapTimerMap;
          CString str = CString( "maptimer_mapopen_" ) + timer->maps[ mapIdx ].id;
          timer->maps[ mapIdx ].display = !timer->maps[ mapIdx ].display;
          Config::SetValue( str.GetPointer(), timer->maps[ mapIdx ].display );
          break;
        }
      }
    }

    bool needsRebuild = false;
    CWBContextMenu* ctxMenu = nullptr;

    if ( toggleOptions.HasKey( Message.Data ) )
    {
      auto& opts = toggleOptions[ Message.Data ];
      if ( !opts.activeText.Length() )
      {
        if ( opts.toggle )
          Config::ToggleValue( opts.configOption );
        else
          Config::SetValue( opts.configOption.GetPointer(), opts.value );
      }
      if ( opts.rebuildMenu )
      {
        ctxMenu = (CWBContextMenu*)( App->FindItemByGuid( Message.Position[ 1 ] ) );
        if ( ctxMenu )
        {
          while ( ctxMenu->GetParentMenu() )
            ctxMenu = ctxMenu->GetParentMenu();
          needsRebuild = true;
        }
      }
    }

    switch ( Message.Data )
    {
    case Menu_Exit:
      GetApplication()->SetDone( true );
      break;
    case Menu_About:
      OpenAboutWindow();
      break;
    case Menu_ToggleMapTimer:
      OpenWindow( "MapTimer" );
      break;
    case Menu_ToggleTS3Control:
      OpenWindow( "TS3Control" );
      break;
    case Menu_ToggleMarkerEditor:
      OpenWindow( "MarkerEditor" );
      break;
    case Menu_ToggleNotepad:
      OpenWindow( "Notepad" );
      break;
    case Menu_ToggleRaidProgress:
      OpenWindow( "RaidProgress" );
      break;
    case Menu_ToggleDungeonProgress:
      OpenWindow( "DungeonProgress" );
      break;
    case Menu_ToggleTPTracker:
      OpenWindow( "TPTracker" );
      break;
      //case Menu_Interface_Small:
    //case Menu_Interface_Normal:
    //case Menu_Interface_Large:
    //case Menu_Interface_Larger:
    //  if ( App->LoadCSSFromFile( UIFileNames[ Message.Data - Menu_Interface_Small ], true ) )
    //    SetConfigValue( "InterfaceSize", Message.Data - Menu_Interface_Small );
    //  App->ReApplyStyle();
    //  break;
    //  break;
    case Menu_DownloadNewBuild:
      ShellExecute( (HWND)App->GetHandle(), "open", "http://www.gw2taco.com", NULL, NULL, SW_SHOW );
      break;
    case Menu_ToggleVsync:
      Config::ToggleValue( "Vsync" );
      App->SetVSync( Config::GetValue( "Vsync" ) );
      break;
    case Menu_ForceFestivals:
      CheckFestivalActive();
      break;
    case Menu_Crash:
    {
      for ( int x = 0; x < 0xbadc0de; x++ )
        ( *(unsigned int*)x ) = 0xbaadf00d;
      int z = 15 / ( (int)sin( 0 ) );
      break;
    }
    case Menu_TS3APIKey:
      ApiKeyInputAction( APIKeys::TS3APIKey, 0 );
      break;
    case Menu_ReloadMarkers:
      ImportPOIS( GetApplication() );
      break;
    case Menu_DeleteMyMarkers:
    {
      GW2TacticalDisplay* tactical = (GW2TacticalDisplay*)GetApplication()->GetRoot()->FindChildByID( "tactical", "gw2tactical" );
      if ( tactical )
        tactical->RemoveUserMarkersFromMap();
      break;
    }
    case Menu_ExportMyMapMarkers:
      ExportMyMapMarkers();
      break;

    case Menu_SupportTacO:
    {
      CString string( "aHR0cHM6Ly93d3cucGF5cGFsLmNvbS9jZ2ktYmluL3dlYnNjcj9jbWQ9X2RvbmF0aW9ucyZidXNpbmVzcz1ib3ljQHNjZW5l"
                      "Lmh1JmxjPVVTJml0ZW1fbmFtZT1HVzIrVGFjTytEZXZlbG9wbWVudCtTdXBwb3J0Jm5vX25vdGU9MCZjbj0mY3VycmVuY3lf"
                      "Y29kZT1VU0QmYm49UFAtRG9uYXRpb25zQkY6YnRuX2RvbmF0ZUNDX0xHLmdpZjpOb25Ib3N0ZWQ=" );

      TU8* data = nullptr;
      TS32 dataLength = 0;

      string.DecodeBase64( data, dataLength );
      if ( data )
      {
        ShellExecuteA( (HWND)App->GetHandle(), "open", (LPCTSTR)data, 0, 0, 5 );
        delete[] data;
      }
    }

    break;
    case Menu_AddGW2ApiKey:
    {
      GW2::APIKey* newKey = new GW2::APIKey();
      GW2::apiKeyManager.keys += newKey;
      ApiKeyInputAction( APIKeys::GW2APIKey, GW2::apiKeyManager.keys.NumItems() - 1 );
      break;
    }
    break;
    }

    if ( needsRebuild && ctxMenu )
      RebuildMainMenu( ctxMenu );

    break;
  }
  case WBM_CHAR:
    if ( RebindMode )
    {
      if ( !ScriptRebindMode )
      {
        for ( TS32 x = 0; x < KeyBindings.NumItems(); x++ )
        {
          if ( KeyBindings.GetByIndex( x ) == ActionToRebind )
          {
            Config::DeleteKeyBinding( KeyBindings.GetKDPair( x )->Key );
            KeyBindings.DeleteByIndex( x );
            x--;
          }
        }

        if ( Message.Key != VK_ESCAPE )
        {
          KeyBindings[ Message.Key ] = ActionToRebind;
          Config::SetKeyBinding( ActionToRebind, Message.Key );
        }
      }
      else
      {
        /*
                for ( TS32 x = 0; x < ScriptKeyBindings.NumItems(); x++ )
                {
                  if ( ScriptKeyBindings.GetByIndex( x ) == scriptKeyBinds[ ScriptActionToRebind ].eventName )
                  {
                    DeleteScriptKeyBinding( scriptKeyBinds[ ScriptActionToRebind ].eventName );
                    ScriptKeyBindings.DeleteByIndex( x );
                    x--;
                  }
                }

                ScriptKeyBindings[ Message.Key ] = scriptKeyBinds[ ScriptActionToRebind ].eventName;
                SetScriptKeyBinding( scriptKeyBinds[ ScriptActionToRebind ].eventName, Message.Key );
        */
      }

      RebindMode = false;
      ScriptRebindMode = false;
      return true;
    }

    if ( Config::GetValue( "KeybindsEnabled" ) && KeyBindings.HasKey( Message.Key ) )
    {
      switch ( KeyBindings[ Message.Key ] )
      {
      case TacOKeyAction::AddPOI:
        AddPOI( App, 0 );
        return true;
      case TacOKeyAction::AddDefaultPOI_1:
      case TacOKeyAction::AddDefaultPOI_2:
      case TacOKeyAction::AddDefaultPOI_3:
      case TacOKeyAction::AddDefaultPOI_4:
        AddPOI( App, (int)KeyBindings[ Message.Key ] - (int)TacOKeyAction::AddDefaultPOI_1 + 1 );
        return true;
      case TacOKeyAction::DeleteSelectedMarker:
      {
        if ( Config::IsWindowOpen( "MarkerEditor" ) )
        {
          auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
          if ( editor && !editor->IsHidden() )
            editor->DeleteSelectedMarker();
        }
        return true;
      }
      case TacOKeyAction::DeleteSelectedTrailSegment:
      {
        if ( Config::IsWindowOpen( "MarkerEditor" ) )
        {
          auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
          if ( editor && !editor->IsHidden() )
            editor->DeleteSelectedTrailSegment();
        }
        return true;
      }
      case TacOKeyAction::CopySelectedMarker:
      {
        if ( Config::IsWindowOpen( "MarkerEditor" ) )
        {
          auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
          if ( editor && !editor->IsHidden() )
            editor->CopySelectedMarker();
        }
      }
      case TacOKeyAction::RemovePOI:
        DeletePOI();
        return true;
      case TacOKeyAction::ActivatePOI:
      {
        UpdatePOI( App );
        return true;
      }
      case TacOKeyAction::EditNotepad:
      {
        GW2Notepad* d = (GW2Notepad*)FindChildByID( _T( "notepad" ), _T( "notepad" ) );
        if ( d )
        {
          d->StartEdit();
          return true;
        }
        return true;
      }
      case TacOKeyAction::StartTrailRec:
      {
        CWBButton* startTrail = (CWBButton*)App->GetRoot()->FindChildByID( _T( "starttrail" ), _T( "button" ) );
        GW2TrailDisplay* trails = (GW2TrailDisplay*)App->GetRoot()->FindChildByID( _T( "trail" ), _T( "gw2Trails" ) );
        if ( startTrail && trails )
        {
          //startTrail->Push( !startTrail->IsPushed() );
          App->SendMessage( CWBMessage( App, WBM_COMMAND, startTrail->GetGuid() ) );
        }
      }
      return true;
      case TacOKeyAction::PauseTrailRec:
      {
        CWBButton* pauseTrail = (CWBButton*)App->GetRoot()->FindChildByID( _T( "pausetrail" ), _T( "button" ) );
        GW2TrailDisplay* trails = (GW2TrailDisplay*)App->GetRoot()->FindChildByID( _T( "trail" ), _T( "gw2Trails" ) );
        if ( pauseTrail && trails )
        {
          //pauseTrail->Push( !pauseTrail->IsPushed() );
          App->SendMessage( CWBMessage( App, WBM_COMMAND, pauseTrail->GetGuid() ) );
        }
      }
      return true;
      case TacOKeyAction::DeleteLastTrailSegment:
      {
        GW2TrailDisplay* trails = (GW2TrailDisplay*)App->GetRoot()->FindChildByID( _T( "trail" ), _T( "gw2Trails" ) );
        if ( trails )
          trails->DeleteLastTrailSegment();
      }
      return true;
      case TacOKeyAction::ResumeTrailAndCreateNewSection:
      {
        CWBButton* pauseTrail = (CWBButton*)App->GetRoot()->FindChildByID( _T( "startnewsection" ), _T( "button" ) );
        GW2TrailDisplay* trails = (GW2TrailDisplay*)App->GetRoot()->FindChildByID( _T( "trail" ), _T( "gw2Trails" ) );
        if ( pauseTrail && !pauseTrail->IsHidden() && trails )
          App->SendMessage( CWBMessage( App, WBM_COMMAND, pauseTrail->GetGuid() ) );
      }
      return true;
      case TacOKeyAction::Toggle_tactical_layer:
        Config::ToggleValue( "TacticalLayerVisible" );
        return true;
      case TacOKeyAction::Toggle_range_circles:
        Config::ToggleValue( "RangeCirclesVisible" );
        return true;
      case TacOKeyAction::Toggle_tactical_compass:
        Config::ToggleValue( "TacticalCompassVisible" );
        return true;
      case TacOKeyAction::Toggle_locational_timers:
        Config::ToggleValue( "LocationalTimersVisible" );
        return true;
      case TacOKeyAction::Toggle_hp_grids:
        Config::ToggleValue( "HPGridVisible" );
        return true;
      case TacOKeyAction::Toggle_mouse_highlight:
        Config::ToggleValue( "MouseHighlightVisible" );
        return true;
      case TacOKeyAction::Toggle_map_timer:
        OpenWindow( "MapTimer" );
        return true;
      case TacOKeyAction::Toggle_ts3_window:
        OpenWindow( "TS3Control" );
        return true;
      case TacOKeyAction::Toggle_marker_editor:
        OpenWindow( "MarkerEditor" );
        return true;
      case TacOKeyAction::Toggle_notepad:
        OpenWindow( "Notepad" );
        return true;
      case TacOKeyAction::Toggle_raid_progress:
        OpenWindow( "RaidProgress" );
        return true;
      case TacOKeyAction::Toggle_dungeon_progress:
        OpenWindow( "DungeonProgress" );
        return true;
      case TacOKeyAction::Toggle_tp_tracker:
        OpenWindow( "TPTracker" );
        return true;
      case TacOKeyAction::Toggle_window_edit_mode:
        Config::ToggleValue( "EditMode" );
        return true;

      }
    }

    if ( ScriptKeyBindings.HasKey( Message.Key ) )
      TriggerScriptEngineKeyEvent( ScriptKeyBindings[ Message.Key ] );

    break;
  case WBM_FOCUSLOST:
    if ( Message.GetTarget() == GetGuid() )
    {
      RebindMode = false;
      ScriptRebindMode = false;
    }
    if ( APIKeyInput && Message.GetTarget() == APIKeyInput->GetGuid() )
    {
      ApiKeyInputMode = false;
      switch ( ApiKeyToSet )
      {
      case APIKeys::None:
        break;
      case APIKeys::TS3APIKey:
        Config::SetString( "TS3APIKey", APIKeyInput->GetText() );
        break;
      case APIKeys::GW2APIKey:
      {
        GW2::apiKeyManager.keys[ ApiKeyIndex ]->SetKey( APIKeyInput->GetText() );
        GW2::apiKeyManager.keys[ ApiKeyIndex ]->FetchData();
        GW2::apiKeyManager.RebuildConfigValues();
      }
      break;
      default:
        break;
      }
      SAFEDELETE( APIKeyInput );
      return true;
    }
    break;
  default:
    break;
  }

  return CWBItem::MessageProc( Message );
}

void GW2TacO::SetInfoLine( const CString& string )
{
  lastInfoLine = string;
}

void GW2TacO::SetMouseToolTip( const CString& toolTip )
{
  mouseToolTip = toolTip;
}

void GW2TacO::InitScriptEngines()
{
  /*
    AngelWrapper* scriptEngine = new AngelWrapper();

    scriptEngines.Add( scriptEngine );
    scriptEngines[ 0 ]->AddScriptSection( "test.angel" );
    scriptEngines[ 0 ]->BuildScript();

    for ( TS32 x = 0; x < scriptEngines.NumItems(); x++ )
      scriptEngines[ x ]->InitScript();
  */
}

void GW2TacO::TickScriptEngines()
{
  /*
    for ( TS32 x = 0; x < scriptEngines.NumItems(); x++ )
      scriptEngines[ x ]->CallScriptTick();
  */
}

void GW2TacO::TriggerScriptEngineAction( GUID& guid )
{
  /*
    for ( TS32 x = 0; x < scriptEngines.NumItems(); x++ )
      scriptEngines[ x ]->TriggerAction( guid );
  */
}

void GW2TacO::TriggerScriptEngineKeyEvent( const CString& eventID )
{
  /*
    for ( TS32 x = 0; x < scriptEngines.NumItems(); x++ )
      scriptEngines[ x ]->TriggerKeyPress( eventID );
  */
}

void GW2TacO::OpenAboutWindow()
{
  auto child = FindChildByID( "About", "window" );
  if ( child ) return;

  CPoint cl = GetClientRect().Center();

  CWBWindow* w = new CWBWindow( this, CRect( cl - CPoint( 180, 160 ), cl + CPoint( 180, 50 + 26 ) ), "About GW2 TacO" );
  w->SetID( "About" );

  w->ReapplyStyles();

  CWBLabel* l1 = new CWBLabel( w, w->GetClientRect() + CPoint( 0, 2 ), "GW2 TacO - The Guild Wars 2 Tactical Overlay" );
  l1->ApplyStyleDeclarations( "font-family:ProFont;text-align:center;vertical-align:top;" );
  extern CString tacoBuild;
  l1 = new CWBLabel( w, w->GetClientRect() + CPoint( 0, 16 ), CString( "Build " + TacOBuild + " built on " + buildDateTime ).GetPointer() );
  l1->ApplyStyleDeclarations( "font-family:ProFont;text-align:center;vertical-align:top;" );
  l1 = new CWBLabel( w, w->GetClientRect() + CPoint( 0, 32 ), "(c) BoyC / Conspiracy" );
  l1->ApplyStyleDeclarations( "font-family:ProFont;text-align:center;vertical-align:top;" );
  l1 = new CWBLabel( w, w->GetClientRect() + CPoint( 0, 48 ), "Taco Icon from http://icons8.com" );
  l1->ApplyStyleDeclarations( "font-family:ProFont;text-align:center;vertical-align:top;" );
  l1 = new CWBLabel( w, w->GetClientRect() + CPoint( 0, 64 ), "Crash tracking by www.bugsplat.com" );
  l1->ApplyStyleDeclarations( "font-family:ProFont;text-align:center;vertical-align:top;" );
  l1 = new CWBLabel( w, w->GetClientRect() + CPoint( 0, 80 ), "If you like TacO, send some Mystic Coins to BoyC.2653 :)" );
  l1->ApplyStyleDeclarations( "font-family:ProFont;text-align:center;vertical-align:top;" );

  auto TacoIcon = new CWBButton( w, CRect( -50, -40 + 16, 50, 72 + 16 ) + w->GetClientRect().Center() );
  TacoIcon->ApplyStyleDeclarations( "background-color:none;background: skin(TacoIcon) center middle;" );

  TS32 width = w->GetClientRect().Width();
  TS32 height = w->GetClientRect().Height();

  auto WebsiteButton = new CWBButton( w, CRect( 3, height - 25, width / 2 - 1, height - 3 ), "WebSite" );
  WebsiteButton->SetID( "GoToWebsite" );
  WebsiteButton->ApplyStyleDeclarations( "font-family:ProFont;" );

  auto ContactButton = new CWBButton( w, CRect( width / 2 + 2, height - 25, width - 3, height - 3 ), "email: boyc@scene.hu" );
  ContactButton->SetID( "SendEmail" );
  ContactButton->ApplyStyleDeclarations( "font-family:ProFont;" );

  //CWBTextBox *tb = new CWBTextBox(w, w->GetClientRect());
}

float GetWindowTooSmallScale()
{
  extern CWBApplication* App;

  if ( !App || !App->GetRoot() )
    return 1.0f;

  CRect rect = App->GetRoot()->GetClientRect();

  if ( rect.Width() < 1024 || rect.Height() < 768 )
  {
    float xScale = rect.Width() / 1024.0f;
    float yScale = rect.Height() / 768.0f;
    return min( xScale, yScale );
  }

  return 1.0f;
}

CVector3 camDirArray[ 4096 ]{};
int camDirIdx = 0;

void GW2TacO::OnDraw( CWBDrawAPI* API )
{
  //API->DrawRect( GetClientRect(), CColor( 0, 0, 0, 255 ) );

  mouseToolTip = "";

  float windowTooSmallScale = GetWindowTooSmallScale();
  if ( windowTooSmallScale != lastScaleValue || scaleCountDownHack == 0 )
  {
    StoreIconSizes();
    AdjustMenuForWindowTooSmallScale( windowTooSmallScale );
    lastScaleValue = windowTooSmallScale;
  }
  scaleCountDownHack--;

  if ( Config::IsWindowOpen( "TS3Control" ) )
    teamSpeakConnection.Tick();
  CheckItemPickup();

  //auto style = GetWindowLong((HWND)App->GetHandle(), GWL_EXSTYLE);

  //CWBItem *it = App->GetFocusItem();
  //if (it && it->InstanceOf("textbox"))
  //{
  //	if (style&WS_EX_TRANSPARENT)
  //	{
  //		SetWindowLong((HWND)App->GetHandle(), GWL_EXSTYLE, style & (~WS_EX_TRANSPARENT));
  //		SetForegroundWindow((HWND)App->GetHandle());
  //	}
  //}
  //else
  //{
  //	if (!(style&WS_EX_TRANSPARENT))
  //	{
  //		LOG_ERR("Changing back!",
  //		SetWindowLong((HWND)App->GetHandle(), GWL_EXSTYLE, style | WS_EX_TRANSPARENT);
  //	}
  //}

  auto it = FindChildByID( _T( "MenuHoverBox" ) );
  if ( it )
  {
    auto taco = (CWBButton*)FindChildByID( _T( "MenuButton" ), _T( "button" ) );
    if ( taco )
    {
#define speed 500.0f

      TS32 currTime = globalTimer.GetTime();
      TF32 delta = max( 0, min( 1, ( currTime - lastMenuHoverTransitionTime ) / speed ) );

      TBOOL hover = ClientToScreen( it->GetClientRect() ).Contains( App->GetMousePos() );

      if ( App->GetRoot()->FindChildByID( "TacOMenu", "contextmenu" ) )
      {
        hover = true;
        taco->Push( true );
      }
      else
        taco->Push( false );

      if ( hover != menuHoverLastFrame )
      {
        lastMenuHoverTransitionTime = TS32( currTime - ( 1 - delta ) * speed );
        delta = 1 - delta;
      }

      TF32 col = 1 - delta * 0.5f;
      if ( hover )
        col = 0.5f + delta * 0.5f;

      TS32 o = (TS32)max( 0, min( 255, col * 255 ) );

      //taco->ApplyStyleDeclarations( CString::Format( "opacity:%f", col ) );

      taco->SetDisplayProperty( WB_STATE_NORMAL, WB_ITEM_OPACITY, CColor::FromARGB( o * 0x01010101 ) );
      taco->SetDisplayProperty( WB_STATE_ACTIVE, WB_ITEM_OPACITY, CColor::FromARGB( o * 0x01010101 ) );
      taco->SetDisplayProperty( WB_STATE_HOVER, WB_ITEM_OPACITY, CColor::FromARGB( o * 0x01010101 ) );

      menuHoverLastFrame = hover;
    }
  }

  auto tpFlairButton = FindChildByID( _T( "RedCircle" ) );
  if ( tpFlairButton && showPickupHighlight && Config::GetValue( "EnableTPNotificationIcon" ) )
  {
    CRect r = tpFlairButton->ClientToScreen( tpFlairButton->GetClientRect() );
    auto& dd = tpFlairButton->GetDisplayDescriptor();
    auto skin = dd.GetSkin( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE );
    CWBSkinElement* e = App->GetSkin()->GetElement( skin );
    if ( e )
    {
      API->DrawAtlasElementRotated( e->GetHandle(), r, 0x80ffffff, globalTimer.GetTime() / 1000.0f );
      API->DrawAtlasElementRotated( e->GetHandle(), r, 0x80ffffff, -( globalTimer.GetTime() / 1000.0f ) );
    }
  }

  int ypos = 0;

  if ( Config::GetValue( "InfoLineVisible" ) )
  {
    auto font = App->GetFont( "ProFontOutlined" );
    if ( !font ) return;

    CString infoline = lastInfoLine;

    if ( !lastInfoLine.Length() )
    {
      int idx{};
      CVector3 pos;

      if ( Config::IsWindowOpen( "MarkerEditor" ) )
      {
        auto editor = App->GetRoot()->FindChildByID<GW2MarkerEditor>( "MarkerEditor" );
        if ( editor && !editor->IsHidden() )
        {
          idx = editor->GetSelectedVertexIndex();
          pos = editor->GetSelectedVertexPosition();
        }
      }


      infoline = CString::Format( "map: %d world: %d shard: %d position: %f %f %f campos: %.2f %.2f %.2f game fps: %.2f overlay fps: %.2f map:%d compPos:%d compRot:%d cW:%d cH:%d cR:%f pX:%f pY:%f mcX:%f mcY:%f mS:%f idx: %d pos: %f %f %f",
                                  mumbleLink.mapID, mumbleLink.worldID, mumbleLink.mapInstance, mumbleLink.charPosition.x, mumbleLink.charPosition.y, mumbleLink.charPosition.z, mumbleLink.camDir.x, mumbleLink.camDir.y, mumbleLink.camDir.z,
                                  mumbleLink.GetFrameRate(), App->GetFrameRate(), int( mumbleLink.isMapOpen ), int( mumbleLink.isMinimapTopRight ), int( mumbleLink.isMinimapRotating ), int( mumbleLink.miniMap.compassWidth ), int( mumbleLink.miniMap.compassHeight ), mumbleLink.miniMap.compassRotation, mumbleLink.miniMap.playerX, mumbleLink.miniMap.playerY,
                                  mumbleLink.miniMap.mapCenterX, mumbleLink.miniMap.mapCenterY, mumbleLink.miniMap.mapScale, idx, pos.x, pos.y, pos.z );

      if ( Config::GetValue( "CircleCalc_enabled" ) )
      {
        CVector3 minvals;
        CVector3 maxvals;
        bool initialized = false;

        auto& POIs = GetMapPOIs();

        for ( int x = 0; x < POIs.NumItems(); x++ )
        {
          auto& p = POIs.GetByIndex( x );

          if ( p.mapID == mumbleLink.mapID && !p.category )
          {
            if ( !initialized )
            {
              minvals = maxvals = p.position;
              initialized = true;
            }
            else
            {
              minvals.x = min( p.position.x, minvals.x );
              minvals.y = min( p.position.y, minvals.y );
              minvals.z = min( p.position.z, minvals.z );
              maxvals.x = max( p.position.x, maxvals.x );
              maxvals.y = max( p.position.y, maxvals.y );
              maxvals.z = max( p.position.z, maxvals.z );
            }
          }
        }

        float maxdistance2d = 0;
        float maxdistance3d = 0;
        CVector3 center = ( maxvals + minvals ) * 0.5f;

        for ( int x = 0; x < POIs.NumItems(); x++ )
        {
          auto& p = POIs.GetByIndex( x );

          if ( p.mapID == mumbleLink.mapID && !p.category )
          {
            CVector3 d = p.position - center;

            maxdistance3d = max( d.Length(), maxdistance3d );
            d.y = 0;
            maxdistance2d = max( d.Length(), maxdistance2d );
          }
        }

        float playerdist = /*WorldToGameCoords*/( ( center - mumbleLink.charPosition ).Length() );
        maxdistance2d = /*WorldToGameCoords*/( maxdistance2d );
        maxdistance3d = /*WorldToGameCoords*/( maxdistance3d );

        infoline = CString::Format( "map: %d markercenter: %.2f %.2f %.2f maxdist2d: %.2f maxdist3d: %.2f playerdist: %.2f", mumbleLink.mapID, center.x, center.y, center.z, maxdistance2d, maxdistance3d, playerdist );
      }
    }

    CPoint startpos = font->GetTextPosition( infoline, GetClientRect(), WBTA_CENTERX, WBTA_TOP, WBTT_UPPERCASE );

    font->Write( API, infoline, startpos, 0xffffffff, WBTT_UPPERCASE, true );
    ypos += font->GetLineHeight();
  }

  extern TBOOL isTacOUptoDate;
  extern int newTacOVersion;

  CString currDownload = GetCurrentDownload();
  if ( currDownload.Length() )
  {
    auto font = App->GetFont( "UniFontOutlined" );
    if ( !font ) return;

    CString dlText = DICT( "downloadingmarkerpack" ) + " " + currDownload;

    CPoint startpos = font->GetTextPosition( dlText, GetClientRect(), WBTA_CENTERX, WBTA_TOP, WBTT_UPPERCASE );
    startpos.y += ypos;

    font->Write( API, dlText, startpos, 0xffffffff, WBTT_UPPERCASE, true );
    ypos += font->GetLineHeight();
  }

  if ( !isTacOUptoDate )
  {
    auto font = App->GetFont( "UniFontOutlined" );
    if ( !font ) return;

    CString infoline = DICT( "new_build_txt1" ) + CString::Format( " %d ", newTacOVersion - RELEASECOUNT ) + DICT( "new_build_txt2" );

    CPoint startpos = font->GetTextPosition( infoline, GetClientRect(), WBTA_CENTERX, WBTA_TOP, WBTT_UPPERCASE );
    startpos.y += ypos;

    font->Write( API, infoline, startpos, 0xffffffff, WBTT_UPPERCASE, true );
    ypos += font->GetLineHeight();

    TU8* data2 = nullptr;
    TS32 size = 0;
    buildText2.DecodeBase64( data2, size );
    CString build( (TS8*)data2, size );
    SAFEDELETEA( data2 );

    CPoint spos2 = font->GetTextPosition( build, GetClientRect(), WBTA_CENTERX, WBTA_TOP, WBTT_UPPERCASE );

    font->Write( API, build, CPoint( spos2.x, startpos.y + font->GetLineHeight() ), 0xffffffff, WBTT_UPPERCASE, true );
    ypos += font->GetLineHeight();
  }

  extern int gw2WindowCount;
  if ( gw2WindowCount > 1 )
  {
    auto font = App->GetFont( "UniFontOutlined" );
    if ( !font ) return;

    CString infoline = DICT( "multiclientwarning" );
    CPoint spos2 = font->GetTextPosition( infoline, GetClientRect(), WBTA_CENTERX, WBTA_TOP, WBTT_UPPERCASE );

    font->Write( API, infoline, CPoint( spos2.x, ypos ), 0xffff4040, WBTT_UPPERCASE, true );
    ypos += font->GetLineHeight();
  }

  if ( RebindMode )
  {
    API->DrawRect( GetClientRect(), 0x60000000 );
    CWBFont* f = GetFont( GetState() );

    CString line1;

    if ( !ScriptRebindMode )
    {
      TS32 key = -1;
      for ( TS32 x = 0; x < KeyBindings.NumItems(); x++ )
        if ( KeyBindings.GetByIndex( x ) == ActionToRebind )
        {
          key = KeyBindings.GetKDPair( x )->Key;
          break;
        }

      if ( key == -1 )
      {
        line1 = DICT( "action" ) + " '" + DICT( ActionNames[ (TS32)ActionToRebind ] ) + "' " + DICT( "currently_not_bound" );
      }
      else
      {
        line1 = DICT( "action" ) + " '" + DICT( ActionNames[ (TS32)ActionToRebind ] ) + "' " + DICT( "currently_bound" ) + CString::Format( " '%c'", key );
      }
    }
    else
    {
      if ( ScriptActionToRebind < 0 /*|| ScriptActionToRebind >= scriptKeyBinds.NumItems()*/ )
      {
        RebindMode = false;
        ScriptRebindMode = false;
      }
      else
      {
        /*
                TS32 key = 0;
                for ( TS32 x = 0; x < ScriptKeyBindings.NumItems(); x++ )
                  if ( ScriptKeyBindings.GetKDPair( x )->Data == scriptKeyBinds[ ScriptActionToRebind ].eventName )
                  {
                    key = ScriptKeyBindings.GetKDPair( x )->Key;
                    break;
                  }
                line1 = CString( "Action '" ) + scriptKeyBinds[ ScriptActionToRebind ].eventDescription + CString::Format( "' currently bound to key '%c'", key );
        */
      }
    }
    CString line2 = DICT( "press_to_bind" );
    CString line3 = DICT( "escape_to_unbind" );
    CPoint line1p = f->GetTextPosition( line1, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
    CPoint line2p = f->GetTextPosition( line2, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
    CPoint line3p = f->GetTextPosition( line3, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
    f->Write( API, line1, line1p - CPoint( 0, f->GetLineHeight() / 2 ) );
    f->Write( API, line2, line2p - CPoint( 0, f->GetLineHeight() / 2 ) + CPoint( 0, f->GetLineHeight() ) );
    f->Write( API, line3, line3p - CPoint( 0, f->GetLineHeight() / 2 ) + CPoint( 0, 2 * f->GetLineHeight() ) );
  }

  if ( ApiKeyInputMode )
  {
    API->DrawRect( GetClientRect(), 0x60000000 );
    CWBFont* f = GetFont( GetState() );

    CString line1 = DICT( "enter_api" ) + " " + DICT( APIKeyNames[ (TS32)ApiKeyToSet ] ) + " " + DICT( "below_and_press" );
    CPoint line1p = f->GetTextPosition( line1, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );

    if ( ApiKeyToSet == APIKeys::TS3APIKey )
    {
      CString line2 = DICT( "ts3_help_1" );
      CString line3 = DICT( "ts3_help_2" );
      CPoint line2p = f->GetTextPosition( line2, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
      CPoint line3p = f->GetTextPosition( line3, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );

      f->Write( API, line2, line2p - CPoint( 0, f->GetLineHeight() / 2 ) + CPoint( 0, f->GetLineHeight() * 3 ) );
      f->Write( API, line3, line3p - CPoint( 0, f->GetLineHeight() / 2 ) + CPoint( 0, f->GetLineHeight() * 4 ) );
    }

    if ( ApiKeyToSet == APIKeys::GW2APIKey )
    {
      CString line2 = DICT( "gw2_api_help_1" );
      CString line3 = CString( "https://account.arena.net/applications" );
      CPoint line2p = f->GetTextPosition( line2, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );
      CPoint line3p = f->GetTextPosition( line3, GetClientRect(), WBTA_CENTERX, WBTA_CENTERY, WBTT_NONE, true );

      f->Write( API, line2, line2p - CPoint( 0, f->GetLineHeight() / 2 ) + CPoint( 0, f->GetLineHeight() * 3 ) );
      f->Write( API, line3, line3p - CPoint( 0, f->GetLineHeight() / 2 ) + CPoint( 0, f->GetLineHeight() * 4 ) );
    }

    f->Write( API, line1, line1p - CPoint( 0, f->GetLineHeight() / 2 ) );
  }

/*
  memmove( camDirArray, camDirArray + 1, sizeof( camDirArray ) - sizeof( CVector3 ) );
  camDirArray[ GetClientRect().Width() ] = mumbleLink.camDir.Normalized();

  int yp = GetClientRect().Height() / 2;

  for ( int x = 0; x < GetClientRect().Width(); x++ )
  {
    API->DrawLine( CPoint( x, yp - camDirArray[ x ].x * yp / 2 ), CPoint( x + 1, yp - camDirArray[ x + 1 ].x * yp / 2 ), CColor( 255, 0, 0, 255 ) );
    API->DrawLine( CPoint( x, yp - camDirArray[ x ].y * yp / 2 ), CPoint( x + 1, yp - camDirArray[ x + 1 ].y * yp / 2 ), CColor( 0, 255, 0, 255 ) );
    API->DrawLine( CPoint( x, yp - camDirArray[ x ].z * yp / 2 ), CPoint( x + 1, yp - camDirArray[ x + 1 ].z * yp / 2 ), CColor( 0, 0, 255, 255 ) );
  }
*/
}

void SetMouseToolTip( const CString& toolTip )
{
  extern CWBApplication* App;

  if ( !App )
    return;

  GW2TacO* tacoRoot = (GW2TacO*)App->GetRoot()->FindChildByID( "tacoroot", "GW2TacO" );
  if ( !tacoRoot )
    return;

  tacoRoot->SetMouseToolTip( toolTip );
}

void GW2TacO::OnPostDraw( CWBDrawAPI* API )
{
  CWBFont* font = GetApplication()->GetRoot()->GetFont( WB_STATE_NORMAL );

  if ( !font )
    return;

  if ( !mouseToolTip.Length() )
    return;

  TS32 width = font->GetWidth( mouseToolTip );

  CPoint pos = GetApplication()->GetMousePos();
  pos.x += 6;
  pos.y -= font->GetLineHeight() / 2;

  API->DrawRect( CRect( pos, pos + CPoint( width, font->GetLineHeight() ) ), CColor( 0, 0, 0, 0x80 ) );
  font->Write( API, mouseToolTip, pos );
}

void GW2TacO::OpenWindow( CString s )
{
  CRect pos;
  if ( !Config::HasWindowData( s.GetPointer() ) )
    pos = CRect( -150, -150, 150, 150 ) + GetClientRect().Center();
  else pos = Config::GetWindowPosition( s.GetPointer() );

  auto itm = FindChildByID( s.GetPointer() );
  if ( itm )
  {
    bool openState = false;

    if ( itm->IsHidden() )
      openState = true;

    itm->Hide( !openState );

    //delete itm;
    Config::SetWindowOpenState( s.GetPointer(), openState );
    return;
  }

  if ( s == "MapTimer" )
  {
    OverlayWindow* w = new OverlayWindow( this, pos );
    w->SetID( s );
    Config::SetWindowOpenState( s.GetPointer(), true );
    GW2MapTimer* mt = new GW2MapTimer( w, w->GetClientRect() );
    w->ReapplyStyles();
  }

  if ( s == "TS3Control" )
  {
    OverlayWindow* w = new OverlayWindow( this, pos );
    w->SetID( s );
    Config::SetWindowOpenState( s.GetPointer(), true );
    TS3Control* mt = new TS3Control( w, w->GetClientRect() );
    w->ReapplyStyles();
  }

  if ( s == "MarkerEditor" )
  {
    OverlayWindow* w = new OverlayWindow( this, pos );
    w->SetID( s );
    Config::SetWindowOpenState( s.GetPointer(), true );
    GW2MarkerEditor* mt = new GW2MarkerEditor( w, w->GetClientRect() );
    mt->SetID( "MarkerEditor" );
    w->ReapplyStyles();
  }

  if ( s == "Notepad" )
  {
    OverlayWindow* w = new OverlayWindow( this, pos );
    w->SetID( s );
    Config::SetWindowOpenState( s.GetPointer(), true );
    GW2Notepad* mt = new GW2Notepad( w, w->GetClientRect() );
    w->ReapplyStyles();
  }

  if ( s == "RaidProgress" )
  {
    OverlayWindow* w = new OverlayWindow( this, pos );
    w->SetID( s );
    Config::SetWindowOpenState( s.GetPointer(), true );
    RaidProgress* mt = new RaidProgress( w, w->GetClientRect() );
    mt->SetID( "RaidProgressView" );
    w->ReapplyStyles();
  }

  if ( s == "DungeonProgress" )
  {
    OverlayWindow* w = new OverlayWindow( this, pos );
    w->SetID( s );
    Config::SetWindowOpenState( s.GetPointer(), true );
    DungeonProgress* mt = new DungeonProgress( w, w->GetClientRect() );
    w->ReapplyStyles();
  }

  if ( s == "TPTracker" )
  {
    OverlayWindow* w = new OverlayWindow( this, pos );
    w->SetID( s );
    Config::SetWindowOpenState( s.GetPointer(), true );
    TPTracker* mt = new TPTracker( w, w->GetClientRect() );
    w->ReapplyStyles();
  }
}

void GW2TacO::BuildChannelTree( TS3Connection::TS3Schandler& h, CWBContextItem* parentitm, TS32 ParentID )
{
  for ( TS32 x = 0; x < h.Channels.NumItems(); x++ )
  {
    TS3Connection::TS3Channel& chn = h.Channels[ x ];
    if ( chn.parentid == ParentID )
    {
      auto newitm = parentitm->AddItem( chn.name.GetPointer(), 0 );
      if ( chn.id != chn.parentid )
        BuildChannelTree( h, newitm, chn.id );
    }
  }
}

void GW2TacO::RebindAction( TacOKeyAction Action )
{
  RebindMode = true;
  ScriptRebindMode = false;
  ActionToRebind = Action;
  SetFocus();
}

void GW2TacO::RebindScriptKey( TS32 eventIndex )
{
  RebindMode = true;
  ScriptRebindMode = true;
  ScriptActionToRebind = eventIndex;
  SetFocus();
}

void GW2TacO::ApiKeyInputAction( APIKeys keyType, TS32 idx )
{
  ApiKeyInputMode = true;
  ApiKeyToSet = keyType;
  APIKeyInput = new CWBTextBox( this, GetClientRect(), WB_TEXTBOX_SINGLELINE );
  APIKeyInput->SetID( "APIkeyInput" );
  APIKeyInput->ReapplyStyles();
  APIKeyInput->EnableHScrollbar( false, false );
  APIKeyInput->EnableVScrollbar( false, false );
  CWBMessage m;
  BuildPositionMessage( GetClientRect(), m );
  m.Resized = true;
  App->SendMessage( m );
  ApiKeyIndex = idx;

  switch ( keyType )
  {
  case APIKeys::None:
    break;
  case APIKeys::TS3APIKey:
    if ( Config::HasString( "TS3APIKey" ) )
      APIKeyInput->SetText( Config::GetString( "TS3APIKey" ) );
    break;
  case APIKeys::GW2APIKey:
  {
    auto key = GW2::apiKeyManager.keys[ idx ];
    APIKeyInput->SetText( key->apiKey );
  }
  break;
  default:
    break;
  }

  APIKeyInput->SetFocus();

}

void GW2TacO::TurnOnTPLight()
{
  showPickupHighlight = true;
}

void GW2TacO::TurnOffTPLight()
{
  showPickupHighlight = false;
}

void GW2TacO::CheckItemPickup()
{
  if ( GW2::apiKeyManager.GetStatus() != GW2::APIKeyManager::Status::OK )
    return;

  GW2::APIKey* key = GW2::apiKeyManager.GetIdentifiedAPIKey();

  if ( key && key->valid && ( globalTimer.GetTime() - lastPickupFetchTime > 150000 || !lastPickupFetchTime ) && !pickupsBeingFetched && !pickupFetcherThread.joinable() )
  {
    pickupsBeingFetched = true;
    pickupFetcherThread = std::thread( [this, key]()
                                       {
                                         SetPerThreadCRTExceptionBehavior();
                                         CString query = key->QueryAPI( "v2/commerce/delivery" );

                                         Object json;
                                         json.parse( query.GetPointer() );

                                         TS32 coins = 0;
                                         TS32 itemCount = 0;

                                         if ( json.has<Number>( "coins" ) )
                                         {
                                           coins = (TS32)( json.get<Number>( "coins" ) );

                                           if ( json.has<Array>( "items" ) )
                                           {
                                             itemCount = json.get<Array>( "items" ).size();

                                             if ( ( !coins && !itemCount ) || !lastItemPickup.Length() )
                                             {
                                               TurnOffTPLight();
                                               if ( !lastItemPickup.Length() )
                                                 lastItemPickup = query;
                                             }
                                             else
                                             {
                                               if ( query != lastItemPickup )
                                               {
                                                 TurnOnTPLight();
                                                 lastItemPickup = query;
                                               }
                                             }
                                           }
                                           else
                                             TurnOffTPLight();
                                         }
                                         else
                                           TurnOffTPLight();

                                         pickupsBeingFetched = false;
                                       } );
  }

  if ( !pickupsBeingFetched && pickupFetcherThread.joinable() )
  {
    lastPickupFetchTime = globalTimer.GetTime();
    pickupFetcherThread.join();
  }
}

void GW2TacO::StoreIconSizes()
{
  if ( iconSizesStored || !App )
    return;

  CWBItem* v1 = App->GetRoot()->FindChildByID( "MenuButton" );
  CWBItem* v2 = App->GetRoot()->FindChildByID( "MenuHoverBox" );
  CWBItem* v3 = App->GetRoot()->FindChildByID( "TPButton" );
  CWBItem* v4 = App->GetRoot()->FindChildByID( "RedCircle" );

  if ( v1 )
    tacoIconRect = v1->GetPosition();

  if ( v2 )
    menuHoverRect = v2->GetPosition();

  if ( v3 )
    tpButtonRect = v3->GetPosition();

  if ( v4 )
    tpHighlightRect = v4->GetPosition();

  iconSizesStored = true;
}

void GW2TacO::AdjustMenuForWindowTooSmallScale( float scale )
{
  if ( !iconSizesStored || !App )
    return;

  CWBItem* v1 = App->GetRoot()->FindChildByID( "MenuButton" );
  CWBItem* v2 = App->GetRoot()->FindChildByID( "MenuHoverBox" );
  CWBItem* v3 = App->GetRoot()->FindChildByID( "TPButton" );
  CWBItem* v4 = App->GetRoot()->FindChildByID( "RedCircle" );

  CString str;
  if ( v1 )
  {
    str = CString::Format( "#MenuButton{ left:%dpx; top:%dpx; width:%dpx; height:%dpx; background:skin(taco_stretch_dark) top left; } #MenuButton:hover{background:skin(taco_stretch_light) top left;} #MenuButton:active{background:skin(taco_stretch_light) top left;}", int( tacoIconRect.x1 * scale ), int( tacoIconRect.y1 * scale ), int( tacoIconRect.Width() * scale ), int( tacoIconRect.Height() * scale ) );
    App->LoadCSS( str, false );
  }

  if ( v2 )
  {
    str = CString::Format( "#MenuHoverBox{ left:%dpx; top:%dpx; width:%dpx; height:%dpx; }", int( menuHoverRect.x1 * scale ), int( menuHoverRect.y1 * scale ), int( menuHoverRect.Width() * scale ), int( menuHoverRect.Height() * scale ) );
    App->LoadCSS( str, false );
  }

  if ( v3 )
  {
    str = CString::Format( "#TPButton{ left:%dpx; top:%dpx; width:%dpx; height:%dpx; }", int( tpButtonRect.x1 * scale ), int( tpButtonRect.y1 * scale ), int( tpButtonRect.Width() * scale ), int( tpButtonRect.Height() * scale ) );
    App->LoadCSS( str, false );
  }

  if ( v4 )
  {
    str = CString::Format( "#RedCircle{ left:%dpx; top:%dpx; width:%dpx; height:%dpx; background:skin(redcircle_stretch) top left; }", int( tpHighlightRect.x1 * scale ), int( tpHighlightRect.y1 * scale ), int( tpHighlightRect.Width() * scale ), int( tpHighlightRect.Height() * scale ) );
    App->LoadCSS( str, false );
  }

  App->ReApplyStyle();
}

