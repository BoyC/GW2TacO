#include "MarkerEditor.h"
#include "MumbleLink.h"
#include "MarkerPack.h"
#include "OverlayConfig.h"
#include "TrailLogger.h"
#include "GW2Tactical.h"

extern CWBApplication* App;

enum TypeParameterTypes
{
  Boolean,
  String,
  Float,
  Int,
  Color,
  FloatNormalized,
  DropDown,
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
  Texture,

  MAX
};

struct TypeParameter
{
  TypeParameters param;
  char* targetName;
  char* enableCheckBoxName;
  TypeParameterTypes paramType;
};

TypeParameter typeParameters[ (int)TypeParameters::MAX ] = {

  {TypeParameters::Size,                 "size",           "cb_size",           TypeParameterTypes::Float },
  {TypeParameters::MiniMapSize,          "miniMapSize",    "cb_minimapsize",    TypeParameterTypes::Int },
  {TypeParameters::MiniMapFadeoutLevel,  "miniMapFadeOut", "cb_minimapfadeout", TypeParameterTypes::Float },
  {TypeParameters::MinSize,              "minSize",        "cb_minsize",        TypeParameterTypes::Int },
  {TypeParameters::MaxSize,              "maxSize",        "cb_maxsize",        TypeParameterTypes::Int },
  {TypeParameters::IconFile,             "icon",           "cb_icon",           TypeParameterTypes::String },
  {TypeParameters::ScaleWithZoom,        "scalewithzoom",  "cb_scalewithzoom",  TypeParameterTypes::Boolean },
  {TypeParameters::Color,                "color",          "cb_color",          TypeParameterTypes::Color },
  {TypeParameters::Alpha,                "alpha",          "cb_alpha",          TypeParameterTypes::FloatNormalized },
  {TypeParameters::FadeNear,             "fadeNear",       "cb_fadenear",       TypeParameterTypes::Float },
  {TypeParameters::FadeFar,              "fadeFar",        "cb_fadefar",        TypeParameterTypes::Float },
  {TypeParameters::Height,               "height",         "cb_height",         TypeParameterTypes::Float },
  {TypeParameters::Behavior,             "behavior",       "cb_behavior",       TypeParameterTypes::DropDown },
  {TypeParameters::AchievementID,        "achievementid",  "cb_achievementid",  TypeParameterTypes::Int },
  {TypeParameters::AchievementBit,       "achievementbit", "cb_achievementbit", TypeParameterTypes::Int },
  {TypeParameters::ResetLength,          "resetlength",    "cb_resetlength",    TypeParameterTypes::Int },
  {TypeParameters::DefaultToggle,        "defaulttoggle",  "cb_defaulttoggle",  TypeParameterTypes::Boolean },
  {TypeParameters::HasCountDown,         "hascountdown",   "cb_hascountdown",   TypeParameterTypes::Boolean },
  {TypeParameters::ToggleCategory,       "togglecategory", "cb_togglecategory", TypeParameterTypes::String },
  {TypeParameters::AutoTrigger,          "autotrigger",    "cb_autotrigger",    TypeParameterTypes::Boolean },
  {TypeParameters::TriggerRange,         "triggerrange",   "cb_triggerrange",   TypeParameterTypes::Float },
  {TypeParameters::InfoRange,            "inforange",      "cb_inforange",      TypeParameterTypes::Float },
  {TypeParameters::Info,                 "infotext",       "cb_infotext",       TypeParameterTypes::String },
  {TypeParameters::Copy,                 "copytext",       "cb_copytext",       TypeParameterTypes::String },
  {TypeParameters::CopyMessage,          "copymessage",    "cb_copymessage",    TypeParameterTypes::String },
  {TypeParameters::MiniMapVisible,       "minimapvisible", "cb_minimapvisible", TypeParameterTypes::Boolean },
  {TypeParameters::BigMapVisible,        "bigmapvisible",  "cb_bigmapvisible",  TypeParameterTypes::Boolean },
  {TypeParameters::InGameVisible,        "ingamevisible",  "cb_ingamevisible",  TypeParameterTypes::Boolean },
  {TypeParameters::KeepOnMapEdge,        "keeponmapedge",  "cb_keeponmapedge",  TypeParameterTypes::Boolean },
  {TypeParameters::AnimSpeed,            "trailanimspeed", "cb_trailanimspeed", TypeParameterTypes::Float },
  {TypeParameters::TrailScale,           "trailscale",     "cb_trailscale",     TypeParameterTypes::Float },
  {TypeParameters::Texture,              "trailtexture",   "cb_trailtexture",   TypeParameterTypes::String },
};

bool IsTypeParameterSaved( const MarkerTypeData& data, TypeParameters param )
{
  switch ( param )
  {
  case TypeParameters::Size:
    return data.saveBits.sizeSaved;
  case TypeParameters::MiniMapSize:
    return data.saveBits.miniMapSizeSaved;
  case TypeParameters::MiniMapFadeoutLevel:
    return data.saveBits.miniMapFadeOutLevelSaved;
  case TypeParameters::MinSize:
    return data.saveBits.minSizeSaved;
  case TypeParameters::MaxSize:
    return data.saveBits.maxSizeSaved;
  case TypeParameters::IconFile:
    return data.saveBits.iconFileSaved;
  case TypeParameters::ScaleWithZoom:
    return data.saveBits.scaleWithZoomSaved;
  case TypeParameters::Color:
    return data.saveBits.colorSaved;
  case TypeParameters::Alpha:
    return data.saveBits.alphaSaved;
  case TypeParameters::FadeNear:
    return data.saveBits.fadeNearSaved;
  case TypeParameters::FadeFar:
    return data.saveBits.fadeFarSaved;
  case TypeParameters::Height:
    return data.saveBits.heightSaved;
  case TypeParameters::Behavior:
    return data.saveBits.behaviorSaved;
  case TypeParameters::AchievementID:
    return data.saveBits.achievementIdSaved;
  case TypeParameters::AchievementBit:
    return data.saveBits.achievementBitSaved;
  case TypeParameters::ResetLength:
    return data.saveBits.resetLengthSaved;
  case TypeParameters::DefaultToggle:
    return data.saveBits.defaultToggleSaved;
  case TypeParameters::HasCountDown:
    return data.saveBits.hasCountdownSaved;
  case TypeParameters::ToggleCategory:
    return data.saveBits.toggleCategorySaved;
  case TypeParameters::AutoTrigger:
    return data.saveBits.autoTriggerSaved;
  case TypeParameters::TriggerRange:
    return data.saveBits.triggerRangeSaved;
  case TypeParameters::InfoRange:
    return data.saveBits.infoRangeSaved;
  case TypeParameters::Info:
    return data.saveBits.infoSaved;
  case TypeParameters::Copy:
    return data.saveBits.copySaved;
  case TypeParameters::CopyMessage:
    return data.saveBits.copyMessageSaved;
  case TypeParameters::MiniMapVisible:
    return data.saveBits.miniMapVisibleSaved;
  case TypeParameters::BigMapVisible:
    return data.saveBits.bigMapVisibleSaved;
  case TypeParameters::InGameVisible:
    return data.saveBits.inGameVisibleSaved;
  case TypeParameters::KeepOnMapEdge:
    return data.saveBits.keepOnMapEdgeSaved;
  case TypeParameters::AnimSpeed:
    return data.saveBits.animSpeedSaved;
  case TypeParameters::TrailScale:
    return data.saveBits.trailScaleSaved;
  case TypeParameters::Texture:
    return data.saveBits.textureSaved;
  }
  return false;
}

void SetTypeParameterSaved( MarkerTypeData& data, TypeParameters param, bool saved )
{
  switch ( param )
  {
  case TypeParameters::Size:
    data.saveBits.sizeSaved = saved;
    break;
  case TypeParameters::MiniMapSize:
    data.saveBits.miniMapSizeSaved = saved;
    break;
  case TypeParameters::MiniMapFadeoutLevel:
    data.saveBits.miniMapFadeOutLevelSaved = saved;
    break;
  case TypeParameters::MinSize:
    data.saveBits.minSizeSaved = saved;
    break;
  case TypeParameters::MaxSize:
    data.saveBits.maxSizeSaved = saved;
    break;
  case TypeParameters::IconFile:
    data.saveBits.iconFileSaved = saved;
    break;
  case TypeParameters::ScaleWithZoom:
    data.saveBits.scaleWithZoomSaved = saved; 
    break;
  case TypeParameters::Color:
    data.saveBits.colorSaved = saved; 
    break;
  case TypeParameters::Alpha:
    data.saveBits.alphaSaved = saved; 
    break;
  case TypeParameters::FadeNear:
    data.saveBits.fadeNearSaved = saved; 
    break;
  case TypeParameters::FadeFar:
    data.saveBits.fadeFarSaved = saved;
    break;
  case TypeParameters::Height:
    data.saveBits.heightSaved = saved; 
    break;
  case TypeParameters::Behavior:
    data.saveBits.behaviorSaved = saved; 
    break;
  case TypeParameters::AchievementID:
    data.saveBits.achievementIdSaved = saved;
    break;
  case TypeParameters::AchievementBit:
    data.saveBits.achievementBitSaved = saved; 
    break;
  case TypeParameters::ResetLength:
    data.saveBits.resetLengthSaved = saved; 
    break;
  case TypeParameters::DefaultToggle:
    data.saveBits.defaultToggleSaved = saved; 
    break;
  case TypeParameters::HasCountDown:
    data.saveBits.hasCountdownSaved = saved; 
    break;
  case TypeParameters::ToggleCategory:
    data.saveBits.toggleCategorySaved = saved; 
    break;
  case TypeParameters::AutoTrigger:
    data.saveBits.autoTriggerSaved = saved; 
    break;
  case TypeParameters::TriggerRange:
    data.saveBits.triggerRangeSaved = saved; 
    break;
  case TypeParameters::InfoRange:
    data.saveBits.infoRangeSaved = saved; 
    break;
  case TypeParameters::Info:
    data.saveBits.infoSaved = saved; 
    break;
  case TypeParameters::Copy:
    data.saveBits.copySaved = saved; 
    break;
  case TypeParameters::CopyMessage:
    data.saveBits.copyMessageSaved = saved; 
    break;
  case TypeParameters::MiniMapVisible:
    data.saveBits.miniMapVisibleSaved = saved; 
    break;
  case TypeParameters::BigMapVisible:
    data.saveBits.bigMapVisibleSaved = saved; 
    break;
  case TypeParameters::InGameVisible:
    data.saveBits.inGameVisibleSaved = saved; 
    break;
  case TypeParameters::KeepOnMapEdge:
    data.saveBits.keepOnMapEdgeSaved = saved; 
    break;
  case TypeParameters::AnimSpeed:
    data.saveBits.animSpeedSaved = saved; 
    break;
  case TypeParameters::TrailScale:
    data.saveBits.trailScaleSaved = saved; 
    break;
  case TypeParameters::Texture:
    data.saveBits.textureSaved = saved; 
    break;
  }
}

void SetTypeParameter( MarkerTypeData& data, TypeParameters param, const float& floatResult, const int& intResult, const CString& stringResult, const bool& boolResult )
{
  switch ( param )
  {
  case TypeParameters::Size:
    data.size = floatResult;
    return;
  case TypeParameters::MiniMapSize:
    data.miniMapSize = intResult;
    return;
  case TypeParameters::MiniMapFadeoutLevel:
    data.miniMapFadeOutLevel = floatResult;
    return;
  case TypeParameters::MinSize:
    data.minSize = intResult;
    return;
  case TypeParameters::MaxSize:
    data.maxSize = intResult;
    return;
  case TypeParameters::IconFile:
    data.iconFile = AddStringToMap( stringResult );
    return;
  case TypeParameters::ScaleWithZoom:    
    data.bits.scaleWithZoom = boolResult;
    return;
  case TypeParameters::Color:
    data.color = CColor::FromARGB( intResult );
    return;
  case TypeParameters::Alpha:
    data.alpha = floatResult;
    return;
  case TypeParameters::FadeNear:
    data.fadeNear = floatResult;
    return;
  case TypeParameters::FadeFar:
    data.fadeFar = floatResult;
    return;
  case TypeParameters::Height:
    data.height = floatResult;
    return;
  case TypeParameters::Behavior:
    data.behavior = (POIBehavior)intResult;
    return;
  case TypeParameters::AchievementID:
    data.achievementId = intResult;
    return;
  case TypeParameters::AchievementBit:
    data.achievementBit = intResult;
    return;
  case TypeParameters::ResetLength:
    data.resetLength = intResult;
    return;
  case TypeParameters::DefaultToggle:
    data.bits.defaultToggle = boolResult;
    return;
  case TypeParameters::HasCountDown:
    data.bits.hasCountdown = boolResult;
    return;
  case TypeParameters::ToggleCategory:
    data.toggleCategory = AddStringToMap( stringResult );
    return;
  case TypeParameters::AutoTrigger:
    data.bits.autoTrigger = boolResult;
    return;
  case TypeParameters::TriggerRange:
    data.triggerRange = floatResult;
    return;
  case TypeParameters::InfoRange:
    data.infoRange = floatResult;
    return;
  case TypeParameters::Info:
    data.info = AddStringToMap( stringResult );
    return;
  case TypeParameters::Copy:
    data.copy = AddStringToMap( stringResult );
    return;
  case TypeParameters::CopyMessage:
    data.copyMessage = AddStringToMap( stringResult );
    return;
  case TypeParameters::MiniMapVisible:
    data.bits.miniMapVisible = boolResult;
    return;
  case TypeParameters::BigMapVisible:
    data.bits.bigMapVisible = boolResult;
    return;
  case TypeParameters::InGameVisible:
    data.bits.inGameVisible = boolResult;
    return;
  case TypeParameters::KeepOnMapEdge:
    data.bits.keepOnMapEdge = boolResult;
    return;
  case TypeParameters::AnimSpeed:
    data.animSpeed = floatResult;
    return;
  case TypeParameters::TrailScale:
    data.trailScale = floatResult;
    return;
  case TypeParameters::Texture:
    data.texture = AddStringToMap( stringResult );
    return;
  }
}

void GetTypeParameter( const MarkerTypeData& data, TypeParameters param, float& floatResult, int& intResult, CString& stringResult, bool& boolResult )
{
  switch ( param )
  {
  case TypeParameters::Size:
    floatResult = data.size;
    return;
  case TypeParameters::MiniMapSize:
    intResult = data.miniMapSize;
    return;
  case TypeParameters::MiniMapFadeoutLevel:
    floatResult = data.miniMapFadeOutLevel;
    return;
  case TypeParameters::MinSize:
    intResult = data.minSize;
    return;
  case TypeParameters::MaxSize:
    intResult = data.maxSize;
    return;
  case TypeParameters::IconFile:
    stringResult = GetStringFromMap( data.iconFile );
    return;
  case TypeParameters::ScaleWithZoom:
    boolResult = data.bits.scaleWithZoom;
    return;
  case TypeParameters::Color:
    intResult = data.color.argb();
    return;
  case TypeParameters::Alpha:
    floatResult = data.alpha;
    return;
  case TypeParameters::FadeNear:
    floatResult = data.fadeNear;
    return;
  case TypeParameters::FadeFar:
    floatResult = data.fadeFar;
    return;
  case TypeParameters::Height:
    floatResult = data.height;
    return;
  case TypeParameters::Behavior:
    intResult = (int)data.behavior;
    return;
  case TypeParameters::AchievementID:
    intResult = data.achievementId;
    return;
  case TypeParameters::AchievementBit:
    intResult = data.achievementBit;
    return;
  case TypeParameters::ResetLength:
    intResult = data.resetLength;
    return;
  case TypeParameters::DefaultToggle:
    boolResult = data.bits.defaultToggle;
    return;
  case TypeParameters::HasCountDown:
    boolResult = data.bits.hasCountdown;
    return;
  case TypeParameters::ToggleCategory:
    stringResult = GetStringFromMap( data.toggleCategory );
    return;
  case TypeParameters::AutoTrigger:
    boolResult = data.bits.autoTrigger;
    return;
  case TypeParameters::TriggerRange:
    floatResult = data.triggerRange;
    return;
  case TypeParameters::InfoRange:
    floatResult = data.infoRange;
    return;
  case TypeParameters::Info:
    stringResult = GetStringFromMap( data.info );
    return;
  case TypeParameters::Copy:
    stringResult = GetStringFromMap( data.copy );
    return;
  case TypeParameters::CopyMessage:
    stringResult = GetStringFromMap( data.copyMessage );
    return;
  case TypeParameters::MiniMapVisible:
    boolResult = data.bits.miniMapVisible;
    return;
  case TypeParameters::BigMapVisible:
    boolResult = data.bits.bigMapVisible;
    return;
  case TypeParameters::InGameVisible:
    boolResult = data.bits.inGameVisible;
    return;
  case TypeParameters::KeepOnMapEdge:
    boolResult = data.bits.keepOnMapEdge;
    return;
  case TypeParameters::AnimSpeed:
    floatResult = data.animSpeed;
    return;
  case TypeParameters::TrailScale:
    floatResult = data.trailScale;
    return;
  case TypeParameters::Texture:
    stringResult = GetStringFromMap( data.texture );
    return;
  }
}


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
          SetEditedGUID( tactical->markerPositions.GetKDPair( x )->Key );
          found = true;
        }

      for ( int x = 0; x < tactical->markerMinimapPositions.NumItems(); x++ )
        if ( tactical->markerMinimapPositions.GetByIndex( x ).Contains( mouse ) )
        {
          SetEditedGUID( tactical->markerMinimapPositions.GetKDPair( x )->Key );
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

void GW2MarkerEditor::UpdateEditorFromCategory( const MarkerTypeData& data )
{
  for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
  {
    CWBButton* checkBox = FindChildByID<CWBButton>( typeParameters[ x ].enableCheckBoxName );
    if ( checkBox )
      checkBox->Push( IsTypeParameterSaved( data, (TypeParameters)x ) );

    bool boolData = 0;
    int intData = 0;
    float floatData = 0;
    CString stringData;

    GetTypeParameter( data, (TypeParameters)x, floatData, intData, stringData, boolData );

    switch ( typeParameters[ x ].paramType )
    {
    case Boolean:
    {
      CWBButton* button = FindChildByID<CWBButton>( typeParameters[ x ].targetName );
      if ( button )
        button->Push( boolData );
      break;
    }
    case String:
    {
      CWBTextBox* textBox = FindChildByID<CWBTextBox>( typeParameters[ x ].targetName );
      if ( textBox )
        textBox->SetText( stringData );
      break;
    }
    case FloatNormalized:
    case Float:
    {
      CWBTextBox* textBox = FindChildByID<CWBTextBox>( typeParameters[ x ].targetName );
      if ( textBox )
        textBox->SetText( CString::Format( "%f", floatData ) );
      break;
    }
    case Int:
    {
      CWBTextBox* textBox = FindChildByID<CWBTextBox>( typeParameters[ x ].targetName );
      if ( textBox )
        textBox->SetText( CString::Format( "%d", intData ) );
      break;
    }
    case Color:
    {
      break;
    }
    case DropDown:
    {
      CWBDropDown* dropDown = (CWBDropDown*)FindChildByID( typeParameters[ x ].targetName );
      if ( dropDown )
        dropDown->SelectItemByIndex( intData );
      break;
    }
    }
  }
}

void GW2MarkerEditor::SetEditedGUID( const GUID& guid )
{
  editedCategory = CString();

  auto editedText = FindChildByID<CWBLabel>( "categorylabel" );
  auto marker = FindMarkerByGUID( guid );

  if ( !marker )
  {
    editedMarker = GUID{};
    if ( editedText )
      editedText->SetText( "NO EDITED ITEM CURRENTLY!" );
    return;
  }

  if ( editedText )
    editedText->SetText( "EDITED ITEM IS A MARKER" );

  editedMarker = guid;
  UpdateEditorFromCategory( marker->typeData );
}

void GW2MarkerEditor::SetEditedCategory( const CString& category )
{
  editedMarker = GUID{};

  auto editedText = FindChildByID<CWBLabel>( "categorylabel" );
  auto cat = GetCategory( category );

  if ( !cat )
  {
    editedCategory = CString();
    if ( editedText )
      editedText->SetText( "NO EDITED ITEM CURRENTLY!" );
    return;
  }

  if ( editedText )
    editedText->SetText( "Editing Category: " + cat->GetFullTypeName() );

  editedCategory = category;
  UpdateEditorFromCategory( cat->data );
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

    changeDefault = false;
    selectingEditedCategory = false;

    if ( b->GetID() == _T( "changemarkertype" ) )
    {
      auto ctx = b->OpenContextMenu( App->GetMousePos() );
      OpenTypeContextMenu( ctx, categoryList, false, 0, true );
    }

    if ( b->GetID() == _T( "Editcategory" ) )
    {
      auto ctx = b->OpenContextMenu( App->GetMousePos() );
      OpenTypeContextMenu( ctx, categoryList, false, 0, true );
      selectingEditedCategory = true;
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
      if ( changeDefault )
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
        break;
      }

      if ( selectingEditedCategory )
      {
        SetEditedCategory( categoryList[ message.Data ]->GetFullTypeName() );
        break;
      }

      auto& POIs = GetMapPOIs();

      POIs[ currentPOI ].SetCategory( App, categoryList[ message.Data ] );
      ExportPOIS();
      CWBLabel* type = (CWBLabel*)FindChildByID( "markertype", "label" );
      if ( type )
        type->SetText( "Marker Type: " + categoryList[ message.Data ]->GetFullTypeName() );
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
