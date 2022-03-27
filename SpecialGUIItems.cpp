#include "SpecialGUIItems.h"

ClickThroughButton::ClickThroughButton( CWBItem* Parent, const CRect& Pos, const TCHAR* txt /*= _T( "" ) */ ) : CWBButton( Parent, Pos, txt )
{

}

ClickThroughButton::~ClickThroughButton()
{

}

TBOOL ClickThroughButton::Initialize( CWBItem* Parent, const CRect& Position, const TCHAR* txt /*= _T( "" ) */ )
{
  return CWBButton::Initialize( Parent, Position, txt );
}

CWBItem* ClickThroughButton::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  ClickThroughButton* button = new ClickThroughButton( Root, Pos );
  if ( node.HasAttribute( _T( "text" ) ) )
    button->SetText( node.GetAttribute( _T( "text" ) ) );

  if ( node.HasAttribute( _T( "hidden" ) ) )
  {
    TS32 x = 0;
    node.GetAttributeAsInteger( _T( "hidden" ), &x );
    button->Hide( x );
  }

  return button;
}
