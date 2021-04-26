#include "BasePCH.h"
#include "Application.h"
#include "GuiItem.h"

static WBGUID WB_GUID_COUNTER = 1337;

//////////////////////////////////////////////////////////////////////////
// Display Descriptor

CWBDisplayState::CWBDisplayState()
{
  memset( Visuals, 0, sizeof( TS32 )*WB_ITEM_COUNT );
  memset( VisualSet, 0, sizeof( TBOOL )*WB_ITEM_COUNT );
}

CWBDisplayState::~CWBDisplayState()
{
}

CColor CWBDisplayState::GetColor( WBITEMVISUALCOMPONENT v )
{
  return Visuals[ v ];
}

TBOOL CWBDisplayState::IsSet( WBITEMVISUALCOMPONENT v )
{
  return VisualSet[ v ];//.HasKey(v);
}

void CWBDisplayState::SetValue( WBITEMVISUALCOMPONENT v, TS32 value )
{
  Visuals[ v ] = value;
  VisualSet[ v ] = true;
}

WBSKINELEMENTID CWBDisplayState::GetSkin( WBITEMVISUALCOMPONENT v )
{
  return Visuals[ v ];
}

TS32 CWBDisplayState::GetValue( WBITEMVISUALCOMPONENT v )
{
  return Visuals[ v ];
}

CWBDisplayProperties::CWBDisplayProperties()
{

}

CWBDisplayProperties::~CWBDisplayProperties()
{

}

WBSKINELEMENTID CWBDisplayProperties::GetSkin( WBITEMSTATE s, WBITEMVISUALCOMPONENT v )
{
  //if (States.HasKey(s))
  {
    if ( States[ s ].IsSet( v ) )
      return States[ s ].GetSkin( v );
  }
  //if (States.HasKey(WB_STATE_NORMAL))
  {
    if ( States[ WB_STATE_NORMAL ].IsSet( v ) )
      return States[ WB_STATE_NORMAL ].GetSkin( v );
  }

  return 0xffffffff;
}

CColor CWBDisplayProperties::GetColor( WBITEMSTATE s, WBITEMVISUALCOMPONENT v )
{
  //if (States.HasKey(s))
  {
    if ( States[ s ].IsSet( v ) )
      return States[ s ].GetColor( v );
  }
  //if (States.HasKey(WB_STATE_NORMAL))
  {
    if ( States[ WB_STATE_NORMAL ].IsSet( v ) )
      return States[ WB_STATE_NORMAL ].GetColor( v );
  }

  switch ( v )
  {
  case WB_ITEM_BACKGROUNDCOLOR: return CColor::FromARGB( 0 );// CColor::FromARGB(0xff1e1e1e);
  case WB_ITEM_FOREGROUNDCOLOR: return CColor::FromARGB( 0 );// CColor::FromARGB(0xff1e1e1e);
  case WB_ITEM_BORDERCOLOR: return CColor::FromARGB( 0xff434346 );
  case WB_ITEM_FONTCOLOR: return 0xffffffff;
  }

  return 0xffffffff;
}

void CWBDisplayProperties::SetValue( WBITEMSTATE s, WBITEMVISUALCOMPONENT v, TS32 value )
{
  States[ s ].SetValue( v, value );
}

TS32 CWBDisplayProperties::GetValue( WBITEMSTATE s, WBITEMVISUALCOMPONENT v )
{
  if ( States[ s ].IsSet( v ) )
    return States[ s ].GetValue( v );
  return -1;
}

CWBCSSPropertyBatch::CWBCSSPropertyBatch()
{
  BorderSizes = CRect( 0, 0, 0, 0 );
  TextAlignX = WBTA_CENTERX;
  TextAlignY = WBTA_CENTERY;
}

CWBFont * CWBCSSPropertyBatch::GetFont( CWBApplication *App, WBITEMSTATE State )
{
  if ( Fonts.HasKey( State ) ) return App->GetFont( Fonts[ State ] );
  if ( Fonts.HasKey( WB_STATE_NORMAL ) ) return App->GetFont( Fonts[ WB_STATE_NORMAL ] );
  return App->GetDefaultFont();
}

TBOOL CWBCSSPropertyBatch::ApplyStyle( CWBItem *Owner, CString & prop, CString & value, CStringArray &pseudo )
{
  if ( Owner->InterpretPositionString( *this, prop, value, pseudo ) ) return true;
  if ( Owner->InterpretDisplayString( *this, prop, value, pseudo ) ) return true;
  return false;
}

//////////////////////////////////////////////////////////////////////////
// CWBItem

void CWBItem::UpdateScreenRect()
{
  CRect sr = ScreenRect;

  if ( Parent )
    ScreenRect = Position + Parent->ClientToScreen( CPoint( 0, 0 ) ) + Parent->ContentOffset;
  else
    ScreenRect = Position;

  if ( sr != ScreenRect )
    for ( TS32 x = 0; x < Children.NumItems(); x++ )
      Children[ x ]->UpdateScreenRect();
}

void CWBItem::HandleHScrollbarClick( WBSCROLLDRAGMODE m )
{
  switch ( m )
  {
  case WB_SCROLLDRAG_BUTTON1:
    SetHScrollbarPos( GetHScrollbarPos() - 1, true );
    break;
  case WB_SCROLLDRAG_BUTTON2:
    SetHScrollbarPos( GetHScrollbarPos() + 1, true );
    break;
  case WB_SCROLLDRAG_UP:
    SetHScrollbarPos( GetHScrollbarPos() - HScrollbar.ViewSize, true );
    break;
  case WB_SCROLLDRAG_DOWN:
    SetHScrollbarPos( GetHScrollbarPos() + HScrollbar.ViewSize, true );
    break;
  }
}

void CWBItem::HandleVScrollbarClick( WBSCROLLDRAGMODE m )
{
  switch ( m )
  {
  case WB_SCROLLDRAG_BUTTON1:
    SetVScrollbarPos( GetVScrollbarPos() - 1, true );
    break;
  case WB_SCROLLDRAG_BUTTON2:
    SetVScrollbarPos( GetVScrollbarPos() + 1, true );
    break;
  case WB_SCROLLDRAG_UP:
    SetVScrollbarPos( GetVScrollbarPos() - VScrollbar.ViewSize, true );
    break;
  case WB_SCROLLDRAG_DOWN:
    SetVScrollbarPos( GetVScrollbarPos() + VScrollbar.ViewSize, true );
    break;
  }
}


TBOOL CWBItem::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_NONE:
    LOG( LOG_ERROR, _T( "[gui] Message Type 0 Encountered. Message Target is %d" ), Message.GetTarget() );
    return true;

  case WBM_LEFTBUTTONDOWN:
    if ( App->GetMouseItem() == this ) //handle scrollbars
    {
      CRect b1, up, th, dn, b2;
      CPoint mp = Message.GetPosition(); //mouse pos
      if ( GetHScrollbarRectangles( b1, up, th, dn, b2 ) )
      {
        if ( ClientToScreen( b1 ).Contains( mp ) ) HScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON1;
        if ( ClientToScreen( up ).Contains( mp ) ) HScrollbar.Dragmode = WB_SCROLLDRAG_UP;
        if ( ClientToScreen( th ).Contains( mp ) ) HScrollbar.Dragmode = WB_SCROLLDRAG_THUMB;
        if ( ClientToScreen( dn ).Contains( mp ) ) HScrollbar.Dragmode = WB_SCROLLDRAG_DOWN;
        if ( ClientToScreen( b2 ).Contains( mp ) ) HScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON2;
        if ( HScrollbar.Dragmode != WB_SCROLLDRAG_NONE )
        {
          HScrollbar.DragStartPosition = HScrollbar.ScrollPos;
          HandleHScrollbarClick( HScrollbar.Dragmode );
          App->SetCapture( this );
          return true;
        }
      }
      if ( GetVScrollbarRectangles( b1, up, th, dn, b2 ) )
      {
        if ( ClientToScreen( b1 ).Contains( mp ) ) VScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON1;
        if ( ClientToScreen( up ).Contains( mp ) ) VScrollbar.Dragmode = WB_SCROLLDRAG_UP;
        if ( ClientToScreen( th ).Contains( mp ) ) VScrollbar.Dragmode = WB_SCROLLDRAG_THUMB;
        if ( ClientToScreen( dn ).Contains( mp ) ) VScrollbar.Dragmode = WB_SCROLLDRAG_DOWN;
        if ( ClientToScreen( b2 ).Contains( mp ) ) VScrollbar.Dragmode = WB_SCROLLDRAG_BUTTON2;
        if ( VScrollbar.Dragmode != WB_SCROLLDRAG_NONE )
        {
          VScrollbar.DragStartPosition = VScrollbar.ScrollPos;
          HandleVScrollbarClick( VScrollbar.Dragmode );
          App->SetCapture( this );
          return true;
        }
      }
    }
    return false;
    break;

  case WBM_LEFTBUTTONREPEAT:
    if ( App->GetMouseItem() == this )
    {
      if ( HScrollbar.Dragmode != WB_SCROLLDRAG_NONE && HScrollbar.Dragmode != WB_SCROLLDRAG_THUMB )
      {
        CRect b1, up, th, dn, b2;
        CPoint mp = Message.GetPosition(); //mouse pos
        if ( GetHScrollbarRectangles( b1, up, th, dn, b2 ) )
        {
          if ( ClientToScreen( b1 ).Contains( mp ) && HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1 )	HandleHScrollbarClick( HScrollbar.Dragmode );
          if ( ClientToScreen( up ).Contains( mp ) && HScrollbar.Dragmode == WB_SCROLLDRAG_UP )		HandleHScrollbarClick( HScrollbar.Dragmode );
          if ( ClientToScreen( dn ).Contains( mp ) && HScrollbar.Dragmode == WB_SCROLLDRAG_DOWN )		HandleHScrollbarClick( HScrollbar.Dragmode );
          if ( ClientToScreen( b2 ).Contains( mp ) && HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2 )	HandleHScrollbarClick( HScrollbar.Dragmode );
        }

        return true;
      }

      if ( VScrollbar.Dragmode != WB_SCROLLDRAG_NONE && VScrollbar.Dragmode != WB_SCROLLDRAG_THUMB )
      {
        CRect b1, up, th, dn, b2;
        CPoint mp = Message.GetPosition(); //mouse pos
        if ( GetVScrollbarRectangles( b1, up, th, dn, b2 ) )
        {
          if ( ClientToScreen( b1 ).Contains( mp ) && VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1 )	HandleVScrollbarClick( VScrollbar.Dragmode );
          if ( ClientToScreen( up ).Contains( mp ) && VScrollbar.Dragmode == WB_SCROLLDRAG_UP )		HandleVScrollbarClick( VScrollbar.Dragmode );
          if ( ClientToScreen( dn ).Contains( mp ) && VScrollbar.Dragmode == WB_SCROLLDRAG_DOWN )		HandleVScrollbarClick( VScrollbar.Dragmode );
          if ( ClientToScreen( b2 ).Contains( mp ) && VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2 )	HandleVScrollbarClick( VScrollbar.Dragmode );
        }

        return true;
      }
    }
    break;

  case WBM_MOUSEMOVE:
    if ( App->GetMouseCaptureItem() == this ) //handle scrollbars only
      if ( ScrollbarDragged() )
      {
        CPoint md = Message.GetPosition() - App->GetLeftDownPos(); //mouse delta

        if ( HScrollbar.Dragmode == WB_SCROLLDRAG_THUMB )
        {
          TS32 newpos = CalculateScrollbarMovement( HScrollbar, GetClientRect().Width(), md.x );
          HScrollbar.ScrollPos = newpos;
          App->SendMessage( CWBMessage( App, WBM_HSCROLL, GetGuid(), newpos ) );
        }
        if ( VScrollbar.Dragmode == WB_SCROLLDRAG_THUMB )
        {
          TS32 newpos = CalculateScrollbarMovement( VScrollbar, GetClientRect().Height(), md.y );
          VScrollbar.ScrollPos = newpos;
          App->SendMessage( CWBMessage( App, WBM_VSCROLL, GetGuid(), newpos ) );
        }

        return true;
      }
    return false;
    break;

  case WBM_LEFTBUTTONUP:
    if ( App->GetMouseCaptureItem() == this ) //handle scrollbars only
      if ( ScrollbarDragged() )
      {
        HScrollbar.Dragmode = WB_SCROLLDRAG_NONE;
        VScrollbar.Dragmode = WB_SCROLLDRAG_NONE;
        App->ReleaseCapture();
        return true;
      }
    return false;
    break;

  case WBM_REPOSITION:
    if ( Message.GetTarget() == Guid )
    {
      ApplyPosition( Message.Rectangle );
      if ( Message.Moved ) OnMove( Message.Rectangle.TopLeft() );
      if ( Message.Resized )
      {
        OnResize( Message.Rectangle.Size() );
      }
      for ( TS32 x = 0; x < Children.NumItems(); x++ )
        Children[ x ]->CalculateWindowPosition( GetClientRect().Size() );

      return true;
    }
    return false;
    break;

    //case WBM_FOCUSLOST:
    //	if (Message.GetTarget() == Guid)
    //	{
    //		if (Parent) Parent->ChildInFocus = NULL;
    //		return true;
    //	}
    //	return false;

  case WBM_HIDE:
    if ( Message.GetTarget() == Guid )
    {
      Hidden = true;
      return true;
    }
    return false;
    break;

  case WBM_UNHIDE:
    if ( Message.GetTarget() == Guid )
    {
      Hidden = false;
      return true;
    }
    return false;
    break;

  case WBM_CONTENTOFFSETCHANGE:
    if ( Message.GetTarget() == GetGuid() )
    {
      ChangeContentOffset( Message.GetPosition() );
      return true;
    }
    break;

  default:
    return false;
  }

  return false;
}

void CWBItem::DrawBackgroundItem( CWBDrawAPI *API, CWBDisplayProperties &Descriptor, CRect &Pos, WBITEMSTATE i, WBITEMVISUALCOMPONENT v )
{
  CColor bck = Descriptor.GetColor( i, WB_ITEM_BACKGROUNDCOLOR );
  if ( bck.A() )
  {
    //API->SetCropRect(ClientToScreen(Pos));
    API->DrawRect( Pos, Descriptor.GetColor( i, WB_ITEM_BACKGROUNDCOLOR ) );
  }

  WBSKINELEMENTID id = Descriptor.GetSkin( i, v );

  if ( id != 0xffffffff )
  {
    if ( id & 0x80000000 )
    {
      //skin element
      CWBSkinElement *e = App->GetSkin()->GetElement( id );
      if ( e )
      {
        WBALIGNMENT AlignX = (WBALIGNMENT)Descriptor.GetValue( i, WB_ITEM_BACKGROUNDALIGNMENT_X );
        WBALIGNMENT AlignY = (WBALIGNMENT)Descriptor.GetValue( i, WB_ITEM_BACKGROUNDALIGNMENT_Y );
        CSize elementsize = e->GetElementSize( API );
        CPoint offset = CPoint( 0, 0 );
        CSize size = Pos.Size();

        if ( e->GetBehavior( 0 ) == WB_SKINBEHAVIOR_PIXELCORRECT )
        {
          if ( AlignX == WB_ALIGN_RIGHT )	offset.x = Pos.Width() - elementsize.x;
          if ( AlignX == WB_ALIGN_CENTER )	offset.x = ( Pos.Width() - elementsize.x ) / 2;
          size.x = elementsize.x;
        }

        if ( e->GetBehavior( 1 ) == WB_SKINBEHAVIOR_PIXELCORRECT )
        {
          if ( AlignY == WB_ALIGN_BOTTOM )	offset.y = Pos.Height() - elementsize.y;
          if ( AlignY == WB_ALIGN_MIDDLE )	offset.y = ( Pos.Height() - elementsize.y ) / 2;
          size.y = elementsize.y;
        }

        CRect DisplayRect = CRect( Pos.TopLeft() + offset, Pos.TopLeft() + offset + size );

        App->GetSkin()->RenderElement( API, id, DisplayRect );
      }
    }
    else
    {	//mosaic
      App->GetSkin()->RenderElement( API, id, Pos );
    }
    return;
  }
}

void CWBItem::DrawBackground( CWBDrawAPI *API, WBITEMSTATE State )
{
  DrawBackground( API, GetWindowRect(), State, CSSProperties );
}

void CWBItem::DrawBackground( CWBDrawAPI *API )
{
  DrawBackground( API, GetState() );
}

void CWBItem::DrawBackground( CWBDrawAPI *API, CRect& rect, WBITEMSTATE State, CWBCSSPropertyBatch& cssProps )
{
  DrawBackgroundItem( API, cssProps.DisplayDescriptor, rect, State );
}

void CWBItem::DrawBorder( CWBDrawAPI *API )
{
  DrawBorder( API, GetWindowRect(), CSSProperties );
}

void CWBItem::DrawBorder( CWBDrawAPI *API, CRect& r, CWBCSSPropertyBatch& cssProps )
{
  auto crop = API->GetCropRect();

  CColor color = cssProps.DisplayDescriptor.GetColor( GetState(), WB_ITEM_BORDERCOLOR );

  API->SetCropRect( ClientToScreen( r ) );

  if ( cssProps.BorderSizes.x1 > 0 ) API->DrawRect( CRect( r.TopLeft(), r.BottomLeft() + CPoint( cssProps.BorderSizes.x1, 0 ) ), color );
  if ( cssProps.BorderSizes.y1 > 0 ) API->DrawRect( CRect( r.TopLeft(), r.TopRight() + CPoint( 0, cssProps.BorderSizes.y1 ) ), color );
  if ( cssProps.BorderSizes.x2 > 0 ) API->DrawRect( CRect( r.TopRight() - CPoint( cssProps.BorderSizes.x2, 0 ), r.BottomRight() ), color );
  if ( cssProps.BorderSizes.y2 > 0 ) API->DrawRect( CRect( r.BottomLeft() - CPoint( 0, cssProps.BorderSizes.y2 ), r.BottomRight() ), color );

  API->SetCropRect( crop );
}

void CWBItem::OnDraw( CWBDrawAPI *API )
{
  DrawBackground( API );
  DrawBorder( API );
}

void CWBItem::OnPostDraw( CWBDrawAPI *API )
{

}

void CWBItem::OnMove( const CPoint &p )
{

}

void CWBItem::OnResize( const CSize &s )
{

}

void CWBItem::OnMouseEnter()
{
  //LOG(LOG_DEBUG,_T("Mouse Entered Item %d"),GetGuid());
}

void CWBItem::OnMouseLeave()
{
  //LOG(LOG_DEBUG,_T("Mouse Left Item %d"),GetGuid());
}


void CWBItem::CalculateClientPosition()
{
  CRect OldClient = ClientRect;
  CPoint p = ClientToScreen( CPoint( 0, 0 ) );

  ClientRect = CSSProperties.PositionDescriptor.GetPadding( GetWindowRect().Size(), CSSProperties.BorderSizes );

  HScrollbar.Visible = false;
  VScrollbar.Visible = false;
  AdjustClientAreaToFitScrollbars();

  if ( p != ClientToScreen( CPoint( 0, 0 ) ) || ScreenRect.Width() < 0 )
    UpdateScreenRect(); //this updates all child items for the new position
}

void CWBItem::CalculateWindowPosition( const CSize &s )
{
  SetPosition( CSSProperties.PositionDescriptor.GetPosition( s, StoredContentSize, GetPosition() ) );
}

void CWBItem::DrawTree( CWBDrawAPI *API )
{
  if ( Hidden ) return;

  CRect PCrop = API->GetParentCropRect();
  CRect Crop = API->GetCropRect();
  API->SetParentCropRect( Crop );
  CPoint Offset = API->GetOffset();

  API->SetCropRect( ScreenRect );
  API->SetOffset( ClientToScreen( CPoint( 0, 0 ) ) );

  ApplyOpacity( API );
  OnDraw( API );

  if ( VScrollbar.Visible || HScrollbar.Visible )
  {
    API->SetCropRect( ScreenRect );
    DrawHScrollbar( API );
    DrawVScrollbar( API );
  }

  //crop children to client rect
  API->SetCropRect( ClientToScreen( GetClientRect() ) );

  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    Children[ x ]->DrawTree( API );

  OnPostDraw( API );

  API->SetParentCropRect( PCrop );
  API->SetCropRect( Crop );
  API->SetOffset( Offset );
}

void CWBItem::ApplyPosition( const CRect &Pos )
{
  CRect r = GetScreenRect();

  Position = Pos;
  CalculateClientPosition();
  UpdateScreenRect();

  CRect r2 = GetScreenRect();

  if ( App && r.Contains( App->GetMousePos() ) != r2.Contains( App->GetMousePos() ) )
    App->UpdateMouseItem();
}

TBOOL CWBItem::Focusable() const
{
  return true;
}

CWBItem *CWBItem::GetChildInFocus()
{
  return ChildInFocus;
}

CWBItem *CWBItem::SetCapture()
{
  return App->SetCapture( this );
}

TBOOL CWBItem::ReleaseCapture() const
{
  return App->ReleaseCapture();
}

TBOOL CWBItem::IsMouseTransparent( CPoint &ClientSpacePoint, WBMESSAGE MessageType )
{
  if ( ForceMouseTransparent ) return true;
  if ( Hidden ) return true;
  return false;
}

TBOOL CWBItem::FindItemInParentTree( CWBItem *Item )
{
  CWBItem *i = this;
  while ( i )
  {
    if ( i == Item ) return true;
    i = i->Parent;
  }
  return false;
}

CWBItem *CWBItem::GetItemUnderMouse( CPoint &Pos, CRect &CropRect, WBMESSAGE MessageType )
{
  CRect OldCropRect = CropRect;
  CropRect = CropRect | ScreenRect;

  if ( Hidden || !CropRect.Contains( Pos ) )
  {
    CropRect = OldCropRect;
    return NULL;
  }

  CropRect = CropRect | ClientToScreen( GetClientRect() );

  for ( TS32 x = 0; x < Children.NumItems(); x++ )
  {
    CWBItem *Res = Children[ Children.NumItems() - 1 - x ]->GetItemUnderMouse( Pos, CropRect, MessageType );
    if ( Res ) return Res;
  }

  CropRect = OldCropRect;

  if ( IsMouseTransparent( ScreenToClient( Pos ), MessageType ) )
    return nullptr;

  return this;
}

void CWBItem::SetChildAsTopmost( TS32 Index )
{
  CWBItem *i = Children[ Index ];
  Children.DeleteByIndex( Index );
  Children += i;
}

void CWBItem::SetChildAsBottommost( TS32 Index )
{
  CWBItem *i = Children[ Index ];
  Children.DeleteByIndex( Index );
  Children.InsertFirst( i );
}

void CWBItem::SetTopmost()
{
  if ( !GetParent() ) return;
  TS32 x = GetParent()->Children.Find( this );
  if ( x < 0 ) return;
  Parent->SetChildAsTopmost( x );
}

void CWBItem::SetBottommost()
{
  if ( !GetParent() ) return;
  TS32 x = GetParent()->Children.Find( this );
  if ( x < 0 ) return;
  Parent->SetChildAsBottommost( x );
}

CWBItem::CWBItem() : Guid( WB_GUID_COUNTER++ )
{
  Parent = NULL;
  App = NULL;
  ChildInFocus = NULL;
  Hidden = false;
  Disabled = false;
  Data = NULL;
}

CWBItem::CWBItem( CWBItem *parent, const CRect &position ) : Guid( WB_GUID_COUNTER++ )
{
  Initialize( parent, position );
}

CWBItem::~CWBItem()
{
  if ( Parent )
  {
    Parent->Children.Delete( this );
    if ( Parent->ChildInFocus == this )
      Parent->ChildInFocus = NULL;
  }

  Children.FreeArray();

  if ( App )
    App->UnRegisterItem( this );
}

void CWBItem::AddChild( CWBItem *Item )
{
  Children.Add( Item );
}

TS32 CWBItem::GetChildIndex( CWBItem *Item )
{
  return Children.Find( Item );
}

TBOOL CWBItem::Initialize( CWBItem *parent, const CRect &position )
{
  Hidden = false;
  Disabled = false;
  Data = NULL;
  ChildInFocus = NULL;
  Parent = parent;
  Scrollbar_ThumbMinimalSize = 4;
  Scrollbar_Size = 16;
  Scrollbar_ButtonSize = 16;
  StoredContentSize = position.Size();
  ContentOffset = CPoint( 0, 0 );

  SetBorderSizes( 0, 0, 0, 0 );

  if ( Parent )
  {
    App = Parent->GetApplication();
    if ( App )
      App->RegisterItem( this );
  }

  ApplyPosition( position );

  if ( Parent )
    Parent->AddChild( this );

  return true;
}

void CWBItem::SetParent( CWBItem* i )
{
  CWBItem *p = GetParent();
  if ( p->ChildInFocus == this )
    p->ChildInFocus = nullptr;
  p->Children.Delete( this );
  i->AddChild( this );
  Parent = i;
  App->ApplyStyle( i );
}

CRect CWBItem::GetClientRect() const
{
  return CRect( 0, 0, ClientRect.Width(), ClientRect.Height() );
}

CRect CWBItem::GetWindowRect() const
{
  return Position - ( Position.TopLeft() + ClientRect.TopLeft() );
}

CRect CWBItem::GetScreenRect() const
{
  return ScreenRect;
}

CPoint CWBItem::ClientToScreen( const CPoint &p ) const
{
  return ScreenRect.TopLeft() + ClientRect.TopLeft() + p;
}

CRect CWBItem::ClientToScreen( const CRect &p ) const
{
  return p + ScreenRect.TopLeft() + ClientRect.TopLeft();
}

CPoint CWBItem::ScreenToClient( const CPoint &p ) const
{
  return p - ScreenRect.TopLeft() - ClientRect.TopLeft();
}

CRect CWBItem::ScreenToClient( const CRect &p ) const
{
  return p - ScreenRect.TopLeft() - ClientRect.TopLeft();
}

void CWBItem::BuildPositionMessage( const CRect &Pos, CWBMessage &m )
{
  m = CWBMessage( App, WBM_REPOSITION, Guid );
  m.Rectangle = Pos;
  m.Moved = Pos.x1 != Position.x1 || Pos.y1 != Position.y1;
  m.Resized = Pos.Width() != Position.Width() || Pos.Height() != Position.Height();
}

void CWBItem::SetPosition( const CRect &Pos )
{
  //if (Pos==Position) return;

  CWBMessage m;
  BuildPositionMessage( Pos, m );
  App->SendMessage( m );
}

void CWBItem::SetClientPadding( TS32 left, TS32 top, TS32 right, TS32 bottom )
{
  if ( left != WBMARGIN_KEEP ) CSSProperties.PositionDescriptor.SetValue( WB_PADDING_LEFT, 0, (TF32)left );
  if ( right != WBMARGIN_KEEP ) CSSProperties.PositionDescriptor.SetValue( WB_PADDING_RIGHT, 0, (TF32)right );
  if ( top != WBMARGIN_KEEP ) CSSProperties.PositionDescriptor.SetValue( WB_PADDING_TOP, 0, (TF32)top );
  if ( bottom != WBMARGIN_KEEP ) CSSProperties.PositionDescriptor.SetValue( WB_PADDING_BOTTOM, 0, (TF32)bottom );

  CalculateClientPosition();
}

TBOOL CWBItem::IsWidthSet()
{
  return CSSProperties.PositionDescriptor.IsWidthSet();
}

TBOOL CWBItem::IsHeightSet()
{
  return CSSProperties.PositionDescriptor.IsHeightSet();
}

TS32 CWBItem::GetCalculatedWidth( CSize ParentSize )
{
  return CSSProperties.PositionDescriptor.GetWidth( ParentSize, StoredContentSize );
}

TS32 CWBItem::GetCalculatedHeight( CSize ParentSize )
{
  return CSSProperties.PositionDescriptor.GetHeight( ParentSize, StoredContentSize );
}

CRect CWBItem::GetPosition()
{
  return Position;
}

TBOOL CWBItem::InFocus()
{
  if ( !Parent ) return true;

  CWBItem *fi = App->GetRoot();
  while ( fi )
  {
    if ( fi == this ) return true;
    fi = fi->ChildInFocus;
  }
  return false;
}

TBOOL CWBItem::InLocalFocus()
{
  if ( !Parent ) return true;
  return Parent->ChildInFocus == this;
}

void CWBItem::SetFocus()
{
  CWBItem *fi = App->GetFocusItem();
  if ( fi != this )
  {
    App->SendMessage( CWBMessage( App, WBM_FOCUSGAINED, GetGuid() ) );
    if ( fi ) App->SendMessage( CWBMessage( App, WBM_FOCUSLOST, fi->GetGuid() ) );
  }

  CWBItem *p = Parent;
  CWBItem *i = this;
  while ( p )
  {
    p->ChildInFocus = i;
    i = p;
    p = i->Parent;
  }
}

void CWBItem::ClearFocus()
{
  //if (Parent) Parent->ChildInFocus = NULL;
  App->SendMessage( CWBMessage( App, WBM_FOCUSLOST, GetGuid() ) );
}

TBOOL CWBItem::MouseOver()
{
  CWBItem *mi = App->GetMouseItem();
  while ( mi )
  {
    if ( mi == this ) return true;
    mi = mi->Parent;
  }
  return false;
}

void CWBItem::SavePosition()
{
  StoredPosition = Position;
}

CRect CWBItem::GetSavedPosition() const
{
  return StoredPosition;
}

void CWBItem::SetSavedPosition( CRect& savedPos )
{
  StoredPosition = savedPos;
}

TU32 CWBItem::NumChildren()
{
  return Children.NumItems();
}

CWBItem * CWBItem::GetChild( TU32 idx )
{
  return Children[ idx ];
}

CSize CWBItem::GetContentSize()
{
  return GetClientRect().Size();
}

void CWBItem::Hide( TBOOL Hide )
{
  App->SendMessage( CWBMessage( App, Hide ? WBM_HIDE : WBM_UNHIDE, GetGuid() ) );
}

TBOOL CWBItem::IsHidden()
{
  return Hidden;
}

void CWBItem::SetData( void *data )
{
  Data = data;
}

void *CWBItem::GetData()
{
  return Data;
}

void CWBItem::MarkForDeletion( bool removeFromParent )
{
  if ( removeFromParent )
  {
    if ( Parent->ChildInFocus == this )
      Parent->ChildInFocus = NULL;
    Parent->Children.Delete( this );
    Parent = nullptr;
  }
  App->AddToTrash( this );
}

CWBContextMenu *CWBItem::OpenContextMenu( CPoint pos )
{
  if ( !App ) return NULL;
  CWBContextMenu *ctx = new CWBContextMenu( App->GetRoot(), CRect( pos, pos + CPoint( 10, 10 ) ), GetGuid() );
  App->ApplyStyle( ctx );
  return ctx;
}

void CWBItem::ScrollbardisplayHelperFunct( CWBScrollbarParams &s, TS32 &a1, TS32 &a2, TS32 &thumbsize, TS32 &thumbpos )
{
  a1 += Scrollbar_ButtonSize;
  a2 -= Scrollbar_ButtonSize;

  TS32 mi = s.MinScroll;
  TS32 ma = s.MaxScroll;

  TF32 scrollsize = (TF32)( ma - mi );
  TF32 rs = max( 0.0f, min( 1.0f, s.ViewSize / scrollsize ) );
  TF32 rp = max( 0.0f, min( 1.0f, ( s.ScrollPos - mi ) / ( scrollsize - s.ViewSize ) ) );

  thumbsize = (TS32)max( Scrollbar_ThumbMinimalSize, rs*( a2 - a1 ) );
  thumbpos = (TS32)( ( a2 - thumbsize - a1 )*rp ) + a1;
}

TS32 CWBItem::CalculateScrollbarMovement( CWBScrollbarParams &s, TS32 scrollbarsize, TS32 delta )
{
  TS32 a1 = 0;
  TS32 a2 = scrollbarsize;
  TS32 thumbsize = 0;
  TS32 thumbpos = 0;
  ScrollbardisplayHelperFunct( s, a1, a2, thumbsize, thumbpos );

  TS32 mi = s.MinScroll;
  TS32 ma = s.MaxScroll;
  TF32 scrollsize = (TF32)( ma - mi );

  TF32 sp = max( 0.0f, min( 1.0f, ( s.DragStartPosition - mi ) / ( scrollsize - s.ViewSize ) ) );
  TS32 thumbposstart = (TS32)( ( a2 - thumbsize - a1 )*sp );

  TS32 thumbposdelta = max( 0, min( a2 - thumbsize - a1, thumbposstart + delta ) );
  TS32 newscrollpos = (TS32)( ( thumbposdelta / (float)( a2 - thumbsize - a1 ) )*( scrollsize - s.ViewSize ) ) + mi;

  if ( a2 - thumbsize - a1 == 0 ) //invalid state
  {
    newscrollpos = s.ScrollPos;

    if ( s.ScrollPos<s.MinScroll && s.ScrollPos + s.ViewSize>s.MaxScroll )
      newscrollpos = s.MinScroll;
    else
      if ( s.ScrollPos < s.MinScroll )
        newscrollpos = s.MinScroll;
      else
        if ( s.ScrollPos + s.ViewSize > s.MaxScroll )
          newscrollpos = s.MaxScroll - s.ViewSize;

    if ( newscrollpos < s.MinScroll )
      newscrollpos = s.MinScroll;

  }

  return newscrollpos;
}

WBITEMSTATE CWBItem::GetScrollbarState( WBITEMVISUALCOMPONENT Component, CRect r )
{
  //mouse not over item, early exit if dragging is not in effect
  TBOOL HBar = Component == WB_ITEM_SCROLL_HBAR || Component == WB_ITEM_SCROLL_HTHUMB || Component == WB_ITEM_SCROLL_LEFT || Component == WB_ITEM_SCROLL_RIGHT;
  if ( !MouseOver() && ( HBar&&HScrollbar.Dragmode == WB_SCROLLDRAG_NONE || !HBar&&VScrollbar.Dragmode == WB_SCROLLDRAG_NONE ) ) return WB_STATE_NORMAL;

  CPoint MousePos = App->GetMousePos();

  CRect ScreenRect = ClientToScreen( r );
  TBOOL Hover = ScreenRect.Contains( MousePos );

  //don't highlight if something else uses the mouse (including this item)
  if ( App->GetMouseCaptureItem() )
    if ( HScrollbar.Dragmode == WB_SCROLLDRAG_NONE && VScrollbar.Dragmode == WB_SCROLLDRAG_NONE && !App->GetMouseCaptureItem()->AllowMouseHighlightWhileCaptureItem() ) return WB_STATE_NORMAL;

  //scrollbar not clicked, return hover/normal mode
  if ( HBar && HScrollbar.Dragmode == WB_SCROLLDRAG_NONE ) return Hover ? WB_STATE_HOVER : WB_STATE_NORMAL;
  if ( !HBar && VScrollbar.Dragmode == WB_SCROLLDRAG_NONE ) return Hover ? WB_STATE_HOVER : WB_STATE_NORMAL;

  //handle clicked modes
  switch ( Component )
  {
  case WB_ITEM_SCROLL_UP:
    if ( VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1 ) return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
    break;
  case WB_ITEM_SCROLL_DOWN:
    if ( VScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2 ) return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
    break;
  case WB_ITEM_SCROLL_LEFT:
    if ( HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON1 ) return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
    break;
  case WB_ITEM_SCROLL_RIGHT:
    if ( HScrollbar.Dragmode == WB_SCROLLDRAG_BUTTON2 ) return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
    break;
  case WB_ITEM_SCROLL_HBAR:
    if ( HScrollbar.Dragmode == WB_SCROLLDRAG_UP || HScrollbar.Dragmode == WB_SCROLLDRAG_DOWN ) return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
    if ( HScrollbar.Dragmode == WB_SCROLLDRAG_THUMB ) return WB_STATE_ACTIVE;
    break;
  case WB_ITEM_SCROLL_VBAR:
    if ( VScrollbar.Dragmode == WB_SCROLLDRAG_UP || VScrollbar.Dragmode == WB_SCROLLDRAG_DOWN ) return Hover ? WB_STATE_ACTIVE : WB_STATE_HOVER;
    if ( VScrollbar.Dragmode == WB_SCROLLDRAG_THUMB ) return WB_STATE_ACTIVE;
    break;
  case WB_ITEM_SCROLL_HTHUMB:
    if ( HScrollbar.Dragmode == WB_SCROLLDRAG_THUMB ) return WB_STATE_ACTIVE;
    break;
  case WB_ITEM_SCROLL_VTHUMB:
    if ( VScrollbar.Dragmode == WB_SCROLLDRAG_THUMB ) return WB_STATE_ACTIVE;
    break;
  default:
    break;
  }

  return WB_STATE_NORMAL;
}

TBOOL CWBItem::GetHScrollbarRectangles( CRect &button1, CRect &Scrollup, CRect &Thumb, CRect &Scrolldown, CRect &button2 )
{
  if ( !HScrollbar.Enabled || !HScrollbar.Visible ) return false;

  CRect r = CRect( GetClientRect().BottomLeft(), GetClientRect().BottomRight() + CPoint( 0, Scrollbar_Size ) );
  button1 = CRect( r.TopLeft(), r.BottomLeft() + CPoint( Scrollbar_ButtonSize, 0 ) );
  button2 = CRect( r.TopRight() - CPoint( Scrollbar_ButtonSize, 0 ), r.BottomRight() );

  TS32 thumbsize, thumbpos;
  ScrollbardisplayHelperFunct( HScrollbar, r.x1, r.x2, thumbsize, thumbpos );
  if ( ScrollbarRequired( HScrollbar ) )
  {
    Scrollup = CRect( r.x1, r.y1, thumbpos, r.y2 );
    Thumb = CRect( thumbpos, r.y1, thumbpos + thumbsize, r.y2 );
    Scrolldown = CRect( thumbpos + thumbsize, r.y1, r.x2, r.y2 );
  }
  else
    Scrollup = Thumb = Scrolldown = CRect( 1, 1, -1, -1 );

  return true;
}

void CWBItem::DrawScrollbarButton( CWBDrawAPI *API, CWBScrollbarParams &s, CRect &r, WBITEMVISUALCOMPONENT Button )
{
  WBITEMSTATE State = GetScrollbarState( Button, r );
  WBSKINELEMENTID ButtonSkin = CSSProperties.DisplayDescriptor.GetSkin( State, Button );

  if ( ButtonSkin == 0xffffffff )
  {
    CColor color = CColor::FromARGB( 0xff999999 );
    if ( State == WB_STATE_HOVER ) color = CColor::FromARGB( 0xff1c97ea );
    if ( State == WB_STATE_ACTIVE ) color = CColor::FromARGB( 0xff007acc );
    if ( !ScrollbarRequired( s ) ) color = CColor::FromARGB( 0xff555558 );

    CPoint margin = CPoint( 4, 4 );
    API->DrawRect( CRect( r.TopLeft() + margin, r.BottomRight() - margin ), color );
  }
  else
    App->GetSkin()->RenderElement( API, ButtonSkin, r );
}

void CWBItem::DrawHScrollbar( CWBDrawAPI *API )
{
  if ( !HScrollbar.Enabled || !HScrollbar.Visible ) return;
  CRect b1, su, th, sd, b2;
  GetHScrollbarRectangles( b1, su, th, sd, b2 );

  CRect pr = API->GetParentCropRect();
  CSize RealClientRectSize = CSSProperties.PositionDescriptor.GetPadding( GetWindowRect().Size(), CSSProperties.BorderSizes ).Size();
  CRect RealClientRect = ClientToScreen( CRect( 0, 0, RealClientRectSize.x, RealClientRectSize.y ) ) | pr;
  API->SetParentCropRect( RealClientRect );
  API->SetCropRect( RealClientRect );

  //draw background
  WBITEMSTATE BackgroundState = GetScrollbarState( WB_ITEM_SCROLL_HBAR, th );
  CRect BackgroundRect = CRect( b1.TopLeft(), b2.BottomRight() );
  WBSKINELEMENTID Background = CSSProperties.DisplayDescriptor.GetSkin( BackgroundState, WB_ITEM_SCROLL_HBAR );

  if ( Background == 0xffffffff )
    API->DrawRect( BackgroundRect, CColor::FromARGB( 0xff3e3e42 ) );
  else
    App->GetSkin()->RenderElement( API, Background, BackgroundRect );

  //draw thumb
  WBITEMSTATE ThumbState = GetScrollbarState( WB_ITEM_SCROLL_HTHUMB, th );
  WBSKINELEMENTID Thumb = CSSProperties.DisplayDescriptor.GetSkin( ThumbState, WB_ITEM_SCROLL_HTHUMB );

  if ( Thumb == 0xffffffff )
  {
    CColor color = CColor::FromARGB( 0xff686868 );
    if ( ThumbState == WB_STATE_HOVER ) color = CColor::FromARGB( 0xff9e9e9e );
    if ( ThumbState == WB_STATE_ACTIVE ) color = CColor::FromARGB( 0xffefebef );

    CPoint thumbmargin = CPoint( 0, 4 );
    API->DrawRect( CRect( th.TopLeft() + thumbmargin, th.BottomRight() - thumbmargin ), color );
  }
  else
    App->GetSkin()->RenderElement( API, Thumb, th );

  //draw buttons
  DrawScrollbarButton( API, HScrollbar, b2, WB_ITEM_SCROLL_RIGHT );
  DrawScrollbarButton( API, HScrollbar, b1, WB_ITEM_SCROLL_LEFT );

  API->SetParentCropRect( pr );
}

TBOOL CWBItem::GetVScrollbarRectangles( CRect &button1, CRect &Scrollup, CRect &Thumb, CRect &Scrolldown, CRect &button2 )
{
  if ( !VScrollbar.Enabled || !VScrollbar.Visible ) return false;

  CRect r = CRect( GetClientRect().TopRight(), GetClientRect().BottomRight() + CPoint( Scrollbar_Size, 0 ) );
  button1 = CRect( r.TopLeft(), r.TopRight() + CPoint( 0, Scrollbar_ButtonSize ) );
  button2 = CRect( r.BottomLeft() - CPoint( 0, Scrollbar_ButtonSize ), r.BottomRight() );

  TS32 thumbsize, thumbpos;
  ScrollbardisplayHelperFunct( VScrollbar, r.y1, r.y2, thumbsize, thumbpos );
  if ( ScrollbarRequired( VScrollbar ) )
  {
    Scrollup = CRect( r.x1, r.y1, r.x2, thumbpos );
    Thumb = CRect( r.x1, thumbpos, r.x2, thumbpos + thumbsize );
    Scrolldown = CRect( r.x1, thumbpos + thumbsize, r.x2, r.y2 );
  }
  else
    Scrollup = Thumb = Scrolldown = CRect( 1, 1, -1, -1 );

  return true;
}

void CWBItem::DrawVScrollbar( CWBDrawAPI *API )
{
  if ( !VScrollbar.Enabled || !VScrollbar.Visible ) return;
  CRect b1, su, th, sd, b2;
  GetVScrollbarRectangles( b1, su, th, sd, b2 );

  CRect pr = API->GetParentCropRect();
  CSize RealClientRectSize = CSSProperties.PositionDescriptor.GetPadding( GetWindowRect().Size(), CSSProperties.BorderSizes ).Size();
  CRect RealClientRect = ClientToScreen( CRect( 0, 0, RealClientRectSize.x, RealClientRectSize.y ) ) | pr;
  API->SetParentCropRect( RealClientRect );
  API->SetCropRect( RealClientRect );

  //draw background
  WBITEMSTATE BackgroundState = GetScrollbarState( WB_ITEM_SCROLL_VBAR, th );
  CRect BackgroundRect = CRect( b1.TopLeft(), b2.BottomRight() );
  WBSKINELEMENTID Background = CSSProperties.DisplayDescriptor.GetSkin( BackgroundState, WB_ITEM_SCROLL_VBAR );

  if ( Background == 0xffffffff )
    API->DrawRect( BackgroundRect, CColor::FromARGB( 0xff3e3e42 ) );
  else
    App->GetSkin()->RenderElement( API, Background, BackgroundRect );

  //draw thumb
  WBITEMSTATE ThumbState = GetScrollbarState( WB_ITEM_SCROLL_VTHUMB, th );
  WBSKINELEMENTID Thumb = CSSProperties.DisplayDescriptor.GetSkin( ThumbState, WB_ITEM_SCROLL_VTHUMB );

  if ( Thumb == 0xffffffff )
  {
    CColor color = CColor::FromARGB( 0xff686868 );
    if ( ThumbState == WB_STATE_HOVER ) color = CColor::FromARGB( 0xff9e9e9e );
    if ( ThumbState == WB_STATE_ACTIVE ) color = CColor::FromARGB( 0xffefebef );

    CPoint thumbmargin = CPoint( 4, 0 );
    API->DrawRect( CRect( th.TopLeft() + thumbmargin, th.BottomRight() - thumbmargin ), color );
  }
  else
    App->GetSkin()->RenderElement( API, Thumb, th );

  //draw buttons
  DrawScrollbarButton( API, VScrollbar, b2, WB_ITEM_SCROLL_DOWN );
  DrawScrollbarButton( API, VScrollbar, b1, WB_ITEM_SCROLL_UP );

  API->SetParentCropRect( pr );
}

TBOOL CWBItem::ScrollbarDragged()
{
  return HScrollbar.Dragmode != WB_SCROLLDRAG_NONE || VScrollbar.Dragmode != WB_SCROLLDRAG_NONE;
}

void CWBItem::ScrollbarHelperFunct( CWBScrollbarParams &s, TS32 &r, TBOOL ScrollbarNeeded )
{
  if ( !s.Enabled )
  {
    if ( s.Visible ) //remove scrollbar
    {
      r += Scrollbar_Size;
      s.Visible = false;
    }
  }
  else
  {
    if ( !s.Dynamic )
    {
      if ( !s.Visible ) //add scrollbar
      {
        r -= Scrollbar_Size;
        s.Visible = true;
      }
    }
    else
    {
      if ( ScrollbarNeeded != s.Visible )
      {
        if ( s.Visible )
          r += Scrollbar_Size; //remove scrollbar
        else
          r -= Scrollbar_Size; //add scrollbar
        s.Visible = !s.Visible;
      }
    }
  }
}

TBOOL CWBItem::ScrollbarRequired( CWBScrollbarParams &s )
{
  return s.ViewSize < s.MaxScroll - s.MinScroll || ( s.ScrollPos < s.MinScroll && s.ScrollPos + s.ViewSize < s.MaxScroll ) || ( s.ScrollPos > s.MaxScroll - s.ViewSize && s.ScrollPos > s.MinScroll );
}

void CWBItem::AdjustClientAreaToFitScrollbars()
{
  CRect crect = ClientRect;

  //x axis
  ScrollbarHelperFunct( VScrollbar, crect.x2, ScrollbarRequired( VScrollbar ) );

  //y axis
  ScrollbarHelperFunct( HScrollbar, crect.y2, ScrollbarRequired( HScrollbar ) );

  //if (ClientRect != crect)
  //{
  //	if (GetType() != _T("console"))
  //	LOG_DBG("Scrollbar added/removed (HScroll: %d VScroll: %d) from clientarea of %d (%s #%s - .%s)", (TS32)ScrollbarRequired(HScrollbar), (TS32)ScrollbarRequired(VScrollbar), GetGuid(), GetType().GetPointer(), GetID().GetPointer(), GetClassString().GetPointer());
  //}

  if ( App && crect != ClientRect )
    App->SendMessage( CWBMessage( App, WBM_CLIENTAREACHANGED, GetGuid() ) );

  ClientRect = crect;
}

void CWBItem::EnableHScrollbar( TBOOL Enabled, TBOOL Dynamic )
{
  HScrollbar.Enabled = Enabled;
  HScrollbar.Dynamic = Dynamic;
  AdjustClientAreaToFitScrollbars();
}

void CWBItem::EnableVScrollbar( TBOOL Enabled, TBOOL Dynamic )
{
  VScrollbar.Enabled = Enabled;
  VScrollbar.Dynamic = Dynamic;
  AdjustClientAreaToFitScrollbars();
}

TBOOL CWBItem::IsHScrollbarEnabled()
{
  return HScrollbar.Enabled;
}

TBOOL CWBItem::IsVScrollbarEnabled()
{
  return VScrollbar.Enabled;
}

void CWBItem::SetHScrollbarParameters( TS32 MinScroll, TS32 MaxScroll, TS32 ViewSize )
{
  TBOOL Changed = HScrollbar.MinScroll != MinScroll || HScrollbar.MaxScroll != MaxScroll || HScrollbar.ViewSize != ViewSize;
  HScrollbar.MinScroll = MinScroll;
  HScrollbar.MaxScroll = MaxScroll;
  HScrollbar.ViewSize = ViewSize;
  if ( Changed ) CalculateClientPosition();
}

void CWBItem::SetVScrollbarParameters( TS32 MinScroll, TS32 MaxScroll, TS32 ViewSize )
{
  TBOOL Changed = VScrollbar.MinScroll != MinScroll || VScrollbar.MaxScroll != MaxScroll || VScrollbar.ViewSize != ViewSize;
  VScrollbar.MinScroll = MinScroll;
  VScrollbar.MaxScroll = MaxScroll;
  VScrollbar.ViewSize = ViewSize;
  if ( Changed ) CalculateClientPosition();
}

void CWBItem::GetHScrollbarParameters( TS32 &MinScroll, TS32 &MaxScroll, TS32 &ViewSize )
{
  MinScroll = HScrollbar.MinScroll;
  MaxScroll = HScrollbar.MaxScroll;
  ViewSize = HScrollbar.ViewSize;
}

void CWBItem::GetVScrollbarParameters( TS32 &MinScroll, TS32 &MaxScroll, TS32 &ViewSize )
{
  MinScroll = VScrollbar.MinScroll;
  MaxScroll = VScrollbar.MaxScroll;
  ViewSize = VScrollbar.ViewSize;
}

void CWBItem::SetHScrollbarPos( TS32 ScrollPos, TBOOL Clamp )
{
  TS32 sc = ScrollPos;
  if ( Clamp )
    sc = max( HScrollbar.MinScroll, min( sc, HScrollbar.MaxScroll - HScrollbar.ViewSize ) );
  if ( sc != HScrollbar.ScrollPos )
  {
    HScrollbar.ScrollPos = sc;
    App->SendMessage( CWBMessage( App, WBM_HSCROLL, GetGuid(), sc ) );
    CalculateClientPosition();
  }
}

void CWBItem::SetVScrollbarPos( TS32 ScrollPos, TBOOL Clamp )
{
  TS32 sc = ScrollPos;
  if ( Clamp )
    sc = max( VScrollbar.MinScroll, min( sc, VScrollbar.MaxScroll - VScrollbar.ViewSize ) );
  if ( sc != VScrollbar.ScrollPos )
  {
    VScrollbar.ScrollPos = sc;
    App->SendMessage( CWBMessage( App, WBM_VSCROLL, GetGuid(), sc ) );
    CalculateClientPosition();
  }
}

void CWBItem::ApplyRelativePosition()
{
  if ( Parent )
    SetPosition( CSSProperties.PositionDescriptor.GetPosition( Parent->GetClientRect().Size(), StoredContentSize, GetPosition() ) );
}

void CWBItem::VisualStyleApplicator( CWBDisplayProperties &desc, WBITEMVISUALCOMPONENT TargetComponent, TS32 Value, CStringArray &pseudo )
{
  TS32 StateCount = 0;
  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    CString p = pseudo[ x ].Trimmed();
    if ( p == _T( "active" ) || p == _T( "hover" ) || p == _T( "disabled" ) || p == _T( "disabled-active" ) || p == _T( "normal" ) ) StateCount++;
  }

  if ( !StateCount )
  {
    desc.SetValue( WB_STATE_NORMAL, TargetComponent, Value );
    desc.SetValue( WB_STATE_ACTIVE, TargetComponent, Value );
    desc.SetValue( WB_STATE_HOVER, TargetComponent, Value );
    desc.SetValue( WB_STATE_DISABLED, TargetComponent, Value );
    desc.SetValue( WB_STATE_DISABLED_ACTIVE, TargetComponent, Value );
  }
  else
  {
    for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
    {
      CString p = pseudo[ x ].Trimmed();
      if ( p == _T( "active" ) )
      {
        desc.SetValue( WB_STATE_ACTIVE, TargetComponent, Value );
        continue;
      }
      if ( p == _T( "hover" ) )
      {
        desc.SetValue( WB_STATE_HOVER, TargetComponent, Value );
        continue;
      }
      if ( p == _T( "disabled" ) )
      {
        desc.SetValue( WB_STATE_DISABLED, TargetComponent, Value );
        continue;
      }
      if ( p == _T( "disabled-active" ) )
      {
        desc.SetValue( WB_STATE_DISABLED_ACTIVE, TargetComponent, Value );
        continue;
      }
      if ( p == _T( "normal" ) )
      {
        desc.SetValue( WB_STATE_NORMAL, TargetComponent, Value );
        continue;
      }
    }
  }
}

TBOOL CWBItem::InterpretPositionString( CWBCSSPropertyBatch &props, CString & prop, CString & value, CStringArray &pseudo )
{
  if ( prop == _T( "left" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_LEFT, value );
    return true;
  }

  if ( prop == _T( "right" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_RIGHT, value );
    return true;
  }

  if ( prop == _T( "top" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_TOP, value );
    return true;
  }

  if ( prop == _T( "bottom" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_BOTTOM, value );
    return true;
  }

  if ( prop == _T( "width" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_WIDTH, value );
    return true;
  }

  if ( prop == _T( "height" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_HEIGHT, value );
    return true;
  }

  if ( prop == _T( "padding-left" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_PADDING_LEFT, value );
    return true;
  }

  if ( prop == _T( "padding-right" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_PADDING_RIGHT, value );
    return true;
  }

  if ( prop == _T( "padding-top" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_PADDING_TOP, value );
    return true;
  }

  if ( prop == _T( "padding-bottom" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_PADDING_BOTTOM, value );
    return true;
  }

  if ( prop == _T( "margin" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_LEFT, value );
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_RIGHT, value );
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_TOP, value );
    PositionApplicator( props.PositionDescriptor, WB_MARGIN_BOTTOM, value );
    return true;
  }

  if ( prop == _T( "padding" ) )
  {
    PositionApplicator( props.PositionDescriptor, WB_PADDING_LEFT, value );
    PositionApplicator( props.PositionDescriptor, WB_PADDING_RIGHT, value );
    PositionApplicator( props.PositionDescriptor, WB_PADDING_TOP, value );
    PositionApplicator( props.PositionDescriptor, WB_PADDING_BOTTOM, value );
    return true;
  }

  TS32 dw = 0;

  if ( prop == _T( "border" ) )
  {
    if ( ScanPXValue( value, dw, prop ) ) props.BorderSizes = CRect( dw, dw, dw, dw );
    return true;
  }

  if ( prop == _T( "border-left" ) )
  {
    if ( ScanPXValue( value, dw, prop ) ) props.BorderSizes.x1 = dw;
    return true;
  }

  if ( prop == _T( "border-top" ) )
  {
    if ( ScanPXValue( value, dw, prop ) ) props.BorderSizes.y1 = dw;
    return true;
  }

  if ( prop == _T( "border-right" ) )
  {
    if ( ScanPXValue( value, dw, prop ) ) props.BorderSizes.x2 = dw;
    return true;
  }

  if ( prop == _T( "border-bottom" ) )
  {
    if ( ScanPXValue( value, dw, prop ) ) props.BorderSizes.y2 = dw;
    return true;
  }


  return false;
}

CStringArray CWBItem::ExplodeValueWithoutSplittingParameters( CString String )
{
  CStringArray aOut;
  int nPrevious = 0;
  int nNext = 0;

#ifndef UNICODE
#define SPACE isspace
#define SPACETYPE unsigned char
#else
#define SPACE iswspace
#define SPACETYPE wchar_t
#endif

  unsigned int x = 0;
  TS32 bracketcnt = 0;

  while ( x < String.Length() )
  {
    while ( x < String.Length() && SPACE( (SPACETYPE)String[ x ] ) ) x++;

    if ( String[ x ] == _T( '(' ) ) bracketcnt++;
    if ( String[ x ] == _T( ')' ) && bracketcnt ) bracketcnt--;

    nPrevious = x;
    while ( x < String.Length() && ( bracketcnt || ( !SPACE( (SPACETYPE)String[ x ] ) ) ) )
    {
      if ( String[ x ] == _T( '(' ) ) bracketcnt++;
      if ( String[ x ] == _T( ')' ) && bracketcnt ) bracketcnt--;
      x++;
    }

    aOut.Add( String.Substring( nPrevious, x - nPrevious ) );
  }

  return aOut;
}

TBOOL CWBItem::InterpretDisplayString( CWBCSSPropertyBatch &props, CString & prop, CString & value, CStringArray &pseudo )
{
  if ( prop == _T( "background" ) )
  {
    CStringArray Attribs = ExplodeValueWithoutSplittingParameters( value );

    for ( TS32 x = 0; x < Attribs.NumItems(); x++ )
    {
      TU32 dw = 0;
      if ( Attribs[ x ].Scan( _T( "#%x" ), &dw ) == 1 )	VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( dw | 0xff000000 ), pseudo );
      if ( Attribs[ x ] == ( _T( "none" ) ) )
      {
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR, 0, pseudo );
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE, 0xffffffff, pseudo );
      }

      if ( Attribs[ x ] == ( _T( "left" ) ) )				VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_LEFT, pseudo );
      if ( Attribs[ x ] == ( _T( "center" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_CENTER, pseudo );
      if ( Attribs[ x ] == ( _T( "right" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_RIGHT, pseudo );
      if ( Attribs[ x ] == ( _T( "top" ) ) )				VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_TOP, pseudo );
      if ( Attribs[ x ] == ( _T( "middle" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_MIDDLE, pseudo );
      if ( Attribs[ x ] == ( _T( "bottom" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_BOTTOM, pseudo );

      if ( Attribs[ x ].Find( _T( "rgba(" ) ) == 0 )
      {
        CColor col;
        if ( !ParseRGBA( Attribs[ x ], col ) )
        {
          LOG_WARN( "[gui] CSS rgba() description invalid, skipping: %s", Attribs[ x ].GetPointer() );
          continue;
        }
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR, col, pseudo );
      }

      WBSKINELEMENTID id;
      if ( ScanSkinValue( Attribs[ x ], id, prop ) )
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE, id, pseudo );
    }

    return true;
  }

  if ( prop == _T( "background-color" ) )
  {
    CStringArray Attribs = ExplodeValueWithoutSplittingParameters( value );

    for ( TS32 x = 0; x < Attribs.NumItems(); x++ )
    {
      TU32 dw = 0;
      if ( Attribs[ x ].Scan( _T( "#%x" ), &dw ) == 1 )	VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( dw | 0xff000000 ), pseudo );
      if ( Attribs[ x ] == ( _T( "none" ) ) )				VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR, 0, pseudo );

      if ( Attribs[ x ].Find( _T( "rgba(" ) ) == 0 )
      {
        CColor col;
        if ( !ParseRGBA( Attribs[ x ], col ) )
        {
          LOG_WARN( "[gui] CSS rgba() description invalid, skipping: %s", Attribs[ x ].GetPointer() );
          continue;
        }
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDCOLOR, col, pseudo );
      }
    }

    return true;
  }

  if ( prop == _T( "foreground-color" ) )
  {
    CStringArray Attribs = ExplodeValueWithoutSplittingParameters( value );

    for ( TS32 x = 0; x < Attribs.NumItems(); x++ )
    {
      TU32 dw = 0;
      if ( Attribs[ x ].Scan( _T( "#%x" ), &dw ) == 1 )	VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_FOREGROUNDCOLOR, CColor::FromARGB( dw | 0xff000000 ), pseudo );
      if ( Attribs[ x ] == ( _T( "none" ) ) )				VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_FOREGROUNDCOLOR, 0, pseudo );

      if ( Attribs[ x ].Find( _T( "rgba(" ) ) == 0 )
      {
        CColor col;
        if ( !ParseRGBA( Attribs[ x ], col ) )
        {
          LOG_WARN( "[gui] CSS rgba() description invalid, skipping: %s", Attribs[ x ].GetPointer() );
          continue;
        }
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_FOREGROUNDCOLOR, col, pseudo );
      }
    }

    return true;
  }

  if ( prop == _T( "background-position" ) )
  {
    CStringArray Attribs = value.ExplodeByWhiteSpace();

    for ( TS32 x = 0; x < Attribs.NumItems(); x++ )
    {
      TU32 dw = 0;

      if ( Attribs[ x ] == ( _T( "left" ) ) )				VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_LEFT, pseudo );
      if ( Attribs[ x ] == ( _T( "center" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_CENTER, pseudo );
      if ( Attribs[ x ] == ( _T( "right" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_X, WB_ALIGN_RIGHT, pseudo );
      if ( Attribs[ x ] == ( _T( "top" ) ) )				VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_TOP, pseudo );
      if ( Attribs[ x ] == ( _T( "middle" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_MIDDLE, pseudo );
      if ( Attribs[ x ] == ( _T( "bottom" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDALIGNMENT_Y, WB_ALIGN_BOTTOM, pseudo );
    }

    return true;
  }

  if ( prop == _T( "background-image" ) )
  {
    CStringArray Attribs = ExplodeValueWithoutSplittingParameters( value );

    for ( TS32 x = 0; x < Attribs.NumItems(); x++ )
    {
      TU32 dw = 0;

      if ( Attribs[ x ] == ( _T( "none" ) ) )				VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE, 0xffffffff, pseudo );

      WBSKINELEMENTID id;
      if ( ScanSkinValue( Attribs[ x ], id, prop ) )
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BACKGROUNDIMAGE, id, pseudo );
    }

    return true;
  }

  if ( prop == _T( "border-color" ) )
  {
    TU32 dw = 0;
    value.Scan( _T( "#%x" ), &dw );
    VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_BORDERCOLOR, CColor::FromARGB( dw | 0xff000000 ), pseudo );
    return true;
  }

  if ( prop == _T( "opacity" ) )
  {
    TF32 dw = 0;
    value.Scan( _T( "%f" ), &dw );

    TS32 o = (TS32)max( 0, min( 255, dw * 255 ) );

    VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_OPACITY, CColor::FromARGB( o * 0x01010101 ), pseudo );
    return true;
  }

  if ( prop[ 0 ] == _T( 's' ) ) //quick check for scrollbar related stuff
  {
    WBSKINELEMENTID id;

    if ( prop == _T( "scrollbar-up" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_UP, id, pseudo );
      return true;
    }
    if ( prop == _T( "scrollbar-down" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_DOWN, id, pseudo );
      return true;
    }
    if ( prop == _T( "scrollbar-left" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_LEFT, id, pseudo );
      return true;
    }
    if ( prop == _T( "scrollbar-right" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_RIGHT, id, pseudo );
      return true;
    }
    if ( prop == _T( "scrollbar-background-horizontal" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_HBAR, id, pseudo );
      return true;
    }
    if ( prop == _T( "scrollbar-background-vertical" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_VBAR, id, pseudo );
      return true;
    }
    if ( prop == _T( "scrollbar-thumb-horizontal" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_HTHUMB, id, pseudo );
      return true;
    }
    if ( prop == _T( "scrollbar-thumb-vertical" ) )
    {
      if ( ScanSkinValue( value, id, prop ) ) VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_SCROLL_VTHUMB, id, pseudo );
      return true;
    }
  }

  if ( InterpretFontString( props, prop, value, pseudo ) ) return true;

  return false;
}


TBOOL CWBItem::InterpretFontString( CWBCSSPropertyBatch &props, CString & prop, CString & value, CStringArray &pseudo )
{

  if ( prop == _T( "font-color" ) )
  {
    TU32 dw = 0;
    if ( value.Scan( _T( "#%x" ), &dw ) == 1 )
    {
      VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_FONTCOLOR, CColor::FromARGB( dw | 0xff000000 ), pseudo );
      return true;
    }

    CColor col;
    if ( ParseRGBA( value, col ) )
      VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_FONTCOLOR, col, pseudo );

    return true;
  }

  if ( prop == _T( "text-transform" ) )
  {
    if ( value == ( _T( "none" ) ) )			VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM, WBTT_NONE, pseudo );
    if ( value == ( _T( "capitalize" ) ) )	VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM, WBTT_CAPITALIZE, pseudo );
    if ( value == ( _T( "uppercase" ) ) )		VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM, WBTT_UPPERCASE, pseudo );
    if ( value == ( _T( "lowercase" ) ) )		VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_TEXTTRANSFORM, WBTT_LOWERCASE, pseudo );

    return true;
  }

  if ( prop == _T( "font" ) )
  {
    CStringArray Attribs = ExplodeValueWithoutSplittingParameters( value );

    for ( TS32 x = 0; x < Attribs.NumItems(); x++ )
    {
      //try to apply as color
      TU32 dw = 0;
      if ( Attribs[ x ].Scan( _T( "#%x" ), &dw ) == 1 )
      {
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_FONTCOLOR, CColor::FromARGB( dw | 0xff000000 ), pseudo );
        continue;
      }

      CColor col;
      if ( ParseRGBA( Attribs[ x ], col ) )
      {
        VisualStyleApplicator( props.DisplayDescriptor, WB_ITEM_FONTCOLOR, col, pseudo );
        continue;
      }

      //if failed apply as font
      FontStyleApplicator( props, pseudo, Attribs[ x ] );
    }

    return true;
  }

  if ( prop == _T( "font-family" ) )
  {
    FontStyleApplicator( props, pseudo, value );
    return true;
  }

  if ( prop == _T( "text-align" ) )
  {
    if ( value == _T( "left" ) ) props.TextAlignX = WBTA_LEFT;
    if ( value == _T( "center" ) ) props.TextAlignX = WBTA_CENTERX;
    if ( value == _T( "right" ) ) props.TextAlignX = WBTA_RIGHT;
    return true;
  }

  if ( prop == _T( "vertical-align" ) )
  {
    if ( value == _T( "top" ) ) props.TextAlignY = WBTA_TOP;
    if ( value == _T( "middle" ) ) props.TextAlignY = WBTA_CENTERY;
    if ( value == _T( "bottom" ) ) props.TextAlignY = WBTA_BOTTOM;
    return true;
  }

  return false;
}

TBOOL CWBItem::ApplyStyle( CString & prop, CString & value, CStringArray &pseudo )
{
  if ( InterpretPositionString( CSSProperties, prop, value, pseudo ) )
  {
    ContentChanged();
    return true;
  }
  if ( InterpretDisplayString( CSSProperties, prop, value, pseudo ) )
  {
    ContentChanged();
    return true;
  }

  if ( prop == _T( "visibility" ) )
  {
    if ( value == _T( "hidden" ) )
    {
      Hidden = true;
      return true;
    }
    if ( value == _T( "visible" ) )
    {
      Hidden = false;
      return true;
    }
    LOG_WARN( "[guiitem] Item style error: invalid visibility value '%s'", value.GetPointer() );
    return true;
  }

  if ( prop == _T( "overflow" ) )
  {
    if ( value == _T( "hidden" ) )
    {
      EnableHScrollbar( false, false );
      EnableVScrollbar( false, false );
      return true;
    }

    if ( value == _T( "auto" ) )
    {
      EnableHScrollbar( true, true );
      EnableVScrollbar( true, true );
      return true;
    }

    if ( value == _T( "scroll" ) )
    {
      EnableHScrollbar( true, false );
      EnableVScrollbar( true, false );
      return true;
    }
  }

  if ( prop == _T( "overflow-x" ) )
  {
    if ( value == _T( "hidden" ) )
    {
      EnableHScrollbar( false, false );
      return true;
    }

    if ( value == _T( "auto" ) )
    {
      EnableHScrollbar( true, true );
      return true;
    }

    if ( value == _T( "scroll" ) )
    {
      EnableHScrollbar( true, false );
      return true;
    }
  }

  if ( prop == _T( "overflow-y" ) )
  {
    if ( value == _T( "hidden" ) )
    {
      EnableVScrollbar( false, false );
      return true;
    }

    if ( value == _T( "auto" ) )
    {
      EnableVScrollbar( true, true );
      return true;
    }

    if ( value == _T( "scroll" ) )
    {
      EnableVScrollbar( true, false );
      return true;
    }
  }

  if ( prop[ 0 ] == _T( 's' ) ) //quick check for scrollbar related stuff
  {
    TS32 dw = 0;

    if ( prop == _T( "scrollbar-size" ) )
    {
      if ( ScanPXValue( value, dw, prop ) ) Scrollbar_Size = dw;
      return true;
    }

    if ( prop == _T( "scrollbar-button-size" ) )
    {
      if ( ScanPXValue( value, dw, prop ) ) Scrollbar_ButtonSize = dw;
      return true;
    }

    if ( prop == _T( "scrollbar-thumb-minimum-size" ) )
    {
      if ( ScanPXValue( value, dw, prop ) ) Scrollbar_ThumbMinimalSize = dw;
      return true;
    }
  }


  return false;
}

void CWBItem::PositionApplicator( CWBPositionDescriptor &pos, WBPOSITIONTYPE Type, CString &value )
{
  if ( Type == WB_WIDTH || Type == WB_HEIGHT )
  {
    if ( value == _T( "none" ) )
    {
      pos.ClearMetrics( Type );
      return;
    }
    if ( value == _T( "auto" ) )
    {
      pos.ClearMetrics( Type );
      pos.SetAutoSize( Type );
      return;
    }

  }
  else
    if ( value == _T( "auto" ) )
    {
      pos.ClearMetrics( Type );
      return;
    }

  TBOOL px = value.Find( _T( "px" ) ) >= 0;
  TBOOL pc = value.Find( _T( "%" ) ) >= 0;

  TF32 pxv = 0;
  TF32 pcv = 0;

  if ( !pc && !px )
  {
    LOG_WARN( "[guiitem] Item style error: missing 'px' or '%%' in '%s' value", value.GetPointer() );
    return;
  }

  if ( px && !pc )
  {
    if ( value.Scan( _T( "%fpx" ), &pxv ) != 1 )
    {
      LOG_WARN( "[guiitem] Item style error: invalid value '%s' (px)", value.GetPointer() );
      return;
    }
    pos.ClearMetrics( Type );
    pos.SetMetric( Type, WB_PIXELS, pxv );
    return;
  }

  if ( pc && !px )
  {
    if ( value.Scan( _T( "%f%%" ), &pcv ) != 1 )
    {
      LOG_WARN( "[guiitem] Item style error: invalid value '%s' (%%)", value.GetPointer() );
      return;
    }
    pos.ClearMetrics( Type );
    pos.SetMetric( Type, WB_RELATIVE, pcv / 100.0f );
    return;
  }

  if ( value.Scan( _T( "%fpx%f%%" ), &pxv, &pcv ) != 2 )
    if ( value.Scan( _T( "%f%%%fpx" ), &pcv, &pxv ) != 2 )
    {
      LOG_WARN( "[guiitem] Item style error: invalid value '%s' (px, %%)", value.GetPointer() );
      return;
    }

  pos.ClearMetrics( Type );
  pos.SetMetric( Type, WB_PIXELS, pxv );
  pos.SetMetric( Type, WB_RELATIVE, pcv / 100.0f );
}

CWBItem * CWBItem::FindChildByID( CString &value, CString &type )
{
  CWBItem *i = ChildSearcherFunct( value, type );
  //if ( !i )
  //{
  //  if ( !type.Length() )
  //    LOG_WARN( "[gui] UI item '%s' not found!", value.GetPointer() );
  //  else
  //    LOG_WARN( "[gui] UI item '%s' of type '%s' not found!", value.GetPointer(), type.GetPointer() );
  //}
  return i;
}

CWBItem * CWBItem::FindChildByID( TCHAR *value, const TCHAR *type )
{
  return FindChildByID( CString( value ), CString( type ) );
}

CWBItem * CWBItem::FindChildByID( CString &value, const TCHAR *type )
{
  return FindChildByID( value, CString( type ) );
}

CWBItem * CWBItem::FindParentByID( CString &value, CString &type )
{
  CWBItem *i = GetParent();
  while ( i )
  {
    if ( i->GetID() == value )
    {
      if ( !type.Length() ) return i;
      if ( i->InstanceOf( type ) ) return i;
      //if (type == i->GetType()) return i;
      //LOG_WARN( "[gui] Found UI item '%s' of wrong type '%s'. Type should be '%s'", value.GetPointer(), GetType().GetPointer(), type.GetPointer() );
    }
    i = i->GetParent();
  }
  return NULL;
}

CWBItem * CWBItem::FindParentByID( TCHAR *value, TCHAR *type )
{
  return FindParentByID( CString( value ), CString( type ) );
}

CWBItem * CWBItem::FindParentByID( CString &value, TCHAR *type )
{
  return FindParentByID( value, CString( type ) );
}

CWBItem * CWBItem::ChildSearcherFunct( CString &value, CString &type )
{
  if ( GetID() == value )
  {
    if ( type.Length() == 0 ) return this;
    if ( InstanceOf( type ) ) return this;

    //LOG_WARN( "[gui] Found UI item '%s' of wrong type '%s'. Type should be '%s'", value.GetPointer(), GetType().GetPointer(), type.GetPointer() );
  }

  for ( TS32 x = 0; x < Children.NumItems(); x++ )
  {
    CWBItem *i = Children[ x ]->ChildSearcherFunct( value, type );
    if ( i ) return i;
  }

  return NULL;
}

void CWBItem::DeleteChildren()
{
  Children.FreeArray();
}

void CWBItem::ApplyStyleDeclarations( const CString &String )
{
  if ( !App ) return;
  App->StyleManager.ApplyStylesFromDeclarations( this, String );
}

WBITEMSTATE CWBItem::GetState()
{
  WBITEMSTATE i = WB_STATE_NORMAL;
  if ( Disabled )
    i = WB_STATE_DISABLED;
  if ( MouseOver() && !Disabled )
    i = WB_STATE_HOVER;

  if ( i == WB_STATE_HOVER && App->GetMouseCaptureItem() )
    if ( App->GetMouseCaptureItem() != this && !App->GetMouseCaptureItem()->AllowMouseHighlightWhileCaptureItem() ) i = WB_STATE_NORMAL;

  if ( InFocus() )
    i = Disabled ? WB_STATE_DISABLED_ACTIVE : WB_STATE_ACTIVE;
  return i;
}

void CWBItem::SetBorderSizes( TS8 Left, TS8 Top, TS8 Right, TS8 Bottom )
{
  CSSProperties.BorderSizes = CRect( Left, Top, Right, Bottom );
}

void CWBItem::SetDisplayProperty( WBITEMSTATE s, WBITEMVISUALCOMPONENT v, TS32 value )
{
  CSSProperties.DisplayDescriptor.SetValue( s, v, value );
}

CWBFont *CWBItem::GetFont( WBITEMSTATE State )
{
  return CSSProperties.GetFont( App, State );
}

void CWBItem::ApplyOpacity( CWBDrawAPI *API )
{
  CColor o = CSSProperties.DisplayDescriptor.GetColor( GetState(), WB_ITEM_OPACITY );
  API->SetOpacity( (TU8)( o.A()*OpacityMultiplier ) );
}

CWBPositionDescriptor & CWBItem::GetPositionDescriptor()
{
  return CSSProperties.PositionDescriptor;
}

CSize CWBItem::GetClientWindowSizeDifference()
{
  CRect w = GetWindowRect();
  CRect c = GetClientRect();

  return CSize( w.Width() - c.Width(), w.Height() - c.Height() );
}

void CWBItem::Enable( TBOOL Enabled )
{
  Disabled = !Enabled;
}

TBOOL CWBItem::IsEnabled()
{
  return !Disabled;
}

void CWBItem::SetChildInFocus( CWBItem *i )
{
  if ( !i )
  {
    ChildInFocus = NULL;
    return;
  }
  if ( i->Parent == this ) ChildInFocus = i;
}

TBOOL CWBItem::ParseRGBA( CString description, CColor &output )
{
  CStringArray Params = description.Explode( _T( "," ) );
  if ( Params.NumItems() < 3 || Params.NumItems() > 4 ) return false;

  TS32 result = 0;
  TS32 c[ 3 ];
  result += Params[ 0 ].Scan( _T( "rgba(%d" ), &c[ 0 ] );
  result += Params[ 1 ].Scan( _T( "%d" ), &c[ 1 ] );
  result += Params[ 2 ].Scan( _T( "%d" ), &c[ 2 ] );

  if ( result < 3 )	return false;

  TF32 a = 1;

  if ( Params.NumItems() == 4 )
    if ( Params[ 3 ].Scan( _T( "%f" ), &a ) != 1 ) return false;

  TS32 Colors[ 3 ];
  for ( TS32 y = 0; y < 3; y++ )
    Colors[ y ] = max( 0, min( 255, c[ y ] ) );
  TS32 Alpha = (TS32)( max( 0, min( 1, a ) ) * 255 );

  output = CColor::FromARGB( ( Alpha << 24 ) + ( Colors[ 0 ] << 16 ) + ( Colors[ 1 ] << 8 ) + Colors[ 2 ] );
  return true;
}

void CWBItem::FontStyleApplicator( CWBCSSPropertyBatch &desc, CStringArray &pseudo, CString &name )
{
  if ( pseudo.NumItems() <= 1 )
  {
    desc.Fonts.Flush();
    desc.Fonts[ WB_STATE_NORMAL ] = name;
  }
  else
  {
    for ( TS32 y = 1; y < pseudo.NumItems(); y++ )
    {
      CString p = pseudo[ y ].Trimmed();
      if ( p == _T( "active" ) )
      {
        desc.Fonts[ WB_STATE_ACTIVE ] = name;
        continue;
      }
      if ( p == _T( "hover" ) )
      {
        desc.Fonts[ WB_STATE_HOVER ] = name;
        continue;
      }
      if ( p == _T( "disabled" ) )
      {
        desc.Fonts[ WB_STATE_DISABLED ] = name;
        continue;
      }
      if ( p == _T( "disabled-active" ) )
      {
        desc.Fonts[ WB_STATE_DISABLED_ACTIVE ] = name;
        continue;
      }
      if ( p == _T( "normal" ) )
      {
        desc.Fonts[ WB_STATE_NORMAL ] = name;
        continue;
      }
    }
  }
}

TBOOL CWBItem::ScanPXValue( CString &Value, TS32 &Result, CString &PropName )
{
  Result = 0;
  if ( Value.Scan( _T( "%dpx" ), &Result ) != 1 )
  {
    LOG_WARN( "[guiitem] Item style error: invalid %s value '%s' (px)", PropName.GetPointer(), Value.GetPointer() );
    return false;
  }
  return true;
}

TBOOL CWBItem::ScanSkinValue( CString &Value, WBSKINELEMENTID &Result, CString &PropName )
{
  if ( Value.Find( _T( "skin(" ) ) == 0 )
  {
    TS32 i = Value.Find( _T( ")" ) );
    if ( i > 0 )
    {
      Value.GetPointer()[ i ] = 0;
      Result = App->GetSkin()->GetElementID( CString( Value.GetPointer() + 5 ) );
      if ( Result == 0xffffffff )
      {
        LOG_WARN( "[gui] Skin element not found: %s", Value.GetPointer() + 5 );
        return false;
      }

      return true;
    }
  }
  return false;
}

void CWBItem::SetFont( WBITEMSTATE State, CString &Font )
{
  CSSProperties.Fonts[ State ] = Font;
}

void CWBItem::ContentChanged()
{
  if ( !CSSProperties.PositionDescriptor.IsAutoResizer() ) return;

  CSize ParentSize = GetWindowRect().Size();
  CSize ClientSize = CSSProperties.PositionDescriptor.GetPadding( GetWindowRect().Size(), CSSProperties.BorderSizes ).Size();

  StoredContentSize = GetContentSize() + ParentSize - ClientSize;
  CWBMessage m;
  BuildPositionMessage( CRect( GetPosition().TopLeft(), GetPosition().TopLeft() + StoredContentSize ), m );
  m.Resized = true;
  m.Moved = true;
  App->SendMessage( m );
}

void CWBItem::ChangeContentOffset( CPoint ContentOff )
{
  if ( ContentOff == ContentOffset ) return;
  ContentOffset = ContentOff;
  for ( TU32 x = 0; x < NumChildren(); x++ )
    GetChild( x )->UpdateScreenRect();
}

void CWBItem::ChangeContentOffsetX( TS32 OffsetX )
{
  App->SendMessage( CWBMessage( App, WBM_CONTENTOFFSETCHANGE, GetGuid(), OffsetX, ContentOffset.y ) );
}

void CWBItem::ChangeContentOffsetY( TS32 OffsetY )
{
  App->SendMessage( CWBMessage( App, WBM_CONTENTOFFSETCHANGE, GetGuid(), ContentOffset.x, OffsetY ) );
}

TBOOL CWBItem::ScrollbarsEnabled()
{
  return HScrollbar.Enabled || VScrollbar.Enabled;
}

void CWBItem::SetTreeOpacityMultiplier( TF32 OpacityMul )
{
  OpacityMultiplier = OpacityMul;
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    Children[ x ]->SetTreeOpacityMultiplier( OpacityMul );
}

TF32 CWBItem::GetTreeOpacityMultiplier()
{
  return OpacityMultiplier;
}

void CWBItem::ReapplyStyles()
{
  App->StyleManager.ApplyStyles( this );
  CWBMessage m;
  BuildPositionMessage( GetPosition(), m );
  m.Resized = true;
  MessageProc( m );
}

void CWBItem::SetForcedMouseTransparency( TBOOL transparent )
{
  ForceMouseTransparent = transparent;
}

TBOOL CWBItem::MarkedForDeletion()
{
  return App->Trash.Find( this ) >= 0;
}
