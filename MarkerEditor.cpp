#include "MarkerEditor.h"
#include "MumbleLink.h"
#include "MarkerPack.h"
#include "OverlayConfig.h"
#include "TrailLogger.h"
#include "GW2Tactical.h"

//////////////////////////////////////////////////////////////////////////
// Marker DOM

POI* FindMarkerByGUID( const GUID& guid )
{
  for ( auto& map : POISet )
  {
    if ( map.second.HasKey( guid ) )
      return &map.second[ guid ];
  }
  return nullptr;
}

GW2Trail* FindTrailByGUID( const GUID& guid )
{
  for ( auto& map : trailSet )
  {
    if ( map.second.HasKey( guid ) )
      return map.second[ guid ];
  }
  return nullptr;
}

int MarkerDOM::bufferIndex = 0;
CArray<MarkerActionData> MarkerDOM::actionBuffer;
CArray<MarkerActionData> MarkerDOM::undoBuffer;

MarkerActionData::MarkerActionData( MarkerAction action, const GUID& guid, const CVector3& pos /*= CVector3()*/, const int intData /*= 0 */ )
  : action( action )
  , guid( guid )
  , position( pos )
  , intData( intData )
{}


MarkerActionData::MarkerActionData( MarkerAction action, const int stringId, const MarkerTypeData& typeData )
  : action( action )
  , intData( stringId )
  , typeData( typeData )
{}

MarkerActionData::MarkerActionData( MarkerAction action, const GUID& guid, const MarkerTypeData& typeData, const CVector3& pos /*= CVector3()*/, const CVector3& rot /*= CVector3()*/, int intData /*= 0*/, int stringId /*= -1 */ )
  : action( action )
  , guid( guid )
  , typeData( typeData )
  , position( pos )
  , rotation( rot )
  , intData( intData )
  , stringID( stringId )
{}

void MarkerActionData::Do()
{}

void MarkerDOM::AddMarker( const GUID& guid, const CVector3& pos, const int mapID )
{
  actionBuffer += MarkerActionData( MarkerAction::AddMarker, guid, pos, mapID );
  undoBuffer += MarkerActionData( MarkerAction::DeleteMarker, guid );
  Redo();
}

void MarkerDOM::DeleteMarker( const GUID& guid )
{
  auto* marker = FindMarkerByGUID( guid );
  if ( !marker )
    return;

  actionBuffer += MarkerActionData( MarkerAction::DeleteMarker, guid );
  undoBuffer += MarkerActionData( MarkerAction::AddMarker, guid, marker->typeData, marker->position, marker->rotation, marker->mapID, marker->Type );
  Redo();
}

void MarkerDOM::SetMarketPosition( const GUID& guid, const CVector3& pos )
{
  auto* marker = FindMarkerByGUID( guid );
  if ( !marker )
    return;

  actionBuffer += MarkerActionData( MarkerAction::MoveMarker, guid, pos );
  undoBuffer += MarkerActionData( MarkerAction::MoveMarker, guid, marker->position );
  Redo();
}

void MarkerDOM::SetTrailVertexPosition( const GUID& guid, int vertex, const CVector3& pos )
{}

void MarkerDOM::Undo()
{
  if ( bufferIndex > 0 )
  {
    bufferIndex--;
    undoBuffer[ bufferIndex ].Do();
  }
}

void MarkerDOM::Redo()
{
  if ( bufferIndex < actionBuffer.NumItems() )
  {
    actionBuffer[ bufferIndex ].Do();
    bufferIndex++;
  }
}

TBOOL GW2MarkerEditor::HandleUberToolMessages( CWBMessage& message )
{
/*
  switch ( message.GetMessage() )
  {
  default:
    break;
  }
*/
  return false;
}

void GW2MarkerEditor::DrawUberTool( CWBDrawAPI* API, const CRect& drawrect )
{
  cam.SetLookAtLH( mumbleLink.camPosition, mumbleLink.camPosition + mumbleLink.camDir, CVector3( 0, 1, 0 ) );
  persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (TF32)drawrect.Height(), 0.01f, 150.0f );
  uberTool.SetTransformation( CVector3( 1, 1, 1 ), CQuaternion( CVector3( 0, 0, 0 ) ), mumbleLink.charPosition );

  constBuffer->Reset();
  constBuffer->AddData( uberTool, 16 * 4 );
  constBuffer->AddData( cam, 16 * 4 );
  constBuffer->AddData( persp, 16 * 4 );
  constBuffer->AddData( CVector4( 1, 1, 1, 1 ), 4 * 4 );
  constBuffer->Upload();

  API->FlushDrawBuffer();

  App->GetDevice()->SetShaderConstants( 0, 1, &constBuffer );
  App->GetDevice()->SetVertexShader( vxShader );
  App->GetDevice()->SetPixelShader( pxShader );
  App->GetDevice()->SetVertexFormat( vertexFormat );
  depthStencil->Apply();
  rasterizer->Apply();

  App->GetDevice()->SetVertexBuffer( circle.mesh, 0 );
  App->GetDevice()->DrawTriangles( circle.triCount );

  API->SetUIRenderState();
}

//////////////////////////////////////////////////////////////////////////
// Marker Editor

TBOOL GW2MarkerEditor::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

GW2MarkerEditor::GW2MarkerEditor( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{
  App->GenerateGUITemplate( this, "gw2pois", "markereditor" );
  InitUberTool();
}

GW2MarkerEditor::~GW2MarkerEditor()
{
  SAFEDELETE( vertexFormat );
  SAFEDELETE( vxShader );
  SAFEDELETE( pxShader );
  SAFEDELETE( constBuffer );
  SAFEDELETE( rasterizer );
  SAFEDELETE( depthStencil );
}

CWBItem* GW2MarkerEditor::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new GW2MarkerEditor( Root, Pos );
}

void GW2MarkerEditor::OnDraw( CWBDrawAPI* API )
{
  TBOOL autoHide = Config::GetValue( "AutoHideMarkerEditor" );

  if ( !Config::GetValue( "TacticalLayerVisible" ) )
    return;

  if ( !mumbleLink.IsValid() ) return;

  if ( mumbleLink.mapID == -1 ) return;

  auto& POIs = GetMapPOIs();

  for ( TS32 x = 0; x < POIs.NumItems(); x++ )
  {
    auto& cpoi = POIs.GetByIndex( x );

    if ( cpoi.mapID != mumbleLink.mapID ) continue;
    if ( cpoi.external ) continue;

    CVector3 v = cpoi.position - CVector3( mumbleLink.charPosition );
    if ( v.Length() < cpoi.typeData.triggerRange )
    {
      if ( autoHide )
      {
        if ( hidden )
          for ( TU32 z = 0; z < NumChildren(); z++ )
            GetChild( z )->Hide( false );
        hidden = false;
      }

      if ( currentPOI != cpoi.guid )
      {
        CWBLabel* type = (CWBLabel*)FindChildByID( "markertype", "label" );
        if ( type )
        {
          CString typeName;
          if ( cpoi.category )
            typeName = cpoi.category->GetFullTypeName();

          type->SetText( "Type: " + typeName );
          if ( !typeName.Length() )
            type->SetText( "Type: undefined" );
        }
      }

      currentPOI = cpoi.guid;
      return;
    }
  }

  if ( autoHide )
  {
    if ( !hidden )
      for ( TU32 x = 0; x < NumChildren(); x++ )
        GetChild( x )->Hide( true );

    hidden = true;
  }
  else
  {
    if ( hidden )
      for ( TU32 x = 0; x < NumChildren(); x++ )
        GetChild( x )->Hide( false );

    hidden = false;
  }
}

struct UberToolVertex
{
  CVector4 Pos;
  CVector4 Color;
};

void GW2MarkerEditor::InitUberTool()
{
  constBuffer = App->GetDevice()->CreateConstantBuffer();

  rasterizer = App->GetDevice()->CreateRasterizerState();
  rasterizer->SetCullMode( CORECULL_NONE );
  rasterizer->Update();

  depthStencil = App->GetDevice()->CreateDepthStencilState();
  depthStencil->SetDepthEnable( false );
  depthStencil->Update();

  LPCSTR code = R"_(
cbuffer resdata : register(b0)
{
  float4x4 obj;
  float4x4 camera;
  float4x4 persp;
  float4 color;
}
struct VSIN 
{ 
  float4 Position : POSITIONT; 
  float4 Color : TEXCOORD0; 
};
struct VSOUT 
{ 
  float4 Position : SV_POSITION; 
  float4 Color : COLOR0; 
};
VSOUT vsmain(VSIN x) 
{ 
  VSOUT k; 
  k.Position=mul(obj,x.Position); 
  k.Position=mul(camera,k.Position); 
  k.Position=mul(persp,k.Position);
  k.Color=x.Color;
  return k; 
}
float4 psmain(VSOUT x) : SV_TARGET0 
{  
  return x.Color * color;
}
)_";

  vxShader = App->GetDevice()->CreateVertexShader( code, (TS32)strlen( code ), "vsmain", "vs_4_0" );
  pxShader = App->GetDevice()->CreatePixelShader( code, (TS32)strlen( code ), "psmain", "ps_4_0" );

  COREVERTEXATTRIBUTE vxDesc[] =
  {
    COREVXATTR_POSITIONT4,
    COREVXATTR_TEXCOORD4,
    COREVXATTR_STOP,
  };

  COREVERTEXATTRIBUTE* vx = vxDesc;
  CArray<COREVERTEXATTRIBUTE> Att;
  while ( *vx != COREVXATTR_STOP ) Att += *vx++;

  vertexFormat = App->GetDevice()->CreateVertexFormat( Att, vxShader );
  if ( !vertexFormat )
    LOG( LOG_ERROR, _T( "[GW2TacO]  Error creating Trail Vertex Format" ) );

  CArray<UberToolVertex> planeVertices;
  planeVertices += { CVector4( 0, 0, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 0, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 1, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 0, 0, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 1, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 1, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
  plane.mesh = App->GetDevice()->CreateVertexBuffer( (TU8*)planeVertices.GetPointer( 0 ), planeVertices.NumItems() * sizeof( UberToolVertex ) );
  plane.triCount = planeVertices.NumItems() / 3;

  CArray<UberToolVertex> arrowVertices;

  float segCount = 12;
  float shaftRad = 0.1f;
  float headRad = 0.3f;
  float arrowHead = 0.8f;

  for ( int x = 0; x < segCount; x++ )
  {
    float c1 = cos( x / segCount * PI * 2 );
    float s1 = sin( x / segCount * PI * 2 );
    float c2 = cos( ( x + 1 ) / segCount * PI * 2 );
    float s2 = sin( ( x + 1 ) / segCount * PI * 2 );

    // shaft
    arrowVertices += { CVector4( c1* shaftRad, s1* shaftRad, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c2* shaftRad, s2* shaftRad, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c2* shaftRad, s2* shaftRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c1* shaftRad, s1* shaftRad, 0, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c2* shaftRad, s2* shaftRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c1* shaftRad, s1* shaftRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };

    // expansion
    arrowVertices += { CVector4( c1* shaftRad, s1* shaftRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c2* shaftRad, s2* shaftRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c2* headRad, s2* headRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };

    arrowVertices += { CVector4( c1* shaftRad, s1* shaftRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c2* headRad, s2* headRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c1* headRad, s1* headRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };

    // head
    arrowVertices += { CVector4( c1* headRad, s1* headRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( c2* headRad, s2* headRad, arrowHead, 1 ), CVector4( 1, 1, 1, 1 ) };
    arrowVertices += { CVector4( 0, 0, 1, 1 ), CVector4( 1, 1, 1, 1 ) };
  }
  arrow.mesh = App->GetDevice()->CreateVertexBuffer( (TU8*)arrowVertices.GetPointer( 0 ), arrowVertices.NumItems() * sizeof( UberToolVertex ) );
  arrow.triCount = arrowVertices.NumItems() / 3;

  CArray<UberToolVertex> circleVertices;

  segCount = 48;
  float thickness = 0.05f;

  for ( int x = 0; x < segCount; x++ )
  {
    float c1 = cos( x / segCount * PI * 2 ) * 0.5f;
    float s1 = sin( x / segCount * PI * 2 ) * 0.5f;
    float c2 = cos( ( x + 1 ) / segCount * PI * 2 ) * 0.5f;
    float s2 = sin( ( x + 1 ) / segCount * PI * 2 ) * 0.5f;

    circleVertices += { CVector4( c1, s1, -thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c2, s2, -thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c2, s2,  thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c1, s1, -thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c2, s2,  thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c1, s1,  thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
  }

  circle.mesh = App->GetDevice()->CreateVertexBuffer( (TU8*)circleVertices.GetPointer( 0 ), circleVertices.NumItems() * sizeof( UberToolVertex ) );
  circle.triCount = circleVertices.NumItems() / 3;
}

TBOOL GW2MarkerEditor::MessageProc( CWBMessage& message )
{
  switch ( message.GetMessage() )
  {
  case WBM_COMMAND:
  {
    if ( hidden )
      break;

    CWBButton* b = (CWBButton*)App->FindItemByGuid( message.GetTarget(), _T( "button" ) );
    if ( !b )
      break;
    if ( b->GetID() == _T( "changemarkertype" ) )
    {
      auto ctx = b->OpenContextMenu( App->GetMousePos() );
      OpenTypeContextMenu( ctx, categoryList, false, 0, true );
      changeDefault = false;
    }

    if ( b->GetID() == _T( "changedefaultmarkertype" ) )
    {
      auto ctx = b->OpenContextMenu( App->GetMousePos() );
      OpenTypeContextMenu( ctx, categoryList, false, 0, true );
      changeDefault = true;
    }

    GW2TrailDisplay* trails = (GW2TrailDisplay*)App->GetRoot()->FindChildByID( _T( "trail" ), _T( "gw2Trails" ) );

    if ( b->GetID() == _T( "starttrail" ) )
    {
      b->Push( !b->IsPushed() );
      b->SetText( b->IsPushed() ? "Stop Recording" : "Start New Trail" );
      if ( trails )
        trails->StartStopTrailRecording( b->IsPushed() );
    }

    if ( b->GetID() == _T( "pausetrail" ) && trails )
      trails->PauseTrail( !b->IsPushed() );

    if ( b->GetID() == _T( "startnewsection" ) && trails )
      trails->PauseTrail( false, true );

    if ( b->GetID() == _T( "deletelastsegment" ) && trails )
      trails->DeleteLastTrailSegment();

    if ( b->GetID() == _T( "savetrail" ) && trails )
      trails->ExportTrail();

    if ( b->GetID() == _T( "loadtrail" ) && trails )
      trails->ImportTrail();

  }
  break;

  case WBM_CONTEXTMESSAGE:
    if ( message.Data >= 0 && message.Data < categoryList.NumItems() )
    {
      if ( !changeDefault )
      {
        auto& POIs = GetMapPOIs();

        POIs[ currentPOI ].SetCategory( App, categoryList[ message.Data ] );
        ExportPOIS();
        CWBLabel* type = (CWBLabel*)FindChildByID( "markertype", "label" );
        if ( type )
          type->SetText( "Marker Type: " + categoryList[ message.Data ]->GetFullTypeName() );
      }
      else
      {
        extern CString DefaultMarkerCategory;
        DefaultMarkerCategory = categoryList[ message.Data ]->GetFullTypeName();
        CWBLabel* type = (CWBLabel*)FindChildByID( "defaultmarkertype", "label" );
        if ( type )
          type->SetText( "Default Marker Type: " + categoryList[ message.Data ]->GetFullTypeName() );
      }
    }

    break;

  default:
    break;
  }

  return CWBItem::MessageProc( message );
}

UberToolPart::~UberToolPart()
{
  SAFEDELETE( mesh );
}
