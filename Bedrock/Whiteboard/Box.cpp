#include "BasePCH.h"
#include "Box.h"

void CWBBox::AddChild( CWBItem *Item )
{
  CWBItem::AddChild( Item );
  RearrangeChildren();
}

CWBBox::CWBBox() : CWBItem()
{
}

CWBBox::CWBBox( CWBItem *Parent, const CRect &Pos ) : CWBItem()
{
  Initialize( Parent, Pos );
}

CWBBox::~CWBBox()
{

}

TBOOL CWBBox::Initialize( CWBItem *Parent, const CRect &Position )
{
  Arrangement = WB_ARRANGE_NONE;
  Spacing = 0;
  AlignmentX = WB_ALIGN_LEFT;
  AlignmentY = WB_ALIGN_TOP;
  SizingX = WB_SIZING_KEEP;
  SizingY = WB_SIZING_KEEP;

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;
  return true;
}

TBOOL CWBBox::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_CLIENTAREACHANGED:
    if ( Message.GetTarget() == GetGuid() )
    {
      //LOG_WARN("Box %d clientarea changed", GetGuid());
      RearrangeChildren();
    }
    break;
    //case WBM_CONTENTOFFSETCHANGE:
    //	if (CWBItem::MessageProc(Message))
    //	{
    //		//LOG_DBG("Box %d contentoffsetchange: %d %d", GetGuid(), Message.GetPosition().x, Message.GetPosition().y);
    //		return true;
    //	}
    //	break;

  case WBM_MOUSEWHEEL:
    if ( App->GetCtrlState() && IsVScrollbarEnabled() )
    {
      CPoint content = GetContentSize();
      CRect client = GetClientRect();
      if ( content.y < client.Height() )
        break;

      CRect BRect = CRect( 0, 0, 0, 0 );

      for ( TU32 x = 0; x < NumChildren(); x++ )
        if ( !GetChild( x )->IsHidden() )
          BRect = BRect & GetChild( x )->GetPosition();

      SetVScrollbarPos( max( BRect.y1, min( BRect.y2 - client.Height(), GetVScrollbarPos() - (int)( Message.Data*client.Height() / 5.0f ) ) ), false );
      return true;
    }
    break;

  case WBM_HSCROLL:
  {
    TBOOL Result = CWBItem::MessageProc( Message );
    if ( Message.GetTarget() == GetGuid() )
    {
      ChangeContentOffsetX( -Message.Data );
      return true;
    }
    return Result;
  }
  break;
  case WBM_VSCROLL:
  {
    TBOOL Result = CWBItem::MessageProc( Message );
    if ( Message.GetTarget() == GetGuid() )
    {
      //LOG_DBG("Box %d Vscroll: %d", GetGuid(), Message.Data);
      ChangeContentOffsetY( -Message.Data );
      return true;
    }
    return Result;
  }
  break;

  case WBM_REPOSITION:
  {
    if ( Message.GetTarget() == GetGuid() )
    {
      //LOG_DBG("Box %d Resize: %d %d %d %d", GetGuid(), Message.Rectangle.x1, Message.Rectangle.y1, Message.Rectangle.x2, Message.Rectangle.y2);
      CWBItem::MessageProc( Message );
      RearrangeChildren();
      return true;
    }

    CWBItem *i = App->FindItemByGuid( Message.GetTarget() );

    if ( i && i->GetParent() == this )
    {
      //LOG_DBG("Box %d Child Resize: %d %d %d %d", GetGuid(), Message.Rectangle.x1, Message.Rectangle.y1, Message.Rectangle.x2, Message.Rectangle.y2);
      if ( Arrangement == WB_ARRANGE_NONE ) break; //do nothing here

      CRect r = Message.Rectangle;
      CRect o = i->GetPosition();

      if ( SizingX == WB_SIZING_FILL )
      {
        r.x1 = o.x1;
        r.x2 = o.x2;
      }

      if ( SizingY == WB_SIZING_FILL )
      {
        r.y1 = o.y1;
        r.y2 = o.y2;
      }

      Message.Rectangle = r;
      Message.Resized = true;
      Message.Moved = r.TopLeft() == o.TopLeft();

      i->MessageProc( Message ); //do the resize as the item sees fit

      RearrangeChildren();
      return true;
    }
  }
  break;
  }

  return CWBItem::MessageProc( Message );
}

void CWBBox::RearrangeHorizontal()
{
  CRect ClientRect = GetClientRect();
  TS32 pos = 0;
  TF32 Excess = 0;

  TS32 NumDynamicChildren = 0;
  TS32 NonDynamicWidth = 0;
  TS32 LastDynamic = 0;

  for ( TU32 x = 0; x < NumChildren(); x++ )
  {
    if ( !GetChild( x )->IsWidthSet() )
    {
      NumDynamicChildren++;
      LastDynamic = x;
    }
    else NonDynamicWidth += GetChild( x )->GetCalculatedWidth( ClientRect.Size() );
  }

  TS32 width = ( NumChildren() - 1 )*Spacing;
  TS32 DynamicWidth = GetClientRect().Width() - NonDynamicWidth - width;
  TF32 itemsizes = DynamicWidth / (TF32)NumDynamicChildren;

  for ( TU32 x = 0; x < NumChildren(); x++ ) width += GetChild( x )->GetPosition().Width();

  if ( AlignmentX == WB_ALIGN_RIGHT )
    pos = GetClientRect().Width() - width;
  if ( AlignmentX == WB_ALIGN_CENTER )
    pos = ( GetClientRect().Width() - width ) / 2;

  for ( TU32 x = 0; x < NumChildren(); x++ )
  {
    CRect ChildPosition = GetChild( x )->GetPosition();

    if ( SizingX == WB_SIZING_FILL )
    {
      if ( !GetChild( x )->IsWidthSet() )
      {
        ChildPosition.x1 = pos;
        Excess += itemsizes;
        ChildPosition.x2 = ChildPosition.x1 + (TS32)Excess;
        Excess -= (TS32)Excess;
        if ( x == LastDynamic && Excess >= 0.5 ) ChildPosition.x2++; //fucking float inaccuracies...
      }
      else
      {
        ChildPosition.x1 = pos;
        ChildPosition.x2 = ChildPosition.x1 + GetChild( x )->GetCalculatedWidth( ClientRect.Size() );
      }
    }
    else
    {
      TS32 posw = ChildPosition.Width();
      ChildPosition.x1 = pos;
      ChildPosition.x2 = pos + posw;
    }

    if ( SizingY == WB_SIZING_FILL )
    {
      ChildPosition.y1 = 0;
      ChildPosition.y2 = GetClientRect().Height();
    }

    TS32 off = 0;
    if ( AlignmentY == WB_ALIGN_BOTTOM )
      off = ClientRect.Height() - ChildPosition.Height();
    if ( AlignmentY == WB_ALIGN_CENTER )
      off = ( ClientRect.Height() - ChildPosition.Height() ) / 2;

    CRect np = CRect( pos, off, pos + ChildPosition.Width(), off + ChildPosition.Height() );

    if ( GetChild( x )->GetPosition() != np )
    {
      CWBMessage m;
      GetChild( x )->BuildPositionMessage( np, m );
      GetChild( x )->MessageProc( m );
    }

    pos = ChildPosition.x2 + Spacing;
  }
}

void CWBBox::RearrangeVertical()
{
  CRect ClientRect = GetClientRect();
  TS32 pos = 0;
  TF32 Excess = 0;

  TS32 NumDynamicChildren = 0;
  TS32 NonDynamicHeight = 0;
  TS32 LastDynamic = 0;

  for ( TU32 x = 0; x < NumChildren(); x++ )
  {
    if ( !GetChild( x )->IsHeightSet() )
    {
      NumDynamicChildren++;
      LastDynamic = x;
    }
    else NonDynamicHeight += GetChild( x )->GetCalculatedHeight( ClientRect.Size() );
  }

  TS32 height = ( NumChildren() - 1 )*Spacing;
  TS32 DynamicHeight = GetClientRect().Height() - NonDynamicHeight - height;
  TF32 itemsizes = DynamicHeight / (TF32)NumDynamicChildren;

  for ( TU32 x = 0; x < NumChildren(); x++ ) height += GetChild( x )->GetPosition().Height();

  if ( AlignmentY == WB_ALIGN_BOTTOM )
    pos = GetClientRect().Height() - height;
  if ( AlignmentY == WB_ALIGN_CENTER )
    pos = ( GetClientRect().Height() - height ) / 2;

  for ( TU32 x = 0; x < NumChildren(); x++ )
  {
    CRect ChildPosition = GetChild( x )->GetPosition();

    if ( SizingX == WB_SIZING_FILL )
    {
      ChildPosition.x1 = 0;
      ChildPosition.x2 = GetClientRect().Width();
    }

    if ( SizingY == WB_SIZING_FILL )
    {
      if ( !GetChild( x )->IsHeightSet() )
      {
        ChildPosition.y1 = pos;
        Excess += itemsizes;
        ChildPosition.y2 = ChildPosition.y1 + (TS32)Excess;
        Excess -= (TS32)Excess;
        if ( x == LastDynamic && Excess >= 0.5 ) ChildPosition.y2++; //fucking float inaccuracies...
      }
      else
      {
        ChildPosition.y1 = pos;
        ChildPosition.y2 = ChildPosition.y1 + GetChild( x )->GetCalculatedHeight( ClientRect.Size() );
      }
    }
    else
    {
      TS32 posh = ChildPosition.Height();
      ChildPosition.y1 = pos;
      ChildPosition.y2 = pos + posh;
    }

    TS32 off = 0;
    if ( AlignmentX == WB_ALIGN_BOTTOM )
      off = ClientRect.Width() - ChildPosition.Width();
    if ( AlignmentX == WB_ALIGN_CENTER )
      off = ( ClientRect.Width() - ChildPosition.Width() ) / 2;

    CRect np = CRect( off, pos, off + ChildPosition.Width(), pos + ChildPosition.Height() );

    if ( GetChild( x )->GetPosition() != np )
    {
      CWBMessage m;
      GetChild( x )->BuildPositionMessage( np, m );
      GetChild( x )->MessageProc( m );
    }

    pos = ChildPosition.y2 + Spacing;
  }
}

void CWBBox::RearrangeChildren()
{
  //LOG_DBG("Box %d Rearrangechildren", GetGuid());

  switch ( Arrangement )
  {
  case WB_ARRANGE_NONE:
    break;
  case WB_ARRANGE_HORIZONTAL:
    RearrangeHorizontal();
    break;
  case WB_ARRANGE_VERTICAL:
    RearrangeVertical();
    break;
  }

  UpdateScrollbarData();
}

void CWBBox::OnDraw( CWBDrawAPI *API )
{
  DrawBackground( API );
  DrawBorder( API );
}

void CWBBox::SetArrangement( WBBOXARRANGEMENT a )
{
  Arrangement = a;
  RearrangeChildren();
}

WBBOXARRANGEMENT CWBBox::GetArrangement()
{
  return Arrangement;
}

void CWBBox::SetSpacing( TS32 s )
{
  Spacing = s;
  RearrangeChildren();
}

TBOOL CWBBox::ApplyStyle( CString & prop, CString & value, CStringArray & pseudo )
{
  if ( CWBItem::ApplyStyle( prop, value, pseudo ) ) return true;

  if ( prop == _T( "child-layout" ) )
  {
    if ( value == _T( "none" ) ) { SetArrangement( WB_ARRANGE_NONE ); return true; }
    if ( value == _T( "horizontal" ) ) { SetArrangement( WB_ARRANGE_HORIZONTAL ); return true; }
    if ( value == _T( "vertical" ) ) { SetArrangement( WB_ARRANGE_VERTICAL ); return true; }
    LOG_WARN( "[gui] %s has invalid parameter: '%s' (none/horizontal/vertical required)", prop.GetPointer(), value.GetPointer() );
    return false;
  }

  if ( prop == _T( "child-spacing" ) )
  {
    TU32 dw = 0;
    value.Scan( _T( "%d" ), &dw );
    SetSpacing( dw );
    return true;
  }

  if ( prop == _T( "child-align-x" ) )
  {
    if ( value == _T( "left" ) ) { SetAlignment( WB_HORIZONTAL, WB_ALIGN_LEFT ); return true; }
    if ( value == _T( "right" ) ) { SetAlignment( WB_HORIZONTAL, WB_ALIGN_RIGHT ); return true; }
    if ( value == _T( "center" ) ) { SetAlignment( WB_HORIZONTAL, WB_ALIGN_CENTER ); return true; }

    LOG_WARN( "[gui] %s has invalid parameter: '%s' (left/center/right required)", prop.GetPointer(), value.GetPointer() );

    return true;
  }

  if ( prop == _T( "child-align-y" ) )
  {
    if ( value == _T( "top" ) ) { SetAlignment( WB_VERTICAL, WB_ALIGN_TOP ); return true; }
    if ( value == _T( "bottom" ) ) { SetAlignment( WB_VERTICAL, WB_ALIGN_BOTTOM ); return true; }
    if ( value == _T( "center" ) ) { SetAlignment( WB_VERTICAL, WB_ALIGN_CENTER ); return true; }

    LOG_WARN( "[gui] %s has invalid parameter: '%s' (top/center/bottom required)", prop.GetPointer(), value.GetPointer() );

    return true;
  }

  if ( prop == _T( "child-fill-x" ) )
  {
    if ( value == _T( "false" ) ) { SetSizing( WB_HORIZONTAL, WB_SIZING_KEEP ); return true; }
    if ( value == _T( "true" ) ) { SetSizing( WB_HORIZONTAL, WB_SIZING_FILL ); return true; }

    LOG_WARN( "[gui] %s has invalid parameter: '%s' (true/false required)", prop.GetPointer(), value.GetPointer() );

    return true;
  }

  if ( prop == _T( "child-fill-y" ) )
  {
    if ( value == _T( "false" ) ) { SetSizing( WB_VERTICAL, WB_SIZING_KEEP ); return true; }
    if ( value == _T( "true" ) ) { SetSizing( WB_VERTICAL, WB_SIZING_FILL ); return true; }

    LOG_WARN( "[gui] %s has invalid parameter: '%s' (true/false required)", prop.GetPointer(), value.GetPointer() );

    return true;
  }

  return false;
}

TBOOL CWBBox::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  return ClickThrough;
}

void CWBBox::SetAlignment( WBBOXAXIS axis, WBALIGNMENT align )
{
  if ( axis == WB_HORIZONTAL ) AlignmentX = align;
  if ( axis == WB_VERTICAL ) AlignmentY = align;
  RearrangeChildren();
}

void CWBBox::SetSizing( WBBOXAXIS axis, WBBOXSIZING siz )
{
  if ( axis == WB_HORIZONTAL ) SizingX = siz;
  if ( axis == WB_VERTICAL ) SizingY = siz;
  RearrangeChildren();
}

CWBItem * CWBBox::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  CWBBox * box = new CWBBox( Root, Pos );

  if ( node.HasAttribute( _T( "clickthrough" ) ) )
  {
    TS32 b = 0;
    node.GetAttributeAsInteger( _T( "clickthrough" ), &b );
    box->ClickThrough = b != 0;
  }

  return box;
}

void CWBBox::UpdateScrollbarData()
{
  if ( !ScrollbarsEnabled() ) return;

  CRect BRect = CRect( 0, 0, 0, 0 );

  for ( TU32 x = 0; x < NumChildren(); x++ )
    if ( !GetChild( x )->IsHidden() )
      BRect = BRect & GetChild( x )->GetPosition();

  //LOG_DBG("Box %d UpdateScrollData %d %d %d %d", GetGuid(), BRect.x1, BRect.y1, BRect.x2, BRect.y2);

  SetHScrollbarParameters( BRect.x1, BRect.x2, GetClientRect().Width() );
  SetVScrollbarParameters( BRect.y1, BRect.y2, GetClientRect().Height() );

  if ( GetClientRect().Width() >= BRect.Width() && GetHScrollbarPos() != BRect.x1 )
  {
    //LOG_DBG("Box %d HScroll Fits", GetGuid());
    SetHScrollbarPos( BRect.x1, true );
  }
  if ( GetClientRect().Height() >= BRect.Height() && GetVScrollbarPos() != BRect.y1 )
  {
    //LOG_DBG("Box %d VScroll Fits", GetGuid());
    SetVScrollbarPos( BRect.y1, true );
  }
}

