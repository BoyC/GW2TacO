#pragma once
#include "Bedrock/WhiteBoard/whiteboard.h"
#include "MumbleLink.h"
#include "gw2tactical.h"
//#include <unordered_map>

void GlobalDoTrailLogging( TS32 mapID, CVector3 charPos );

struct GW2TrailVertex
{
  CVector4 Pos;
  CVector2 UV;
  CVector4 CenterPos;
  CColor Color;
};

class GW2Trail
{
  friend class GW2TrailDisplay;

  CArray<CVector3> positions;

  void Reset( TS32 _mapID = 0 );

  TBOOL SaveToFile( const CString& fname );

public:

  virtual ~GW2Trail();

  TS32 length = 0;
  CCoreVertexBuffer* trailMesh = nullptr;
  CCoreDevice* dev = nullptr;
  CCoreIndexBuffer* idxBuf = nullptr;
  CCoreTexture* texture = nullptr;

  TS32 map = 0;

  void Build( CCoreDevice* dev, TS32 mapID, float* points, int pointCount );
  void Draw();
  void Update();
  void SetupAndDraw( CCoreConstantBuffer* constBuffer, CCoreTexture* texture, CMatrix4x4& cam, CMatrix4x4& persp, float& one, bool scaleData, TS32 fadeoutBubble, float* data, float fadeAlpha, float width, float uvScale, float width2d );

  MarkerTypeData typeData;
  CString Type;
  GUID guid;
  TBOOL External = false;
  CString zipFile;

  GW2TacticalCategory* category = nullptr;
  void SetCategory( CWBApplication* App, GW2TacticalCategory* t );

  TBOOL Import( CStreamReaderMemory& file, TBOOL keepPoints = false );
  TBOOL Import( CString& fileName, const CString& zipFile, TBOOL keepPoints = false );
};

class GW2TrailDisplay : public CWBItem
{
  TF32 asp;
  CMatrix4x4 cam;
  CMatrix4x4 persp;
  CRect drawrect;

  virtual void OnDraw( CWBDrawAPI* API );
  void DrawMinimap( CWBDrawAPI* API );

  CCoreVertexFormat* vertexFormat = nullptr;

  CCoreVertexShader* vxShader = nullptr;
  CCorePixelShader* pxShader = nullptr;
  CCoreConstantBuffer* constBuffer = nullptr;
  CCoreTexture2D* trailTexture = nullptr;
  CCoreSamplerState* trailSampler = nullptr;
  CCoreRasterizerState* trailRasterizer1 = nullptr;
  CCoreRasterizerState* trailRasterizer2 = nullptr;
  CCoreRasterizerState* trailRasterizer3 = nullptr;
  CCoreDepthStencilState* trailDepthStencil = nullptr;

  GW2Trail* editedTrail = nullptr;

  void ClearEditedTrail();
  TBOOL trailBeingRecorded = false;
  TBOOL trailRecordPaused = false;

  LIGHTWEIGHT_CRITICALSECTION critsec;

  CCoreTexture2D* GetTexture( const CString& fname, const CString& zipFile, const CString& categoryZip );

  CDictionary<CString, CCoreTexture2D*> textureCache;

public:

  GW2TrailDisplay( CWBItem* Parent, CRect Position );
  virtual ~GW2TrailDisplay();

  static CWBItem* Factory( CWBItem* Root, CXMLNode& node, CRect& Pos );
  WB_DECLARE_GUIITEM( _T( "gw2Trails" ), CWBItem );

  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );

  void DoTrailLogging( TS32 mapID, CVector3 charPos );

  void StartStopTrailRecording( TBOOL start );
  void PauseTrail( TBOOL pause, TBOOL newSection = false );
  void DeleteLastTrailSegment();
  void DeleteTrailSegment();
  void ExportTrail();
  void ImportTrail();

  void DrawProxy( CWBDrawAPI* API, bool miniMaprender );
};

extern std::unordered_map<int, CDictionaryEnumerable<GUID, GW2Trail*>> trailSet;
CDictionaryEnumerable<GUID, GW2Trail*>& GetMapTrails();