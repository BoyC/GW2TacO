#include "MarkerEditor.h"
#include "MumbleLink.h"
#include "MarkerPack.h"
#include "OverlayConfig.h"
#include "TrailLogger.h"

TBOOL GW2MarkerEditor::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

GW2MarkerEditor::GW2MarkerEditor( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{
  App->GenerateGUITemplate( this, "gw2pois", "markereditor" );
}

GW2MarkerEditor::~GW2MarkerEditor()
{

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
