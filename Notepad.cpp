#include "Notepad.h"
#include "MumbleLink.h"
#include "gw2tactical.h"
#include "OverlayConfig.h"

TBOOL GW2Notepad::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}

GW2Notepad::GW2Notepad( CWBItem *Parent, CRect Position ) : CWBItem( Parent, Position )
{
  App->GenerateGUITemplate( this, "gw2pois", "notepad" );

  SetID( "notepad" );

  CStreamReaderMemory nptext;
  if ( !nptext.Open( "notepad.txt" ) )
    return;

  CWBTextBox *tb = (CWBTextBox*)FindChildByID( "notepad", "textbox" );
  if ( !tb )
    return;

  tb->SetForcedMouseTransparency( true );
  tb->SetText( CString( (TS8*)nptext.GetData(), (TS32)( nptext.GetLength() ) ) );
  tb->SetCursorPos( 0, false );
}

GW2Notepad::~GW2Notepad()
{
  CWBTextBox *tb = (CWBTextBox*)FindChildByID( "notepad", "textbox" );
  if ( !tb )
    return;

  CStreamWriterFile nptext;
  if ( !nptext.Open( "notepad.txt" ) )
    return;

  nptext.Write( tb->GetText().GetPointer(), tb->GetText().Length() );
}

CWBItem * GW2Notepad::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  return new GW2Notepad( Root, Pos );
}

void GW2Notepad::StartEdit()
{
  CWBTextBox *tb = (CWBTextBox*)FindChildByID( "notepad", "textbox" );
  if ( !tb )
    return;

  canSetFocus = true;
  tb->SetFocus();
  tb->SetCursorPos( tb->GetText().Length(), false );
}

void GW2Notepad::OnDraw( CWBDrawAPI *API )
{
}

TBOOL GW2Notepad::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_FOCUSGAINED:
  {
    CWBItem *tb = (CWBItem*)FindChildByID( "notepad", "textbox" );
    if ( tb->GetGuid() == Message.GetTarget() )
      tb->SetForcedMouseTransparency( false );
  }
  break;
  case WBM_FOCUSLOST:
  {
    CWBItem *tb = (CWBItem*)FindChildByID( "notepad", "textbox" );
    if ( tb->GetGuid() == Message.GetTarget() )
      tb->SetForcedMouseTransparency( true );
  }
  break;
  }

  return CWBItem::MessageProc( Message );
}
