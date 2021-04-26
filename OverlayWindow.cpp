#include "OverlayWindow.h"
#include "OverlayConfig.h"

TBOOL OverlayWindow::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  if ( GetConfigValue( "EditMode" ) ) return false;
  return true;
}

OverlayWindow::OverlayWindow( CWBItem *Parent, CRect Position ) : CWBWindow( Parent, Position )
{

}

OverlayWindow::~OverlayWindow()
{
  SetWindowPosition( GetID().GetPointer(), GetPosition() );
}

CWBItem * OverlayWindow::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new OverlayWindow( Root, Pos );
}

void OverlayWindow::OnDraw( CWBDrawAPI *API )
{
  if ( !GetConfigValue( "EditMode" ) ) return;
  CWBWindow::OnDraw( API );
}

TBOOL OverlayWindow::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_LEFTBUTTONDOWN:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseItem() == this )
    {
      SetCapture();
      SavePosition();

      if ( Style & WB_WINDOW_CLOSEABLE )
      {
        if ( GetElementPos( WB_WINELEMENT_CLOSE ).Contains( ScreenToClient( Message.GetPosition() ) ) )
        {
          DragMode = WB_DRAGMODE_CLOSEBUTTON;
          return true;
        }
      }

      if ( Style & WB_WINDOW_RESIZABLE )
      {
        DragMode = GetBorderSelectionArea( Message.GetPosition() );
        if ( DragMode & WB_DRAGMASK ) return true;
      }

      if ( Style & WB_WINDOW_MOVEABLE )
      {
        DragMode = WB_DRAGMODE_MOVE;
        return true;
      }

      DragMode = 0;
    }
    break;
  case WBM_REPOSITION:
    SetWindowPosition( GetID().GetPointer(), GetPosition() );
    break;
  case WBM_CLOSE:
    SetWindowOpenState( GetID().GetPointer(), false );
    break;
  }

  return CWBWindow::MessageProc( Message );
}
