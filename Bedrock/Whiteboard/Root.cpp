#include "BasePCH.h"
#include "Root.h"
#include "Application.h"
#include "Message.h"

TBOOL CWBRoot::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_REPOSITION:
  {
    if ( Message.GetTarget() == GetGuid() || Message.GetTarget() == 0 )
    {
      if ( !Message.Resized ) return true; //ignore movement

      CWBItem::MessageProc( Message );
      for ( TU32 x = 0; x < NumChildren(); x++ )
        App->SendMessage( CWBMessage( App, WBM_ROOTRESIZE, GetChild( x )->GetGuid(), Message.Rectangle.Width(), Message.Rectangle.Height() ) );
      return true;
    }
  }
  }

  return CWBItem::MessageProc( Message );
}


CWBRoot::CWBRoot() : CWBItem()
{
  App = NULL;
}

CWBRoot::CWBRoot( CWBItem *Parent, const CRect &Pos ) : CWBItem()
{
  Initialize( Parent, Pos );
}

CWBRoot::~CWBRoot()
{

}

void CWBRoot::SetApplication( CWBApplication *Application )
{
  App = Application;
}

TBOOL CWBRoot::Initialize( CWBItem *Parent, const CRect &Position )
{
  if ( !CWBItem::Initialize( Parent, Position ) ) return false;
  return true;
}

void CWBRoot::OnDraw( CWBDrawAPI *API )
{

}