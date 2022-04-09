#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include "gw2tactical.h"
#include "TS3Connection.h"
#include <thread>
#include "InputHooks.h"

enum class TacOKeyAction : TS32
{
  NoAction = 0,
  AddPOI,
  RemovePOI,
  ActivatePOI,
  EditNotepad,
  StartTrailRec,
  PauseTrailRec,
  DeleteLastTrailSegment,
  ResumeTrailAndCreateNewSection,
  Toggle_tactical_layer,
  Toggle_range_circles,
  Toggle_tactical_compass,
  Toggle_locational_timers,
  Toggle_hp_grids,
  Toggle_mouse_highlight,
  Toggle_map_timer,
  Toggle_ts3_window,
  Toggle_marker_editor,
  Toggle_notepad,
  Toggle_raid_progress,
  Toggle_dungeon_progress,
  Toggle_tp_tracker,
  Toggle_window_edit_mode,
  AddDefaultPOI_1,
  AddDefaultPOI_2,
  AddDefaultPOI_3,
  AddDefaultPOI_4,
  DeleteSelectedMarker,
  DeleteSelectedTrailSegment,
  CopySelectedMarker,
  UndoEdit,
  RedoEdit

  //if you add one here, add it to the ActionNames array in the .cpp as well!
};

enum class APIKeys
{
  None = 0,
  TS3APIKey,
  GW2APIKey,
};

extern CString ActionNames[];

class GW2TacO : public CWBItem
{
  CString lastInfoLine;
  TBOOL RebindMode = false;
  TBOOL ScriptRebindMode = false;
  TacOKeyAction ActionToRebind = TacOKeyAction::NoAction;
  TS32 ScriptActionToRebind = 0;

  TBOOL ApiKeyInputMode = false;
  APIKeys ApiKeyToSet = APIKeys::None;
  TS32 ApiKeyIndex = 0;

  void OpenAboutWindow();
  void BuildChannelTree( TS3Connection::TS3Schandler& h, CWBContextItem* parentitm, TS32 ParentID );

  CDictionary<TS32, TacOKeyAction> KeyBindings;
  CDictionary<TS32, CString> ScriptKeyBindings;

  void RebindAction( TacOKeyAction Action );
  void RebindScriptKey( TS32 evendIDX );
  CArray<GW2TacticalCategory*> CategoryList;
  //CArray<AngelWrapper*> scriptEngines;

  void ApiKeyInputAction( APIKeys keyType, TS32 idx );
  CWBTextBox* APIKeyInput = nullptr;

  TBOOL menuHoverLastFrame = false;
  TS32 lastMenuHoverTransitionTime = 0;

  void TurnOnTPLight();
  void TurnOffTPLight();

  void CheckItemPickup();

  CString lastItemPickup;
  TBOOL pickupsBeingFetched = false;
  std::thread pickupFetcherThread;
  TS32 lastPickupFetchTime = 0;
  TBOOL showPickupHighlight = false;
  float lastScaleValue = 1.0f;

  void StoreIconSizes();
  void AdjustMenuForWindowTooSmallScale( float scale );

  CString mouseToolTip;
  CString GetKeybindString( TacOKeyAction action );

  void OpenMainMenu( CWBContextMenu* ctx );
  void RebuildMainMenu( CWBContextMenu* ctx );

public:
  virtual void OnDraw( CWBDrawAPI* API );
  virtual void OnPostDraw( CWBDrawAPI* API );
  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );

  GW2TacO( CWBItem* Parent, CRect Position );
  virtual ~GW2TacO();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "GW2TacO" ), CWBItem );
  void OpenWindow( CString s );

  virtual TBOOL MessageProc( CWBMessage& Message ); //return true if this item handled the message

  void SetInfoLine( const CString& string );
  void SetMouseToolTip( const CString& toolTip );

  void InitScriptEngines();
  void TickScriptEngines();
  void TriggerScriptEngineAction( GUID& guid );
  void TriggerScriptEngineKeyEvent( const CString& eventID );
};

extern CString UIFileNames[];

void SetMouseToolTip( const CString& toolTip );
