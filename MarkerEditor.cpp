#include "MarkerEditor.h"
#include "MumbleLink.h"
#include "MarkerPack.h"
#include "OverlayConfig.h"
#include "TrailLogger.h"
#include "GW2Tactical.h"
#include "Language.h"

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

struct TypeParameter
{
  TypeParameters param;
  char* targetName;
  char* enableCheckBoxName;
  TypeParameterTypes paramType;
  bool markerData;
  bool trailData;
};

TypeParameter typeParameters[ (int)TypeParameters::MAX ] = {

  {TypeParameters::Size,                 "size",           "cb_size",           TypeParameterTypes::Float           , true,  false },
  {TypeParameters::MiniMapSize,          "miniMapSize",    "cb_minimapsize",    TypeParameterTypes::Int             , true,  true },
  {TypeParameters::MiniMapFadeoutLevel,  "miniMapFadeOut", "cb_minimapfadeout", TypeParameterTypes::Float           , true,  true },
  {TypeParameters::MinSize,              "minSize",        "cb_minsize",        TypeParameterTypes::Int             , true,  false },
  {TypeParameters::MaxSize,              "maxSize",        "cb_maxsize",        TypeParameterTypes::Int             , true,  false },
  {TypeParameters::IconFile,             "icon",           "cb_icon",           TypeParameterTypes::String          , true,  false },
  {TypeParameters::ScaleWithZoom,        "scalewithzoom",  "cb_scalewithzoom",  TypeParameterTypes::Boolean         , true,  true },
  {TypeParameters::Color,                "color",          "cb_color",          TypeParameterTypes::Color           , true,  true },
  {TypeParameters::Alpha,                "alpha",          "cb_alpha",          TypeParameterTypes::FloatNormalized , true,  true },
  {TypeParameters::FadeNear,             "fadeNear",       "cb_fadenear",       TypeParameterTypes::Float           , true,  true },
  {TypeParameters::FadeFar,              "fadeFar",        "cb_fadefar",        TypeParameterTypes::Float           , true,  true },
  {TypeParameters::Height,               "height",         "cb_height",         TypeParameterTypes::Float           , true,  false },
  {TypeParameters::Behavior,             "behavior",       "cb_behavior",       TypeParameterTypes::DropDown        , true,  false },
  {TypeParameters::AchievementID,        "achievementid",  "cb_achievementid",  TypeParameterTypes::Int             , true,  true  },
  {TypeParameters::AchievementBit,       "achievementbit", "cb_achievementbit", TypeParameterTypes::Int             , true,  true  },
  {TypeParameters::ResetLength,          "resetlength",    "cb_resetlength",    TypeParameterTypes::Int             , true,  false },
  {TypeParameters::DefaultToggle,        "defaulttoggle",  "cb_defaulttoggle",  TypeParameterTypes::Boolean         , false, false }, // this is category only
  {TypeParameters::HasCountDown,         "hascountdown",   "cb_hascountdown",   TypeParameterTypes::Boolean         , true,  false },
  {TypeParameters::ToggleCategory,       "togglecategory", "cb_togglecategory", TypeParameterTypes::String          , true,  false },
  {TypeParameters::AutoTrigger,          "autotrigger",    "cb_autotrigger",    TypeParameterTypes::Boolean         , true,  false },
  {TypeParameters::TriggerRange,         "triggerrange",   "cb_triggerrange",   TypeParameterTypes::Float           , true,  false },
  {TypeParameters::InfoRange,            "inforange",      "cb_inforange",      TypeParameterTypes::Float           , true,  false },
  {TypeParameters::Info,                 "infotext",       "cb_infotext",       TypeParameterTypes::String          , true,  false },
  {TypeParameters::Copy,                 "copytext",       "cb_copytext",       TypeParameterTypes::String          , true,  false },
  {TypeParameters::CopyMessage,          "copymessage",    "cb_copymessage",    TypeParameterTypes::String          , true,  false },
  {TypeParameters::MiniMapVisible,       "minimapvisible", "cb_minimapvisible", TypeParameterTypes::Boolean         , true,  true  },
  {TypeParameters::BigMapVisible,        "bigmapvisible",  "cb_bigmapvisible",  TypeParameterTypes::Boolean         , true,  true  },
  {TypeParameters::InGameVisible,        "ingamevisible",  "cb_ingamevisible",  TypeParameterTypes::Boolean         , true,  true  },
  {TypeParameters::KeepOnMapEdge,        "keeponmapedge",  "cb_keeponmapedge",  TypeParameterTypes::Boolean         , true,  false },
  {TypeParameters::AnimSpeed,            "trailanimspeed", "cb_trailanimspeed", TypeParameterTypes::Float           , false, true  },
  {TypeParameters::TrailScale,           "trailscale",     "cb_trailscale",     TypeParameterTypes::Float           , false, true  },
  {TypeParameters::TrailFile,            "trailfile",      "cb_trailfile",      TypeParameterTypes::String          , false, true  },
  {TypeParameters::Texture,              "trailtexture",   "cb_trailtexture",   TypeParameterTypes::String          , false, true  },
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
  case TypeParameters::TrailFile:
    return data.saveBits.trailDataSaved;
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
  case TypeParameters::TrailFile:
    data.saveBits.trailDataSaved = saved;
    break;
  case TypeParameters::Texture:
    data.saveBits.textureSaved = saved;
    break;
  }
}

void SetTypeParameter( MarkerTypeData& data, TypeParameters param, const float floatResult, const int intResult, const CString& stringResult, const bool boolResult )
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
  case TypeParameters::TrailFile:
    data.trailData = AddStringToMap( stringResult );
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
  case TypeParameters::TrailFile:
    stringResult = GetStringFromMap( data.trailData );
    return;
  case TypeParameters::Texture:
    stringResult = GetStringFromMap( data.texture );
    return;
  }
}


void DrawRangeCircle( CWBDrawAPI* API, const CRect& drawrect, CMatrix4x4& cam, CMatrix4x4& persp, const CVector4& pos, float scale, const CVector3& _x, const CVector3& _y, const CColor& color )
{
  int resolution = 60;

  CVector4 campos = CVector4( mumbleLink.camPosition.x, mumbleLink.camPosition.y, mumbleLink.camPosition.z, 1.0f );
  //CVector2 camDir = CVector2( camSpaceChar.x - campos.x, camSpaceChar.z - campos.z ).Normalized();

  CVector4 charpos = CVector4( mumbleLink.averagedCharPosition.x, mumbleLink.averagedCharPosition.y, mumbleLink.averagedCharPosition.z, 1.0f );;

  CVector4 camSpaceChar = charpos * cam;
  camSpaceChar /= camSpaceChar.w;
  CVector4 camSpaceEye = charpos + CVector4( 0, 1, 0, 0 );

  CVector4 screenSpaceChar = ( camSpaceEye * cam ) * persp;
  screenSpaceChar /= screenSpaceChar.w;
  CVector4 screenSpaceEye = ( camSpaceEye * cam ) * persp;
  screenSpaceEye /= screenSpaceEye.w;

  CColor col1 = color;
  CColor col2 = color;

  for ( int x = 0; x < resolution; x++ )
  {
    float a1 = 1.0f;
    float a2 = 1.0f;
    float f1 = x / (TF32)resolution * PI * 2;
    float f2 = ( x + 1 ) / (TF32)resolution * PI * 2;
    CVector3 _p1 = ( _x * sinf( f1 ) + _y * cosf( f1 ) ) * scale;
    CVector3 _p2 = ( _x * sinf( f2 ) + _y * cosf( f2 ) ) * scale;
    CVector4 p1( _p1.x, _p1.y, _p1.z, 1 );
    CVector4 p2( _p2.x, _p2.y, _p2.z, 1 );

    //CVector3 toPoint = CVector3( p1 - campos );

    /*if ( !zoomedin )
    {
      a1 = 1 - powf( max( 0, camDir * CVector2( p1.x, p1.z ).Normalized() ), 10.0f );
      a2 = 1 - powf( max( 0, camDir * CVector2( p2.x, p2.z ).Normalized() ), 10.0f );
    }*/

    p1 = p1 + pos;
    p2 = p2 + pos;

    p1 = p1 * cam;
    p2 = p2 * cam;

    p1 /= p1.w;
    p2 /= p2.w;

    if ( p1.z < 0.01 && p2.z < 0.01 )
      continue;

    bool behind1 = p1.z > camSpaceChar.z;
    bool behind2 = p2.z > camSpaceChar.z;

    p1.z = max( 0.01f, p1.z );
    p2.z = max( 0.01f, p2.z );

    p1 = p1 * persp;
    p2 = p2 * persp;
    p1 /= p1.w;
    p2 /= p2.w;

    if ( behind1 )
      a1 = 1 - /*( 1 - a1 ) **/ ( 1 - powf( ( CVector2( screenSpaceChar.x, screenSpaceChar.y ) - CVector2( p1.x, p1.y ) ).Length() * 8, 10.0f ) );

    if ( behind2 )
      a2 = 1 - /*( 1 - a1 ) **/ ( 1 - powf( ( CVector2( screenSpaceChar.x, screenSpaceChar.y ) - CVector2( p2.x, p2.y ) ).Length() * 8, 10.0f ) );


    p1 = p1 * 0.5 + CVector4( 0.5, 0.5, 0.5, 0.0 );
    p2 = p2 * 0.5 + CVector4( 0.5, 0.5, 0.5, 0.0 );

    CPoint pa = CPoint( (int)( p1.x * drawrect.Width() ), (int)( ( 1 - p1.y ) * drawrect.Height() ) );
    CPoint pb = CPoint( (int)( p2.x * drawrect.Width() ), (int)( ( 1 - p2.y ) * drawrect.Height() ) );

    col1.A() = (int)( max( 0, min( 1, a1 ) ) * 255 );
    col2.A() = (int)( max( 0, min( 1, a2 ) ) * 255 );

    API->DrawLine( pa, pb, col1, col2 );
  }
}

void DrawRangeDisplay( CWBDrawAPI* API, const CRect& drawrect, CMatrix4x4& cam, CMatrix4x4& persp, CVector3& pos, float scale, const CColor& color )
{
  DrawRangeCircle( API, drawrect, cam, persp, CVector4( pos.x, pos.y, pos.z, 0 ), scale, CVector3( 1, 0, 0 ), CVector3( 0, 1, 0 ), color );
  DrawRangeCircle( API, drawrect, cam, persp, CVector4( pos.x, pos.y, pos.z, 0 ), scale, CVector3( 1, 0, 0 ), CVector3( 0, 0, 1 ), color );
  DrawRangeCircle( API, drawrect, cam, persp, CVector4( pos.x, pos.y, pos.z, 0 ), scale, CVector3( 0, 1, 0 ), CVector3( 0, 0, 1 ), color );

/*
  DrawRangeCircle( API, drawrect, cam, persp, CVector4( pos.x, pos.y, pos.z, 0 ), scale, CVector3( 0.5, 0, 0.5 ).Normalized(), CVector3( 0, 1, 0 ), color );
  DrawRangeCircle( API, drawrect, cam, persp, CVector4( pos.x, pos.y, pos.z, 0 ), scale, CVector3( -0.5, 0, 0.5 ).Normalized(), CVector3( 0, 0, 1 ), color );
*/
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

    if ( App->GetMouseItem() != App->GetRoot() )
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

      auto* trail = FindTrailByGUID( editedMarker );
      if ( trail )
      {
        clickedPos = hoverPos;
        draggedElement = hoverElement;

        clickedOriginalDelta = GetUberToolMovePos( trail->GetVertex( selectedVertexIndex ) ) - trail->GetVertex( selectedVertexIndex );
        originalPosition = trail->GetVertex( selectedVertexIndex );
        //originalRotation = marker->rotation;
        originalScale = CVector3( 1, 1, 1 );// *marker->typeData.size;
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

    int currIdx = 0;
    CVector3 currHitPoint;
    bool indexPlusOne{};

    GUID mouseTrail = GetMouseTrail( currIdx, currHitPoint, indexPlusOne );
    if ( mouseTrail != GUID{} )
    {
      App->UpdateControlKeyStates();
      if ( !App->GetCtrlState() )
      {
        selectedVertexIndex = currIdx;
        originalPosition = GetSelectedVertexPosition();
        SetEditedGUID( mouseTrail );
      }
      else
      {
        int currIdx = 0;
        CVector3 currHitPoint;
        bool indexPlusOne{};

        GUID mouseTrail = GetMouseTrail( currIdx, currHitPoint, indexPlusOne );
        if ( mouseTrail != GUID{} )
        {
          auto* trail = FindTrailByGUID( mouseTrail );
          if ( !trail )
            break;

          trail->AddVertex( indexPlusOne ? currIdx : currIdx + 1, currHitPoint );
          selectedVertexIndex = indexPlusOne ? currIdx : currIdx + 1;
          originalPosition = currHitPoint;

          return true;
        }
      }
      return true;
    }

    auto mouseItem = App->GetItemUnderMouse( App->GetMousePos(), WBM_LEFTBUTTONDOWN );

    if ( !mouseItem || mouseItem == tactical || mouseItem->GetID() == "tacoroot" )
    {
      if ( editedMarker != GUID{} )
        SetEditedGUID( GUID{} );
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
  case WBM_RIGHTBUTTONDOWN:
  {
    if ( draggedElement == UberToolElement::none || editedMarker == GUID{} )
      break;
    auto* marker = FindMarkerByGUID( editedMarker );
    if ( marker )
      marker->position = originalPosition;
    auto* trail = FindTrailByGUID( editedMarker );
    if ( trail )
      trail->SetVertex( selectedVertexIndex, originalPosition );
    return true;
  }
  break;
  case WBM_RIGHTBUTTONUP:
  case WBM_MOUSEMOVE:
    if ( draggedElement != UberToolElement::none && editedMarker != GUID{} )
    {
      auto* marker = FindMarkerByGUID( editedMarker );
      if ( marker )
      {
        CVector3 pos = GetUberToolMovePos( originalPosition );

        if ( !App->GetRightButtonState() )
          marker->position = pos - clickedOriginalDelta;
      }

      auto* trail = FindTrailByGUID( editedMarker );
      if ( trail )
      {
        CVector3 pos = GetUberToolMovePos( originalPosition );

        if ( !App->GetRightButtonState() )
          trail->SetVertex( selectedVertexIndex, pos - clickedOriginalDelta );
      }

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

  if ( mumbleLink.isMapOpen )
    return;

  if ( editedMarker == GUID{} )
    return;

  auto* marker = FindMarkerByGUID( editedMarker );
  auto* trail = FindTrailByGUID( editedMarker );
  if ( !marker && !trail )
    return;

  CVector3 location = marker ? marker->position : trail->GetVertex( selectedVertexIndex );
  if ( marker )
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
  bufferData.persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (TF32)drawrect.Height(), 0.05f, 1000.0f );

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


  if ( marker )
  {
    if ( marker->routeMember || ( marker->typeData.behavior != POIBehavior::AlwaysVisible && marker->typeData.behavior != POIBehavior::WvWObjective ) )
      DrawRangeDisplay( API, App->GetRoot()->GetClientRect(), bufferData.cam, bufferData.persp, location, marker->typeData.triggerRange, CColor::FromARGB( 0xffffff80 ) );

    if ( marker->typeData.info != -1 )
      DrawRangeDisplay( API, App->GetRoot()->GetClientRect(), bufferData.cam, bufferData.persp, location, marker->typeData.infoRange, CColor::FromARGB( 0xff80ff80 ) );
  }

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
  SetEditedGUID( GUID{} );
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
  if ( hidden )
    return true;

  if ( draggedElement == UberToolElement::none && ( MessageType == WBM_RIGHTBUTTONDOWN || MessageType == WBM_RIGHTBUTTONUP ) )
    return true;

  if ( MessageType == WBM_MOUSEMOVE && draggedElement == UberToolElement::none )
    return true;

  // for right click undo
  if ( draggedElement != UberToolElement::none && ( MessageType == WBM_RIGHTBUTTONDOWN || MessageType == WBM_RIGHTBUTTONUP ) )
    return false;

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

  if ( editedMarker != GUID{} )
    return false;

  if ( MessageType == WBM_LEFTBUTTONDOWN )
  {
    int currIdx = 0;
    CVector3 currHitPoint;
    bool indexPlusOne;
    if ( GetMouseTrail( currIdx, currHitPoint, indexPlusOne ) != GUID{} )
      return false;
  }

  return true;
}

bool GW2MarkerEditor::ShouldPassMouseEvent()
{
  return editedMarker != GUID{} && hoverElement == UberToolElement::none;
}

void GW2MarkerEditor::OnDraw( CWBDrawAPI* API )
{
  TBOOL autoHide = false;// Config::GetValue( "AutoHideMarkerEditor" );

  if ( !Config::GetValue( "TacticalLayerVisible" ) )
    return;

  if ( !mumbleLink.IsValid() ) return;

  if ( mumbleLink.mapID == -1 ) return;

/*
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
*/

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

  auto* mouseItem = App->GetMouseItem();

  if ( mouseItem )
  {
    if ( mouseItem->GetID() == "default1" )
      SetMouseToolTip( Config::GetString( "defaultcategory0" ) );
    if ( mouseItem->GetID() == "default2" )
      SetMouseToolTip( Config::GetString( "defaultcategory1" ) );
    if ( mouseItem->GetID() == "default3" )
      SetMouseToolTip( Config::GetString( "defaultcategory2" ) );
    if ( mouseItem->GetID() == "default4" )
      SetMouseToolTip( Config::GetString( "defaultcategory3" ) );
    if ( mouseItem->GetID() == "default5" )
      SetMouseToolTip( Config::GetString( "defaultcategory4" ) );
    if ( mouseItem->GetID() == "starttrail" )
      SetMouseToolTip( DICT( "starttrail" ) );
    if ( mouseItem->GetID() == "pausetrail" )
      SetMouseToolTip( DICT( "pausetrail" ) );
    if ( mouseItem->GetID() == "startnewsection" )
      SetMouseToolTip( DICT( "startnewsection" ) );
    if ( mouseItem->GetID() == "deletelastsegment" )
      SetMouseToolTip( DICT( "deletelastsegment" ) );
    if ( mouseItem->GetID() == "savetrail" )
      SetMouseToolTip( DICT( "savetrail" ) );
    if ( mouseItem->GetID() == "loadtrail" )
      SetMouseToolTip( DICT( "loadtrail" ) );
    if ( mouseItem->GetID() == "newtrail" )
      SetMouseToolTip( DICT( "newtrail" ) );
    if ( mouseItem->GetID() == "prevsegment" )
      SetMouseToolTip( DICT( "prevsegment" ) );
    if ( mouseItem->GetID() == "nextsegment" )
      SetMouseToolTip( DICT( "nextsegment" ) );
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
    {
      bool saved = IsTypeParameterSaved( data, (TypeParameters)x );
      checkBox->Push( saved );
      if ( checkBox->GetParent() )
        checkBox->GetParent()->SetTreeOpacityMultiplier( saved ? 1 : 0.5f );
    }

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
        textBox->SetText( stringData, false, true );
      break;
    }
    case FloatNormalized:
    case Float:
    {
      CWBTextBox* textBox = FindChildByID<CWBTextBox>( typeParameters[ x ].targetName );
      if ( textBox )
        textBox->SetText( CString::Format( "%.2f", floatData ), false, true );
      break;
    }
    case Int:
    {
      CWBTextBox* textBox = FindChildByID<CWBTextBox>( typeParameters[ x ].targetName );
      if ( textBox )
        textBox->SetText( CString::Format( "%d", intData ), false, true );
      break;
    }
    case Color:
    {
      CWBTextBox* textBox = FindChildByID<CWBTextBox>( typeParameters[ x ].targetName );
      if ( textBox )
        textBox->SetText( CString::Format( "%8X", intData ), false, true );
      CWBButton* preview = FindChildByID<CWBButton>( "colorpreview" );
      if ( preview )
        preview->ApplyStyleDeclarations( CString::Format( "background:#%8X", intData ) );
      break;
    }
    case DropDown:
    {
      CWBDropDown* dropDown = (CWBDropDown*)FindChildByID( typeParameters[ x ].targetName );
      if ( dropDown )
        dropDown->SelectItemByIndex( intData, true );
      break;
    }
    }
  }
}

void GW2MarkerEditor::UpdateEditorContent()
{
  MarkerTypeData* data = GetEditedTypeParameters();
  if ( !data )
    return;
  UpdateEditorFromCategory( *data );
}

void GW2MarkerEditor::SetEditedGUID( const GUID& guid )
{
  editedCategory = CString();

  auto editedText = FindChildByID<CWBLabel>( "categorylabel" );
  auto marker = FindMarkerByGUID( guid );
  auto trail = FindTrailByGUID( guid );

  auto changetype = FindChildByID( "changemarkertype" );
  if ( changetype )
    changetype->Hide( !marker && !trail );
  auto deleteMarker = FindChildByID( "deletemarker" );
  if ( deleteMarker )
    deleteMarker->Hide( !marker && !trail );

  bool allowExternals = Config::GetValue( "EnableExternalEditing" ) > 0;

  if ( ( marker && !allowExternals && marker->external ) || ( trail && !allowExternals && trail->External ) )
  {
    editedMarker = GUID{};
    if ( editedText )
      editedText->SetText( "NO EDITED ITEM CURRENTLY!" );
    HideEditorUI( true );
    if ( changetype )
      changetype->Hide( true );
    if ( deleteMarker )
      deleteMarker->Hide( true );
    return;
  }

  if ( !marker && !trail )
  {
    editedMarker = GUID{};
    if ( editedText )
      editedText->SetText( "NO EDITED ITEM CURRENTLY!" );

    HideEditorUI( true );
    if ( changetype )
      changetype->Hide( true );
    if ( deleteMarker )
      deleteMarker->Hide( true );
    return;
  }

  if ( editedText )
  {
    if ( marker )
      editedText->SetText( "EDITED ITEM IS A MARKER (" + marker->category->GetFullTypeName() + ")" );
    else
      editedText->SetText( "EDITED ITEM IS A TRAIL (" + trail->category->GetFullTypeName() + ")" );
  }

  editedMarker = guid;
  HideEditorUI( true );
  ShowMarkerUI( marker, trail );
  UpdateEditorFromCategory( marker ? marker->typeData : trail->typeData );
}

void GW2MarkerEditor::SetEditedCategory( const CString& category )
{
  editedMarker = GUID{};

  auto editedText = FindChildByID<CWBLabel>( "categorylabel" );
  auto cat = GetCategory( category );

  auto changetype = FindChildByID( "changemarkertype" );
  if ( changetype )
    changetype->Hide( true );
  auto deleteMarker = FindChildByID( "deletemarker" );
  if ( deleteMarker )
    deleteMarker->Hide( true );

  if ( !cat )
  {
    editedCategory = CString();
    if ( editedText )
      editedText->SetText( "NO EDITED ITEM CURRENTLY!" );

    HideEditorUI( true );
    return;
  }

  if ( editedText )
    editedText->SetText( "Editing Category: " + cat->GetFullTypeName() );

  editedCategory = category;
  HideEditorUI( false );
  UpdateEditorFromCategory( cat->data );
}

void GW2MarkerEditor::DeleteSelectedMarker()
{
  DeletePOI( editedMarker );
}

GUID GW2MarkerEditor::GetEditedGUID()
{
  return editedMarker;
}

int GW2MarkerEditor::GetSelectedVertexIndex()
{
  return selectedVertexIndex;
}

CVector3 GW2MarkerEditor::GetSelectedVertexPosition()
{
  auto* trail = FindTrailByGUID( editedMarker );
  if ( !trail )
    return CVector3();

  return trail->GetVertex( selectedVertexIndex );
}

void UpdatePOICategoryParameter( GW2TacticalCategory* category, TypeParameters param, const bool boolValue, const int intValue, const float floatValue, const CString& stringValue )
{
  for ( auto& set : POISet )
  {
    for ( int x = 0; x < set.second.NumItems(); x++ )
    {
      auto& poi = set.second.GetByIndex( x );
      if ( poi.category == category )
      {
        if ( !IsTypeParameterSaved( poi.category->data, param ) )
          SetTypeParameter( poi.typeData, param, floatValue, intValue, stringValue, boolValue );
      }
    }
  }

  for ( auto& set : trailSet )
  {
    for ( int x = 0; x < set.second.NumItems(); x++ )
    {
      auto& trail = set.second.GetByIndex( x );
      if ( !trail || !trail->category )
        continue;
      if ( trail->category == category )
      {
        if ( !IsTypeParameterSaved( trail->category->data, param ) )
        {
          SetTypeParameter( trail->typeData, param, floatValue, intValue, stringValue, boolValue );
          if ( param == TypeParameters::TrailFile )
            trail->Reload();
        }
      }
    }
  }
}

void PropagateCategoryParameter( GW2TacticalCategory* category, TypeParameters param, const bool boolValue, const int intValue, const float floatValue, const CString& stringValue )
{
  if ( !category )
    return;

  if ( IsTypeParameterSaved( category->data, param ) )
    return;

  UpdatePOICategoryParameter( category, param, boolValue, intValue, floatValue, stringValue );

  SetTypeParameter( category->data, param, floatValue, intValue, stringValue, boolValue );
  for ( int x = 0; x < category->children.NumItems(); x++ )
    PropagateCategoryParameter( category->children[ x ], param, boolValue, intValue, floatValue, stringValue );
}

void GW2MarkerEditor::UpdateTypeParameterValue( TypeParameters param )
{
  MarkerTypeData* data = GetEditedTypeParameters();
  if ( !data )
    return;

  bool originalBoolValue{};
  int originalIntValue{};
  float originalFloatValue{};
  CString originalStringvalue;

  GetTypeParameter( *data, param, originalFloatValue, originalIntValue, originalStringvalue, originalBoolValue );
  bool changed = false;

  // update

  switch ( typeParameters[ (int)param ].paramType )
  {
  case Boolean:
  {
    CWBButton* b = FindChildByID<CWBButton>( typeParameters[ (int)param ].targetName );
    if ( !b )
      return;

    changed = ( b->IsPushed() != 0 ) != originalBoolValue;
    SetTypeParameter( *data, param, 0, 0, "", b->IsPushed() );
    break;
  }
  case String:
  {
    CWBTextBox* b = FindChildByID<CWBTextBox>( typeParameters[ (int)param ].targetName );
    if ( !b )
      return;

    changed = b->GetText() != originalStringvalue;
    SetTypeParameter( *data, param, 0, 0, b->GetText(), false );
    break;
  }
  case Float:
  {
    CWBTextBox* b = FindChildByID<CWBTextBox>( typeParameters[ (int)param ].targetName );
    if ( !b )
      return;

    float result = 0;
    int value = b->GetText().Scan( "%f", &result );
    if ( value != 1 )
      return;

    changed = result != originalFloatValue;
    SetTypeParameter( *data, param, result, 0, "", false );
    break;
  }
  case Int:
  {
    CWBTextBox* b = FindChildByID<CWBTextBox>( typeParameters[ (int)param ].targetName );
    if ( !b )
      return;

    int result = 0;
    int value = b->GetText().Scan( "%d", &result );
    if ( value != 1 )
      return;

    changed = result != originalIntValue;
    SetTypeParameter( *data, param, 0, result, "", false );
    break;
  }
  case Color:
  {
    CWBTextBox* b = FindChildByID<CWBTextBox>( typeParameters[ (int)param ].targetName );
    if ( !b )
      return;

    int result = 0;
    int value = b->GetText().Scan( "%8X", &result );
    if ( value != 1 )
      return;

    changed = result != originalIntValue;
    SetTypeParameter( *data, param, 0, result, "", false );
    break;
  }
  case FloatNormalized:
  {
    CWBTextBox* b = FindChildByID<CWBTextBox>( typeParameters[ (int)param ].targetName );
    if ( !b )
      return;

    float result = 0;
    int value = b->GetText().Scan( "%f", &result );
    if ( value != 1 )
      return;

    result = max( 0, min( 1, result ) );

    changed = result != originalFloatValue;
    SetTypeParameter( *data, param, result, 0, "", false );
    break;
  }
  case DropDown:
  {
    CWBDropDown* b = FindChildByID<CWBDropDown>( typeParameters[ (int)param ].targetName );
    if ( !b )
      return;

    changed = originalIntValue != b->GetCursorPosition();
    SetTypeParameter( *data, param, 0, b->GetCursorPosition(), "", false );
    break;
  }
  }

  if ( editedCategory.Length() && changed )
  {
    auto cat = GetCategory( editedCategory );
    if ( cat )
    {
      bool boolValue{};
      int intValue{};
      float floatValue{};
      CString stringvalue;

      // force propagation
      SetTypeParameterSaved( *data, param, false );

      GetTypeParameter( cat->data, param, floatValue, intValue, stringvalue, boolValue );
      PropagateCategoryParameter( cat, param, boolValue, intValue, floatValue, stringvalue );
    }
  }

  if ( editedMarker != GUID{} )
  {
    auto trail = FindTrailByGUID( editedMarker );
    if ( trail && param == TypeParameters::TrailFile )
      trail->Reload();
  }

  if ( changed )
    SetTypeParameterSaved( *data, param, true );
}

void GW2MarkerEditor::ToggleTypeParameterSaved( TypeParameters param )
{
  MarkerTypeData* data = GetEditedTypeParameters();
  if ( !data )
    return;

  bool saved = !IsTypeParameterSaved( *data, param );
  SetTypeParameterSaved( *data, param, saved );

  if ( saved )
    return; // we just stored the previous value, no change needed

  // need to propagate parent's default value to current category
  if ( !saved && editedCategory.Length() )
  {
    auto cat = GetCategory( editedCategory );
    if ( cat )
    {
      auto catParent = cat->parent;
      if ( catParent )
      {
        bool boolValue{};
        int intValue{};
        float floatValue{};
        CString stringvalue;

        GetTypeParameter( catParent->data, param, floatValue, intValue, stringvalue, boolValue );
        PropagateCategoryParameter( cat, param, boolValue, intValue, floatValue, stringvalue );
      }
    }
  }

  // need to propagate parent's default value to current marker
  if ( !saved && editedMarker != GUID{} )
  {
    auto marker = FindMarkerByGUID( editedMarker );
    if ( marker && marker->category )
    {
      bool boolValue{};
      int intValue{};
      float floatValue{};
      CString stringvalue;

      GetTypeParameter( marker->category->data, param, floatValue, intValue, stringvalue, boolValue );
      SetTypeParameter( *data, param, floatValue, intValue, stringvalue, boolValue );
    }

    auto trail = FindTrailByGUID( editedMarker );
    if ( trail && trail->category )
    {
      bool boolValue{};
      int intValue{};
      float floatValue{};
      CString stringvalue;

      GetTypeParameter( trail->category->data, param, floatValue, intValue, stringvalue, boolValue );
      SetTypeParameter( *data, param, floatValue, intValue, stringvalue, boolValue );
    }
  }
}

bool GW2MarkerEditor::GetTypeParameterSaved( TypeParameters param )
{
  MarkerTypeData* data = GetEditedTypeParameters();
  if ( !data )
    return false;
  return IsTypeParameterSaved( *data, param );
}

void GW2MarkerEditor::GetTypeParameterValue( TypeParameters param, bool& boolValue, int& intValue, float& floatValue, CString& stringValue )
{
  MarkerTypeData* data = GetEditedTypeParameters();
  if ( !data )
    return;
  GetTypeParameter( *data, param, floatValue, intValue, stringValue, boolValue );
}

MarkerTypeData* GW2MarkerEditor::GetEditedTypeParameters()
{
  if ( editedCategory == "" && editedMarker == GUID{} )
    return nullptr;

  auto marker = FindMarkerByGUID( editedMarker );
  if ( marker )
    return &marker->typeData;

  auto trail = FindTrailByGUID( editedMarker );
  if ( trail )
    return &trail->typeData;

  auto category = GetCategory( editedCategory );
  if ( category )
    return &category->data;

  return nullptr;
}

void GW2MarkerEditor::HideEditorUI( bool fade )
{
  for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
  {
    auto* it = FindChildByID( typeParameters[ x ].targetName );
    if ( it )
      it = it->GetParent();
    if ( it )
      it->Hide( fade );
  }

  CWBLabel* label = FindChildByID<CWBLabel>( "label1" );
  if ( label )
    label->Hide( fade );
  label = FindChildByID<CWBLabel>( "label2" );
  if ( label )
    label->Hide( fade );
  label = FindChildByID<CWBLabel>( "label3" );
  if ( label )
    label->Hide( fade );
  label = FindChildByID<CWBLabel>( "label4" );
  if ( label )
    label->Hide( fade );
  label = FindChildByID<CWBLabel>( "label5" );
  if ( label )
    label->Hide( fade );
/*
  label = FindChildByID<CWBLabel>( "label6" );
  if ( label )
    label->Hide( fade );
*/

}

void GW2MarkerEditor::ShowMarkerUI( bool marker, bool trail )
{
  if ( !marker && !trail )
    return;

  for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
  {
    if ( !( ( typeParameters[ x ].markerData && marker ) || ( typeParameters[ x ].trailData && trail ) ) )
      continue;

    auto* it = FindChildByID( typeParameters[ x ].targetName );
    if ( it )
      it = it->GetParent();
    if ( it )
      it->Hide( false );
  }

  CWBLabel* label = FindChildByID<CWBLabel>( "label1" );
  if ( label )
    label->Hide( false );
  label = FindChildByID<CWBLabel>( "label2" );
  if ( label )
    label->Hide( false );
  label = FindChildByID<CWBLabel>( "label3" );
  if ( label )
    label->Hide( false );
  label = FindChildByID<CWBLabel>( "label4" );
  if ( label )
    label->Hide( false );
  label = FindChildByID<CWBLabel>( "label5" );
  if ( label )
    label->Hide( false );
}

GUID GW2MarkerEditor::GetMouseTrail( int& idx, CVector3& hitPoint, bool& indexPlusOne )
{
  CRect drawrect = App->GetRoot()->GetClientRect();

  POINT mousePos;
  mousePos.x = App->GetMousePos().x;
  mousePos.y = App->GetMousePos().y;

  if ( IsDebuggerPresent() )
    GetCursorPos( &mousePos );

  CVector4 mouse1 = CVector4( mousePos.x / (float)drawrect.Width() * 2 - 1, ( ( 1 - mousePos.y / (float)drawrect.Height() ) * 2 - 1 ), -1, 1 );
  CVector4 mouse2 = CVector4( mouse1.x, mouse1.y, 1, 1 );

  ConstBuffer bufferData;
  bufferData.cam.SetLookAtLH( mumbleLink.camPosition, mumbleLink.camPosition + mumbleLink.camDir, CVector3( 0, 1, 0 ) );
  bufferData.persp.SetPerspectiveFovLH( mumbleLink.fov, drawrect.Width() / (TF32)drawrect.Height(), 0.05f, 1000.0f );

  CMatrix4x4 matrix;
  matrix.SetIdentity(); // no transform for trails, yay!

  CMatrix4x4 invMatrix = ( matrix * bufferData.cam * bufferData.persp ).Inverted();
  CVector4 localMouse1 = mouse1 * invMatrix;
  CVector4 localMouse2 = mouse2 * invMatrix;
  localMouse1 /= localMouse1.w;
  localMouse2 /= localMouse2.w;

  CLine mouseLine( localMouse1, localMouse2 - localMouse1 );

  auto trails = trailSet.find( mumbleLink.mapID );
  if ( trails == trailSet.end() )
    return GUID{};

  float minZ = 1000000;
  idx = 0;
  hitPoint = CVector3();

  GUID result = GUID{};

  for ( int x = 0; x < trails->second.NumItems(); x++ )
  {
    float currZ = minZ;
    int currIdx = 0;
    CVector3 currHitPoint;
    bool currIndexPlusOne = false;
    if ( trails->second.GetByIndex( x )->HitTest( mouseLine, currZ, currIdx, currHitPoint, currIndexPlusOne ) && currZ < minZ )
    {
      minZ = currZ;
      result = trails->second.GetByIndex( x )->guid;
      idx = currIdx;
      hitPoint = currHitPoint;
      indexPlusOne = currIndexPlusOne;
    }
  }

  return result;
}

TBOOL GW2MarkerEditor::MessageProc( CWBMessage& message )
{
  switch ( message.GetMessage() )
  {
  case WBM_SELECTIONCHANGE:
  {
    CWBDropDown* b = (CWBDropDown*)App->FindItemByGuid( message.GetTarget(), "dropdown" );
    if ( !b )
      break;

    for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
    {
      if ( b->GetID() == typeParameters[ x ].targetName )
      {
        UpdateTypeParameterValue( (TypeParameters)x );
        UpdateEditorContent();
        return true;
      }
    }
  }
  break;

  case WBM_FOCUSLOST:
  {
    CWBTextBox* t = (CWBTextBox*)App->FindItemByGuid( message.GetTarget(), "textbox" );
    if ( t )
    {
      for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
      {
        if ( t->GetID() == typeParameters[ x ].targetName )
        {
          UpdateTypeParameterValue( (TypeParameters)x );
          UpdateEditorContent();
          break;
        }
      }
      break;
    }
    break;
  }
  case WBM_COMMAND:
  {
    if ( hidden )
      break;

    CWBTextBox* t = (CWBTextBox*)App->FindItemByGuid( message.GetTarget(), "textbox" );
    if ( t )
    {
      for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
      {
        if ( t->GetID() == typeParameters[ x ].targetName )
        {
          UpdateTypeParameterValue( (TypeParameters)x );
          UpdateEditorContent();
          return true;
        }
      }
      break;
    }

    CWBButton* b = (CWBButton*)App->FindItemByGuid( message.GetTarget(), _T( "button" ) );
    if ( !b )
      break;

    for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
    {
      if ( b->GetID() == typeParameters[ x ].enableCheckBoxName )
      {
        ToggleTypeParameterSaved( (TypeParameters)x );
        b->Push( GetTypeParameterSaved( (TypeParameters)x ) );
        UpdateEditorContent();
        return true;
      }
    }

    for ( int x = 0; x < (int)TypeParameters::MAX; x++ )
    {
      if ( b->GetID() == typeParameters[ x ].targetName )
      {
        b->Push( !b->IsPushed() );
        UpdateTypeParameterValue( (TypeParameters)x );
        bool boolValue{};
        int intValue{};
        float floatValue{};
        CString stringValue;
        GetTypeParameterValue( (TypeParameters)x, boolValue, intValue, floatValue, stringValue );
        b->Push( boolValue );
        UpdateEditorContent();
        return true;
      }
    }


    changeDefault = false;
    selectingEditedCategory = false;

    if ( b->GetID() == _T( "deletemarker" ) && editedMarker != GUID{} )
    {
      auto marker = FindMarkerByGUID( editedMarker );
      auto trail = FindTrailByGUID( editedMarker );

      if ( marker )
        DeletePOI( editedMarker );

      if ( trail )
        DeleteTrail( editedMarker );

      SetEditedGUID( GUID{} );
    }

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
      //b->SetText( b->IsPushed() ? "Stop Recording" : "Start New Trail" );
      if ( trails )
        trails->StartStopTrailRecording( b->IsPushed() );
    }

    if ( b->GetID() == _T( "pausetrail" ) && trails )
      trails->PauseTrail( !b->IsPushed() );

    if ( b->GetID() == _T( "startnewsection" ) && trails )
      trails->PauseTrail( false, true );

    if ( b->GetID() == _T( "deletelastsegment" ) && trails )
    {
      auto trail = FindTrailByGUID( editedMarker );
      if ( trail )
        trail->DeleteVertex( selectedVertexIndex );
      else
        trails->DeleteLastTrailSegment();
    }

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

    if ( b->GetID() == _T( "default5" ) )
      defaultCatBeingSet = 4;

    if ( b->GetID() == _T( "prevsegment" ) )
    {
      auto trail = FindTrailByGUID( editedMarker );
      if ( trail )
      {
        selectedVertexIndex--;
        if ( selectedVertexIndex < 0 )
          selectedVertexIndex = trail->GetVertexCount() - 1;
      }
    }

    if ( b->GetID() == _T( "nextsegment" ) )
    {
      auto trail = FindTrailByGUID( editedMarker );
      if ( trail )
        selectedVertexIndex = ( selectedVertexIndex + 1 ) % trail->GetVertexCount();
    }

    if ( b->GetID() == _T( "newtrail" ) )
    {
      AddTrail( App );
    }
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
      auto& trails = GetMapTrails();

      if ( editedMarker != GUID{} && FindMarkerByGUID( editedMarker ) )
        POIs[ editedMarker ].SetCategory( App, categoryList[ message.Data ] );

      if ( editedMarker != GUID{} && FindTrailByGUID( editedMarker ) )
        trails[ editedMarker ]->SetCategory( App, categoryList[ message.Data ] );

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
