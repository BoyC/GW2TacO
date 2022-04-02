#pragma once
#include "Bedrock/WhiteBoard/whiteboard.h"
#include <objbase.h>
#include <unordered_map>
#include <set>
#include "Achievements.h"

enum class POIBehavior : TS32
{
  AlwaysVisible,
  ReappearOnMapChange,
  ReappearOnDailyReset,
  OnlyVisibleBeforeActivation,
  ReappearAfterTimer,
  ReappearOnMapReset,
  OncePerInstance,
  DailyPerChar,
  OncePerInstancePerChar,
  WvWObjective,
};

struct MarkerTypeData
{
  struct
  {
    TBOOL iconFileSaved : 1;
    TBOOL sizeSaved : 1;
    TBOOL alphaSaved : 1;
    TBOOL fadeNearSaved : 1;
    TBOOL fadeFarSaved : 1;
    TBOOL heightSaved : 1;
    TBOOL behaviorSaved : 1;
    TBOOL resetLengthSaved : 1;
    TBOOL autoTriggerSaved : 1;
    TBOOL hasCountdownSaved : 1;
    TBOOL triggerRangeSaved : 1;
    TBOOL minSizeSaved : 1;
    TBOOL maxSizeSaved : 1;
    TBOOL colorSaved : 1;
    TBOOL trailDataSaved : 1;
    TBOOL animSpeedSaved : 1;
    TBOOL textureSaved : 1;
    TBOOL trailScaleSaved : 1;
    TBOOL toggleCategorySaved : 1;
    TBOOL achievementIdSaved : 1;
    TBOOL achievementBitSaved : 1;
    TBOOL autoTrigger : 1;
    TBOOL hasCountdown : 1;
    TBOOL miniMapVisible : 1;
    TBOOL bigMapVisible : 1;
    TBOOL inGameVisible : 1;
    TBOOL miniMapVisibleSaved : 1;
    TBOOL bigMapVisibleSaved : 1;
    TBOOL inGameVisibleSaved : 1;
    TBOOL scaleWithZoom : 1;
    TBOOL scaleWithZoomSaved : 1;
    TBOOL miniMapSizeSaved : 1;
    TBOOL miniMapFadeOutLevelSaved : 1;
    TBOOL keepOnMapEdge : 1;
    TBOOL keepOnMapEdgeSaved : 1;
    TBOOL infoSaved : 1;
    TBOOL infoRangeSaved : 1;
    TBOOL copySaved : 1;
    TBOOL copyMessageSaved : 1;
    TBOOL defaultToggleSaved : 1;
    TBOOL defaultToggle : 1;
    TBOOL defaultToggleLoaded : 1;
  } bits;

  unsigned char festivalSaveMask = 0;
  unsigned char festivalMask = 0;

  MarkerTypeData();

  TF32 size = 1.0;
  TF32 alpha = 1.0f;
  TF32 fadeNear = -1;
  TF32 fadeFar = -1;
  TF32 height = 1.5f;
  TF32 triggerRange = 2.0f;
  TF32 animSpeed = 1;
  TF32 trailScale = 1;
  TS32 miniMapSize = 20;
  TF32 miniMapFadeOutLevel = 100.0f;
  TF32 infoRange = 2.0f;

  POIBehavior behavior = POIBehavior::AlwaysVisible;
  CColor color = CColor( 0xffffffff );

  TS16 resetLength = 0;
  TS16 minSize = 5;
  TS16 maxSize = 2048;

  TS16 iconFile = -1;
  TS16 trailData = -1;
  TS16 texture = -1;
  TS16 toggleCategory = -1;
  TS16 achievementId = -1;
  TS16 achievementBit = -1;
  TS16 info = -1;
  TS16 copy = -1;
  TS16 copyMessage = -1;

  void Read( CXMLNode& n, TBOOL StoreSaveState );
  void Write( CXMLNode* n );
};

class GW2TacticalCategory;

struct POI
{
  MarkerTypeData typeData;
  CVector3 position;
  CVector3 rotation;
  TS32 mapID;
  GUID guid;
  TS16 Type = -1; //type string id

  WBATLASHANDLE icon = 0;

  CVector4 cameraSpacePosition;

  TU8 wvwObjectiveID;

  time_t lastUpdateTime = 0;
  TBOOL external = false;
  TBOOL routeMember = false;

  TS16 zipFile;
  TS16 iconFile;

  GW2TacticalCategory* category = nullptr;
  void SetCategory( CWBApplication* App, GW2TacticalCategory* t );

  bool IsVisible( const tm& ptm, const time_t& currtime );
};

struct POIActivationDataKey
{
  GUID guid;
  int uniqueData = 0;

  POIActivationDataKey() = default;
  POIActivationDataKey( GUID g, int inst )
    : guid( g )
    , uniqueData( inst )
  {
  }

  bool operator== ( const POIActivationDataKey& d )
  {
    return guid == d.guid && uniqueData == d.uniqueData;
  }
};

struct POIActivationData
{
  GUID poiguid;
  int uniqueData = 0;
  time_t lastUpdateTime = 0;
};

struct POIRoute
{
  CString name;
  TBOOL backwards = true;
  CArray<GUID> route;
  TBOOL external = false;
  TBOOL hasResetPos = false;
  CVector3 resetPos;
  float resetRad = 0;
  int MapID = 0;

  TS32 activeItem = -1;
};

TU32 DictionaryHash( const GUID& i );
TU32 DictionaryHash( const POIActivationDataKey& i );

extern std::unordered_map<int, CDictionaryEnumerable<GUID, POI>> POISet;
extern CDictionaryEnumerable<POIActivationDataKey, POIActivationData> activationData;
extern CArray<POIRoute> Routes;
extern CDictionaryEnumerable<CString, POI> wvwPOIs;
extern GW2TacticalCategory CategoryRoot;
extern CDictionaryEnumerable<CString, GW2TacticalCategory*> CategoryMap;

CDictionaryEnumerable<GUID, POI>& GetMapPOIs();

class GW2TacticalDisplay : public CWBItem
{
  bool tacticalIconsOnEdge;
  TF32 asp;
  CMatrix4x4 cam;
  CMatrix4x4 persp;
  CRect drawrect;

  void InsertPOI( POI& poi );
  void DrawPOI( CWBDrawAPI* API, const tm& ptm, const time_t& currtime, POI& poi, bool drawDistance, CString& infoText );
  void DrawPOIMinimap( CWBDrawAPI* API, const CRect& miniRect, CVector2& pos, const tm& ptm, const time_t& currtime, POI& poi, float alpha, float zoomLevel );
  virtual void OnDraw( CWBDrawAPI* API );
  CVector3 ProjectTacticalPos( CVector3 pos, TF32 fov, TF32 asp );
  CArray<POI*> mapPOIs;
  CArray<POI*> minimapPOIs;
  bool drawWvWNames;

  TS64 bigMessageStart = 0;
  int bigMessageLength = 3000;
  POI* triggeredPOI = nullptr;
  TS32 bigMessage = -1;

public:

  bool storeMarkerPositions = false;
  CDictionary<GUID, CRect> markerPositions;
  CDictionary<GUID, CRect> markerMinimapPositions;

  GW2TacticalDisplay( CWBItem* Parent, CRect Position );
  virtual ~GW2TacticalDisplay();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "gw2tactical" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );
  void RemoveUserMarkersFromMap();
  void TriggerBigMessage( TS32 messageString );
};

class GW2TacticalCategory
{
  CString cachedTypeName;

public:
  CString name;
  CString displayName;

  TS16 zipFile;

  MarkerTypeData data;
  TBOOL keepSaveState = false;
  TBOOL isOnlySeparator = false;
  GW2TacticalCategory* parent = nullptr;
  CArray<GW2TacticalCategory*> children;
  std::set<int> containedMapIds;

  CString GetFullTypeName();

  TBOOL isDisplayed = true;
  TBOOL cachedVisibility = true;
  TBOOL IsVisible() const;
  void CacheVisibility();

  static bool visibilityCached;
  void CalculateVisibilityCache();
  void SetDefaultToggleValues();

  bool hiddenFromContextMenu = false;
  int markerCount = 0;

  virtual ~GW2TacticalCategory()
  {
    children.FreeArray();
  }
};

void AddPOI( CWBApplication* App );
void DeletePOI();
void UpdatePOI( CWBApplication* App );

void OpenTypeContextMenu( CWBContextMenu* ctx, CArray<GW2TacticalCategory*>& CategoryList, TBOOL AddVisibilityMarkers = false, TS32 BaseID = 0, TBOOL markerEditor = false, CDictionary<TS32, Achievement>& achievements = CDictionary<TS32, Achievement>() );
void OpenTypeContextMenu( CWBContextItem* ctx, CArray<GW2TacticalCategory*>& CategoryList, TBOOL AddVisibilityMarkers = false, TS32 BaseID = 0, TBOOL markerEditor = false, CDictionary<TS32, Achievement>& achievements = CDictionary<TS32, Achievement>() );
void AddTypeContextMenu( CWBContextMenu* ctx, CArray<GW2TacticalCategory*>& CategoryList, GW2TacticalCategory* Parent, TBOOL AddVisibilityMarkers, TS32 BaseID, TBOOL closeOnClick );
GW2TacticalCategory* FindInCategoryTree( GW2TacticalCategory* cat );
void SetAllCategoriesToVisibleInContext( GW2TacticalCategory* Parent );

float WorldToGameCoords( float world );
float GameToWorldCoords( float game );
void FindClosestRouteMarkers( TBOOL force );

TS32 AddStringToMap( const CString& string );
CString& GetStringFromMap( TS32 idx );
GW2TacticalCategory* GetCategory( CString s );
