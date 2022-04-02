#include "MarkerEditor.h"
#include "MumbleLink.h"
#include "MarkerPack.h"
#include "OverlayConfig.h"
#include "TrailLogger.h"
#include "GW2Tactical.h"

extern CWBApplication* App;

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
  switch ( message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
  {
    if ( draggedElement != UberToolElement::none )
      break;

    if ( editedMarker != GUID{} && hoverElement != UberToolElement::none )
    {
      auto* marker = FindMarkerByGUID( editedMarker );
      if ( marker )
      {
        clickedPos = hoverPos;
        draggedElement = hoverElement;

        clickedOriginalDelta = GetUberToolMovePos( marker->position ) - marker->position;
        originalPosition = marker->position;
        originalRotation = marker->rotation;
        originalScale = CVector3( 1, 1, 1 ) * marker->typeData.size;
      }

      return true;
    }

    auto tactical = App->GetRoot()->FindChildByID<GW2TacticalDisplay>( "tactical" );

    if ( tactical )
    {
      auto mouse = App->GetMousePos();

      bool found = false;

      for ( int x = 0; x < tactical->markerPositions.NumItems(); x++ )
        if ( tactical->markerPositions.GetByIndex( x ).Contains( mouse ) )
        {
          editedMarker = tactical->markerPositions.GetKDPair( x )->Key;
          found = true;
        }

      for ( int x = 0; x < tactical->markerMinimapPositions.NumItems(); x++ )
        if ( tactical->markerMinimapPositions.GetByIndex( x ).Contains( mouse ) )
        {
          editedMarker = tactical->markerMinimapPositions.GetKDPair( x )->Key;
          found = true;
        }

      if ( found )
        return true;
    }

    return false;
  }
  case WBM_LEFTBUTTONUP:
    if ( draggedElement != UberToolElement::none )
    {
      draggedElement = UberToolElement::none;
      return true;
    }
    break;

  case WBM_MOUSEMOVE:
    if ( draggedElement != UberToolElement::none && editedMarker != GUID{} )
    {
      auto* marker = FindMarkerByGUID( editedMarker );
      if ( !marker )
        return false;

      CVector3 pos = GetUberToolMovePos( originalPosition );

      marker->position = pos - clickedOriginalDelta;

      return true;
    }
    break;
  default:
    break;
  }
  return false;
}

struct ConstBuffer
{
  CMatrix4x4 uberTool;
  CMatrix4x4 cam;
  CMatrix4x4 persp;
  CVector4 color;
};

float moverPlaneSize = 0.3f;

CVector3 GetHitPositionMoverPlane( ConstBuffer& bufferData, CMatrix4x4& matrix, CVector4& mouse1, CVector4& mouse2 )
{
  CMatrix4x4 invMatrix = ( matrix * bufferData.cam * bufferData.persp ).Inverted();
  CVector4 localMouse1 = mouse1 * invMatrix;
  CVector4 localMouse2 = mouse2 * invMatrix;
  localMouse1 /= localMouse1.w;
  localMouse2 /= localMouse2.w;
  CPlane hitPlane( CVector3( 0, 0, 0 ), CVector3( 0, 0, 1 ) );
  return hitPlane.Intersect( CLine( localMouse1, ( localMouse2 - localMouse1 ).Normalized() ) );
}

bool HitTestMoverPlane( ConstBuffer& bufferData, CMatrix4x4& matrix, CVector4& mouse1, CVector4& mouse2, CVector3& hitPos )
{
  CMatrix4x4 invMatrix = ( matrix * bufferData.cam * bufferData.persp ).Inverted();
  CVector4 localMouse1 = mouse1 * invMatrix;
  CVector4 localMouse2 = mouse2 * invMatrix;
  localMouse1 /= localMouse1.w;
  localMouse2 /= localMouse2.w;
  CPlane hitPlane( CVector3( 0, 0, 0 ), CVector3( 0, 0, 1 ) );
  CVector3 intersect = hitPlane.Intersect( CLine( localMouse1, ( localMouse2 - localMouse1 ).Normalized() ) );
  bool hit = intersect.x > 0 && intersect.x < moverPlaneSize&& intersect.y>0 && intersect.y < moverPlaneSize;
  if ( hit )
    hitPos = intersect;
  return hit;
}

CVector3 GetHitPositionMoverArrow( ConstBuffer& bufferData, CMatrix4x4& matrix, CVector4& mouse1, CVector4& mouse2 )
{
  CMatrix4x4 invMatrix = ( matrix * bufferData.cam * bufferData.persp ).Inverted();
  CVector4 localMouse1 = mouse1 * invMatrix;
  CVector4 localMouse2 = mouse2 * invMatrix;
  localMouse1 /= localMouse1.w;
  localMouse2 /= localMouse2.w;

  CVector3 planeDir = CVector3( localMouse2.x - localMouse1.x, localMouse2.y - localMouse1.y, 0 ).Normalized();

  CPlane hitPlane( CVector3( 0, 0, 0 ), planeDir );
  CVector3 intersect = hitPlane.Intersect( CLine( localMouse1, ( localMouse2 - localMouse1 ).Normalized() ) );
  return CVector3( 0, 0, intersect.z );
}

bool HitTestMoverArrow( ConstBuffer& bufferData, CMatrix4x4& matrix, CVector4& mouse1, CVector4& mouse2, CVector3& hitPos )
{
  CMatrix4x4 invMatrix = ( matrix * bufferData.cam * bufferData.persp ).Inverted();
  CVector4 localMouse1 = mouse1 * invMatrix;
  CVector4 localMouse2 = mouse2 * invMatrix;
  localMouse1 /= localMouse1.w;
  localMouse2 /= localMouse2.w;

  CVector3 planeDir = CVector3( localMouse2.x - localMouse1.x, localMouse2.y - localMouse1.y, 0 ).Normalized();  

  CPlane hitPlane( CVector3( 0, 0, 0 ), planeDir );
  CVector3 intersect = hitPlane.Intersect( CLine( localMouse1, ( localMouse2 - localMouse1 ).Normalized() ) );
  bool hit = intersect.z > 0 && intersect.z < 1 && ( intersect.y * intersect.y + intersect.x * intersect.x < 0.15 * 0.15 );
  if ( hit )
    hitPos = CVector3( 0, 0, intersect.z );
  return hit;
}

void UpdateObjectData( CCoreConstantBuffer* constBuffer, ConstBuffer& bufferData, CMatrix4x4& objMatrix, CVector4& color )
{
  bufferData.uberTool = objMatrix;
  bufferData.color = color;
  constBuffer->Reset();
  constBuffer->AddData( &bufferData, sizeof( ConstBuffer ) );
  constBuffer->Upload();
}

void GW2MarkerEditor::DrawUberTool( CWBDrawAPI* API, const CRect& drawrect )
{
  hoverElement = UberToolElement::none;

  if ( editedMarker == GUID{} )
    return;

  auto* marker = FindMarkerByGUID( editedMarker );
  if ( !marker )
    return;

  CVector3 location = marker->position;
  location.y += marker->typeData.height;

  float scale = ( mumbleLink.camPosition - location ).Length() * 0.1f;
  CVector3 eye = mumbleLink.camPosition;
  CVector3 mirror( eye.x > location.x ? 1.0f : -1.0f, eye.y > location.y ? 1.0f : -1.0f, eye.z > location.z ? 1.0f : -1.0f );

  int cnt = 0;
  if ( mirror.x < 0 )
    cnt++;
  if ( mirror.y < 0 )
    cnt++;
  if ( mirror.z < 0 )
    cnt++;

  CCoreRasterizerState* rasterizer = cnt % 2 ? rasterizer1 : rasterizer2;

  UberToolElement hitPart = UberToolElement::none;

  API->FlushDrawBuffer();

  App->GetDevice()->SetShaderConstants( 0, 1, &constBuffer );
  App->GetDevice()->SetVertexShader( vxShader );
  App->GetDevice()->SetPixelShader( pxShader );
  App->GetDevice()->SetVertexFormat( vertexFormat );
  App->GetDevice()->SetRenderState( depthStencil );
  App->GetDevice()->SetRenderState( rasterizer );

  ConstBuffer bufferData;
  bufferData.cam.SetLookAtLH( eye, mumbleLink.camPosition + mumbleLink.camDir, CVector3( 0, 1, 0 ) );
  bufferData.persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (TF32)drawrect.Height(), 0.01f, 150.0f );

  CMatrix4x4 matrix;

  POINT mousePos;
  mousePos.x = App->GetMousePos().x;
  mousePos.y = App->GetMousePos().y;

  if ( IsDebuggerPresent() )
    GetCursorPos( &mousePos );

  CVector4 mouse1 = CVector4( mousePos.x / (float)drawrect.Width() * 2 - 1, ( ( 1 - mousePos.y / (float)drawrect.Height() ) * 2 - 1 ), -1, 1 );
  CVector4 mouse2 = CVector4( mouse1.x, mouse1.y, 1, 1 );

  bool hit = false;
  bool dragged = false;

  // planes

  App->GetDevice()->SetVertexBuffer( plane.mesh, 0 );
  // yz
  matrix = CMatrix4x4::Rotation( CQuaternion( 0, PI / 2.0f, 0 ) ) * CMatrix4x4::Scaling( CVector3( mirror.x, mirror.y, -mirror.z ) * scale ) * CMatrix4x4::Translation( location );
  dragged = draggedElement == UberToolElement::moveYZ;
  hit = draggedElement == UberToolElement::none && !App->GetRightButtonState() && hitPart == UberToolElement::none && HitTestMoverPlane( bufferData, matrix, mouse1, mouse2, hoverPos );
  if ( hit )
    hitPart = UberToolElement::moveYZ;

  UpdateObjectData( constBuffer, bufferData, matrix, hit || dragged ? CVector4( 1, 1, 0, dragged ? 1 : 0.5f ) : CVector4( 1, 0, 0, 0.5f ) );
  App->GetDevice()->DrawTriangles( plane.triCount );

  // xz
  matrix = CMatrix4x4::Rotation( CQuaternion( -PI / 2.0f, 0, 0 ) ) * CMatrix4x4::Scaling( CVector3( mirror.x, mirror.y, -mirror.z ) * scale ) * CMatrix4x4::Translation( location );
  dragged = draggedElement == UberToolElement::moveXZ;
  hit = draggedElement == UberToolElement::none && !App->GetRightButtonState() && hitPart == UberToolElement::none && HitTestMoverPlane( bufferData, matrix, mouse1, mouse2, hoverPos );
  if ( hit )
    hitPart = UberToolElement::moveXZ;

  UpdateObjectData( constBuffer, bufferData, matrix, hit || dragged ? CVector4( 1, 1, 0, dragged ? 1 : 0.5f ) : CVector4( 0, 1, 0, 0.5f ) );
  App->GetDevice()->DrawTriangles( plane.triCount );

  // xy
  matrix = CMatrix4x4::Rotation( CQuaternion( CVector3( 0, 0, 0 ) ) ) * CMatrix4x4::Scaling( CVector3( mirror.x, mirror.y, -mirror.z ) * scale ) * CMatrix4x4::Translation( location );
  dragged = draggedElement == UberToolElement::moveXY;
  hit = draggedElement == UberToolElement::none && !App->GetRightButtonState() && hitPart == UberToolElement::none && HitTestMoverPlane( bufferData, matrix, mouse1, mouse2, hoverPos );
  if ( hit )
    hitPart = UberToolElement::moveXY;

  UpdateObjectData( constBuffer, bufferData, matrix, hit || dragged ? CVector4( 1, 1, 0, dragged ? 1 : 0.5f ) : CVector4( 0, 0, 1, 0.5f ) );
  App->GetDevice()->DrawTriangles( plane.triCount );

  // arrows
  App->GetDevice()->SetVertexBuffer( arrow.mesh, 0 );

  // x
  matrix = CMatrix4x4::Rotation( CQuaternion( 0, PI / 2.0f, 0 ) ) * CMatrix4x4::Scaling( mirror * scale ) * CMatrix4x4::Translation( location );
  dragged = draggedElement == UberToolElement::moveX;
  hit = draggedElement == UberToolElement::none && !App->GetRightButtonState() && hitPart == UberToolElement::none && HitTestMoverArrow( bufferData, matrix, mouse1, mouse2, hoverPos );
  if ( hit )
    hitPart = UberToolElement::moveX;

  UpdateObjectData( constBuffer, bufferData, matrix, hit || dragged ? CVector4( 1, 1, 0, dragged ? 1 : 0.5f ) : CVector4( 1, 0, 0, 0.5 ) );
  App->GetDevice()->DrawTriangles( arrow.triCount );

  // y
  matrix = CMatrix4x4::Rotation( CQuaternion( -PI / 2.0f, 0, 0 ) ) * CMatrix4x4::Scaling( mirror * scale ) * CMatrix4x4::Translation( location );
  dragged = draggedElement == UberToolElement::moveY;
  hit = draggedElement == UberToolElement::none && !App->GetRightButtonState() && hitPart == UberToolElement::none && HitTestMoverArrow( bufferData, matrix, mouse1, mouse2, hoverPos );
  if ( hit )
    hitPart = UberToolElement::moveY;

  UpdateObjectData( constBuffer, bufferData, matrix, hit || dragged ? CVector4( 1, 1, 0, dragged ? 1 : 0.5f ) : CVector4( 0, 1, 0, 0.5f ) );
  App->GetDevice()->DrawTriangles( arrow.triCount );

  // z
  matrix = CMatrix4x4::Rotation( CQuaternion( CVector3( 0, 0, 0 ) ) ) * CMatrix4x4::Scaling( mirror * scale ) * CMatrix4x4::Translation( location );
  dragged = draggedElement == UberToolElement::moveZ;
  hit = draggedElement == UberToolElement::none && !App->GetRightButtonState() && hitPart == UberToolElement::none && HitTestMoverArrow( bufferData, matrix, mouse1, mouse2, hoverPos );
  if ( hit )
    hitPart = UberToolElement::moveZ;

  UpdateObjectData( constBuffer, bufferData, matrix, hit || dragged ? CVector4( 1, 1, 0, dragged ? 1 : 0.5f ) : CVector4( 0, 0, 1, 0.5f ) );
  App->GetDevice()->DrawTriangles( arrow.triCount );


  API->SetUIRenderState();

  hoverElement = hitPart;
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
  SAFEDELETE( rasterizer1 );
  SAFEDELETE( rasterizer2 );
  SAFEDELETE( depthStencil );
}

CWBItem* GW2MarkerEditor::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new GW2MarkerEditor( Root, Pos );
}

bool GW2MarkerEditor::GetMouseTransparency( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  if ( draggedElement == UberToolElement::none && ( MessageType == WBM_RIGHTBUTTONDOWN || MessageType == WBM_RIGHTBUTTONUP ) )
    return true;

  if ( MessageType == WBM_MOUSEMOVE && draggedElement == UberToolElement::none )
    return true;


  if ( hoverElement != UberToolElement::none || draggedElement != UberToolElement::none )
    return false;

  auto tactical = App->GetRoot()->FindChildByID<GW2TacticalDisplay>( "tactical" );
  if ( !tactical )
    return true;

  for ( int x = 0; x < tactical->markerPositions.NumItems(); x++ )
    if ( tactical->markerPositions.GetByIndex( x ).Contains( ClientSpacePoint ) )
      return false;

  for ( int x = 0; x < tactical->markerMinimapPositions.NumItems(); x++ )
    if ( tactical->markerMinimapPositions.GetByIndex( x ).Contains( ClientSpacePoint ) )
      return false;

  return true;
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

CVector3 GW2MarkerEditor::GetUberToolMovePos( const CVector3& location )
{
  CRect drawrect = App->GetRoot()->GetPosition();

  float scale = ( mumbleLink.camPosition - location ).Length() * 0.1f;
  CVector3 eye = mumbleLink.camPosition;
  CVector3 mirror( eye.x > location.x ? 1.0f : -1.0f, eye.y > location.y ? 1.0f : -1.0f, eye.z > location.z ? 1.0f : -1.0f );

  ConstBuffer bufferData;
  bufferData.cam.SetLookAtLH( eye, mumbleLink.camPosition + mumbleLink.camDir, CVector3( 0, 1, 0 ) );
  bufferData.persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (TF32)drawrect.Height(), 0.01f, 150.0f );

  CMatrix4x4 matrix;

  POINT mousePos;
  mousePos.x = App->GetMousePos().x;
  mousePos.y = App->GetMousePos().y;

  if ( IsDebuggerPresent() )
    GetCursorPos( &mousePos );

  CVector4 mouse1 = CVector4( mousePos.x / (float)drawrect.Width() * 2 - 1, ( ( 1 - mousePos.y / (float)drawrect.Height() ) * 2 - 1 ), -1, 1 );
  CVector4 mouse2 = CVector4( mouse1.x, mouse1.y, 1, 1 );

  switch ( draggedElement )
  {
  case moveX:
    matrix = CMatrix4x4::Rotation( CQuaternion( 0, PI / 2.0f, 0 ) ) * CMatrix4x4::Scaling( mirror * scale ) * CMatrix4x4::Translation( location );
    return GetHitPositionMoverArrow( bufferData, matrix, mouse1, mouse2 ) * matrix;
  case moveY:
    matrix = CMatrix4x4::Rotation( CQuaternion( -PI / 2.0f, 0, 0 ) ) * CMatrix4x4::Scaling( mirror * scale ) * CMatrix4x4::Translation( location );
    return GetHitPositionMoverArrow( bufferData, matrix, mouse1, mouse2 ) * matrix;
  case moveZ:
    matrix = CMatrix4x4::Rotation( CQuaternion( CVector3( 0, 0, 0 ) ) ) * CMatrix4x4::Scaling( mirror * scale ) * CMatrix4x4::Translation( location );
    return GetHitPositionMoverArrow( bufferData, matrix, mouse1, mouse2 ) * matrix;
  case moveYZ:
    matrix = CMatrix4x4::Rotation( CQuaternion( 0, PI / 2.0f, 0 ) ) * CMatrix4x4::Scaling( CVector3( mirror.x, mirror.y, -mirror.z ) * scale ) * CMatrix4x4::Translation( location );
    return GetHitPositionMoverPlane( bufferData, matrix, mouse1, mouse2 ) * matrix;
  case moveXZ:
    matrix = CMatrix4x4::Rotation( CQuaternion( -PI / 2.0f, 0, 0 ) ) * CMatrix4x4::Scaling( CVector3( mirror.x, mirror.y, -mirror.z ) * scale ) * CMatrix4x4::Translation( location );
    return GetHitPositionMoverPlane( bufferData, matrix, mouse1, mouse2 ) * matrix;
  case moveXY:
    matrix = CMatrix4x4::Rotation( CQuaternion( CVector3( 0, 0, 0 ) ) ) * CMatrix4x4::Scaling( CVector3( mirror.x, mirror.y, -mirror.z ) * scale ) * CMatrix4x4::Translation( location );
    return GetHitPositionMoverPlane( bufferData, matrix, mouse1, mouse2 ) * matrix;
  case rotateX:
    break;
  case rotateY:
    break;
  case rotateZ:
    break;
  }

  return CVector3();
}

void GW2MarkerEditor::InitUberTool()
{
  constBuffer = App->GetDevice()->CreateConstantBuffer();

  rasterizer1 = App->GetDevice()->CreateRasterizerState();
  rasterizer1->SetCullMode( CORECULL_CW );
  rasterizer1->Update();
  rasterizer2 = App->GetDevice()->CreateRasterizerState();
  rasterizer2->SetCullMode( CORECULL_CCW );
  rasterizer2->Update();

  depthStencil = App->GetDevice()->CreateDepthStencilState();
  depthStencil->SetDepthEnable( true );
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

  float scale = moverPlaneSize;

  CArray<UberToolVertex> planeVertices;
  planeVertices += { CVector4( 0, 0, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 0, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 1, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 0, 0, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 1, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 0, 1, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };

  planeVertices += { CVector4( 1, 0, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 0, 0, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 1, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 1, 1, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 0, 0, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  planeVertices += { CVector4( 0, 1, 0, 1 / scale )* scale, CVector4( 1, 1, 1, 1 ) };
  plane.mesh = App->GetDevice()->CreateVertexBuffer( (TU8*)planeVertices.GetPointer( 0 ), planeVertices.NumItems() * sizeof( UberToolVertex ) );
  plane.triCount = planeVertices.NumItems() / 3;

  CArray<UberToolVertex> arrowVertices;

  float segCount = 12;
  float shaftRad = 0.02f;
  float headRad = 0.1f;
  float arrowHead = 0.7f;

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
    circleVertices += { CVector4( c2, s2, thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c1, s1, -thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c2, s2, thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
    circleVertices += { CVector4( c1, s1, thickness, 1 ), CVector4( 1, 1, 1, 1 ) };
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

    if ( b->GetID() == _T( "default1" ) || b->GetID() == _T( "default2" ) || b->GetID() == _T( "default3" ) || b->GetID() == _T( "default4" ) || b->GetID() == _T( "default5" ) )
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

    if ( b->GetID() == _T( "default1" ) )
      defaultCatBeingSet = 0;

    if ( b->GetID() == _T( "default2" ) )
      defaultCatBeingSet = 1;

    if ( b->GetID() == _T( "default3" ) )
      defaultCatBeingSet = 2;

    if ( b->GetID() == _T( "default4" ) )
      defaultCatBeingSet = 3;

    if ( b->GetID() == _T( "default5" ) )
      defaultCatBeingSet = 4;
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
        switch ( defaultCatBeingSet )
        {
        case 0: 
          Config::SetString( "defaultcategory0", categoryList[ message.Data ]->GetFullTypeName() );
          break;
        case 1:
          Config::SetString( "defaultcategory1", categoryList[ message.Data ]->GetFullTypeName() );
          break;
        case 2:
          Config::SetString( "defaultcategory2", categoryList[ message.Data ]->GetFullTypeName() );
          break;
        case 3:
          Config::SetString( "defaultcategory3", categoryList[ message.Data ]->GetFullTypeName() );
          break;
        case 4:
          Config::SetString( "defaultcategory4", categoryList[ message.Data ]->GetFullTypeName() );
          break;
        }
/*
        CWBLabel* type = (CWBLabel*)FindChildByID( "defaultmarkertype", "label" );
        if ( type )
          type->SetText( "Default Marker Type: " + categoryList[ message.Data ]->GetFullTypeName() );
*/
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
