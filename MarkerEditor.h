#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"
#include "gw2tactical.h"
#include "TrailLogger.h"

enum class MarkerAction
{
  AddMarkerFull, // done
  MoveMarker, // done

  AddTrailFull, // done
  MoveTrailVertex, // done
  AddTrailVertex, // done
  DeleteTrailVertex, // done

  DeleteMarkerTrail, // done
  SetMarkerTrailCategory,
  ChangeMarkerTrailAttrib,

  ChangeCategoryAttrib,

  // todo
  AddCategory,
  DeleteCategory,
};

enum class TypeParameters
{
  Size,
  MiniMapSize,
  MiniMapFadeoutLevel,
  MinSize,
  MaxSize,
  IconFile,
  ScaleWithZoom,
  Color,
  Alpha,
  FadeNear,
  FadeFar,
  Height,
  Behavior,
  AchievementID,
  AchievementBit,
  ResetLength,
  DefaultToggle,
  HasCountDown,
  ToggleCategory,
  AutoTrigger,
  TriggerRange,
  InfoRange,
  Info,
  Copy,
  CopyMessage,
  MiniMapVisible,
  BigMapVisible,
  InGameVisible,
  KeepOnMapEdge,
  AnimSpeed,
  TrailScale,
  TrailFile,
  Texture,

  MAX
};

class MarkerActionData
{
  MarkerAction action{};
  GUID targetMarker{};

  MarkerTypeData typeData{};
  GUID guid{};

  int intData{}; // vertex index / map id
  CVector3 position{}; // marker/vertex pos
  CVector3 rotation{}; // rotation
  TS32 stringID = -1;

  POI fullMarker;
  GW2Trail* fullTrail = nullptr;

public:

  MarkerActionData() = default;
  MarkerActionData( MarkerAction action, const GUID& guid, const int intData = 0, const CVector3& pos = CVector3{}, const int stringData = 0 );
  MarkerActionData( MarkerAction action, const GUID& guid, const int intData, const int stringData );
  MarkerActionData( MarkerAction action, const POI& fullMarker );
  MarkerActionData( MarkerAction action, GW2Trail* fullTrail );
  MarkerActionData( MarkerAction action, const GUID& guid, const MarkerTypeData& data, int stringData = -1 );

  void Do();
};

class MarkerDOM
{
  static int bufferIndex;
  static CArray<MarkerActionData> actionBuffer;
  static CArray<MarkerActionData> undoBuffer;

public:

  static void CropBuffers();

  static void AddMarkerFull( const POI& poi );
  static void DeleteMarkerTrail( const GUID& guid );
  static void MoveMarker( const GUID& guid, const CVector3& pos );
  static void AddTrailFull( GW2Trail* trail );
  static void MoveTrailVertex( const GUID& guid, int vertexID, const CVector3& pos );
  static void AddTrailVertex( const GUID& guid, int vertexID, const CVector3& pos );
  static void DeleteTrailVertex( const GUID& guid, int vertexID );
  static void SetMarkerTrailCategory( const GUID& guid, const CString& category );
  static void ChangeMarkerTrailAttrib( const GUID& guid, const MarkerTypeData& typeData );
  static void ChangeCategoryAttrib( const CString& category, const MarkerTypeData& typeData );

  static void Undo();
  static void Redo();

  static void ResetUndoBuffer();
};

struct UberToolPart
{
  CCoreVertexBuffer* mesh = nullptr;
  int triCount = 0;

  ~UberToolPart();
};

enum class UberToolMode
{
  Move,
  Scale,
  Rotate
};

enum UberToolElement
{
  none,
  moveX,
  moveY,
  moveZ,
  moveYZ,
  moveXZ,
  moveXY,
  rotateX,
  rotateY,
  rotateZ
};

class GW2MarkerEditor : public CWBItem
{
  UberToolMode uberToolMode = UberToolMode::Move;

  CMatrix4x4 cam;
  CMatrix4x4 persp;
  CMatrix4x4 uberTool;

  virtual TBOOL MessageProc( CWBMessage& message );
  virtual void OnDraw( CWBDrawAPI* API );
  TBOOL hidden = false;

  CArray<GW2TacticalCategory*> categoryList;
  TBOOL changeDefault = false;

  GUID editedMarker{};
  int selectedVertexIndex{};
  CString editedCategory{};

  CCoreVertexFormat* vertexFormat = nullptr;
  CCoreVertexShader* vxShader = nullptr;
  CCorePixelShader* pxShader = nullptr;
  CCoreConstantBuffer* constBuffer = nullptr;
  CCoreRasterizerState* rasterizer1 = nullptr;
  CCoreRasterizerState* rasterizer2 = nullptr;
  CCoreDepthStencilState* depthStencil = nullptr;

  UberToolPart plane;
  UberToolPart arrow;
  UberToolPart circle;

  UberToolElement hoverElement = UberToolElement::none;
  UberToolElement draggedElement = UberToolElement::none;

  CVector3 clickedPos{};
  CVector3 hoverPos{};

  CVector3 originalPosition{};
  CVector3 originalRotation{};
  CVector3 originalScale{};
  CVector3 clickedOriginalDelta{};

  CVector3 GetUberToolMovePos( const CVector3& location );
  int defaultCatBeingSet = -1;
  bool selectingEditedCategory = false;

  void InitUberTool();

  void UpdateEditorFromCategory( const MarkerTypeData& data );

  void UpdateTypeParameterValue( TypeParameters param );
  void ToggleTypeParameterSaved( TypeParameters param );
  bool GetTypeParameterSaved( TypeParameters param );
  void GetTypeParameterValue( TypeParameters param, bool& boolValue, int& intValue, float& floatValue, CString& stringValue );

  MarkerTypeData* GetEditedTypeParameters();
  void UpdateEditedTypeParameters( const MarkerTypeData& data );

  void HideEditorUI( bool fade );
  void ShowMarkerUI( bool marker, bool trail );
  GUID GetMouseTrail( int& idx, CVector3& hitPoint, bool& indexPlusOne );

public:

  TBOOL HandleUberToolMessages( CWBMessage& message );
  void DrawUberTool( CWBDrawAPI* API, const CRect& drawrect );

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );

  GW2MarkerEditor( CWBItem* Parent, CRect Position );
  virtual ~GW2MarkerEditor();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "markereditor" ), CWBItem );

  bool GetMouseTransparency( CPoint& ClientSpacePoint, WBMESSAGE MessageType );
  bool ShouldPassMouseEvent();

  void SetEditedGUID( const GUID& guid );
  void SetSelectedVertexIndex( int idx );
  void SetEditedCategory( const CString& category );
  void DeleteSelectedMarker();
  void DeleteSelectedTrailSegment();
  void CopySelectedMarker();
  GUID GetEditedGUID();
  int GetSelectedVertexIndex();
  CVector3 GetSelectedVertexPosition();
  void UpdateEditorContent();
};

bool IsTypeParameterSaved( const MarkerTypeData& data, TypeParameters param );
void GetTypeParameter( const MarkerTypeData& data, TypeParameters param, float& floatResult, int& intResult, CString& stringResult, bool& boolResult );
void SetTypeParameter( MarkerTypeData& data, TypeParameters param, const float floatResult, const int intResult, const CString& stringResult, const bool boolResult );
void SetTypeParameterSaved( MarkerTypeData& data, TypeParameters param, bool saved );
void PropagateCategoryParameter( GW2TacticalCategory* category, TypeParameters param, const bool boolValue, const int intValue, const float floatValue, const CString& stringValue );


