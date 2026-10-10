#include "BasePCH.h"
#include "Application.h"
#include "ContextMenu.h"
#include "Button.h"

// entries are spaced out and toggles get drawn check boxes, like the TacO settings window
static const TS32 EntryPaddingY = 6;
static const TS32 EntryMarginX = 6;

static TS32 GetCheckBoxSize( CWBFont *Font )
{
  return max( 10, Font->GetLineHeight() * 5 / 8 );
}

TS32 CWBContextMenu::GetEntryHeight( CWBFont *Font )
{
  return Font->GetLineHeight() + entryPadding;
}

// the check boxes get a column of their own when any entry has one, so the texts stay aligned
TS32 CWBContextMenu::GetCheckColumnWidth( CWBFont *Font )
{
  for ( TS32 x = 0; x < Items.NumItems(); x++ )
    if ( !Items[ x ]->Separator && Items[ x ]->Check >= 0 )
      return GetCheckBoxSize( Font ) + 8;
  return 0;
}

void CWBContextMenu::OnDraw( CWBDrawAPI *API )
{
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  DrawBackground( API, WB_STATE_NORMAL );

  CRect Client = GetClientRect();
  CRect padding = CSSProperties.PositionDescriptor.GetPadding( Client.Size(), CRect( 0, 0, 0, 0 ) );

  CPoint Offset = CPoint( padding.x1, padding.y1 );
  TS32 checkColumn = Font ? GetCheckColumnWidth( Font ) : 0;

  for ( TS32 x = 0; x < Items.NumItems(); x++ )
  {
    if ( Items[ x ]->Separator )
    {
      int arbitraryValue = 200;

      CRect padding = SeparatorElements.PositionDescriptor.GetPadding( CSize( GetWindowRect().Width(), arbitraryValue ), CRect( 0, 0, 0, 0 ) );
      Offset.y += padding.y1;

      int height = SeparatorElements.PositionDescriptor.GetHeight( CSize( 0, 0 ), CSize( 0, 0 ) );

      CRect separatorRect = CRect( GetWindowRect().x1 + padding.x1, Offset.y, GetWindowRect().x1 + padding.x2, Offset.y + height );

      WBSKINELEMENTID id = SeparatorElements.DisplayDescriptor.GetSkin( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDIMAGE );

      //draw separator
      if ( id != 0xffffffff )
      {
        App->GetSkin()->RenderElement( API, id, separatorRect );
      }
      else
      {
        API->DrawRect( separatorRect, CSSProperties.DisplayDescriptor.GetColor( i, WB_ITEM_BORDERCOLOR ) );
      }

      Offset.y += height;
      Offset.y += arbitraryValue - padding.y2;
    }
    else
      if ( Font )
      {
        //draw entry
        CRect EntryPos = GetItemRect( x );
        EntryPos.x1 += padding.x1;
        EntryPos.x2 -= Client.Width() - padding.x2;

        CColor textColor = CSSProperties.DisplayDescriptor.GetColor( WB_STATE_NORMAL, WB_ITEM_FONTCOLOR );

        if ( Items[ x ]->Highlighted )
          textColor = CSSProperties.DisplayDescriptor.GetColor( WB_STATE_ACTIVE, WB_ITEM_FONTCOLOR );

        //highlight background
        if ( MouseOver() && EntryPos.Contains( ScreenToClient( App->GetMousePos() ) ) )
        {
          API->DrawRect( EntryPos, CSSProperties.DisplayDescriptor.GetColor( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR ) );
          textColor = CSSProperties.DisplayDescriptor.GetColor( WB_STATE_HOVER, WB_ITEM_FONTCOLOR );
        }

        TS32 entryHeight = GetEntryHeight( Font );
        CPoint textPos = Offset + CPoint( EntryMarginX + checkColumn, entryPadding / 2 );

        if ( Items[ x ]->Check >= 0 )
        {
          TS32 size = GetCheckBoxSize( Font );
          CPoint boxPos = Offset + CPoint( EntryMarginX, ( entryHeight - size ) / 2 );
          CRect box( boxPos, boxPos + CPoint( size, size ) );
          API->DrawRect( box, CColor::FromARGB( 0x80000000 ) );
          API->DrawRectBorder( box, CSSProperties.DisplayDescriptor.GetColor( WB_STATE_NORMAL, WB_ITEM_FONTCOLOR ) );
          if ( Items[ x ]->Check )
            API->DrawRect( CRect( box.x1 + 2, box.y1 + 2, box.x2 - 2, box.y2 - 2 ), CSSProperties.DisplayDescriptor.GetColor( WB_STATE_ACTIVE, WB_ITEM_FONTCOLOR ) );
        }

        TS32 width = Font->Write( API, Items[ x ]->Text, textPos, textColor, TextTransform );

        if ( Items[ x ]->Children.NumItems() ) //draw sub arrow
        {
          if ( CSSProperties.DisplayDescriptor.GetSkin( i, WB_ITEM_SUBSKIN ) != 0xffffffff )
          {

          }
          else
          {
            TS32 wi = Font->GetWidth( _T( ">" ) );
            Font->Write( API, _T( ">" ), CPoint( GetWindowRect().x2 - wi - EntryMarginX - CSSProperties.BorderSizes.x2 - ( Client.x2 - padding.x2 ), textPos.y ), textColor );
          }
        }

        Offset.y += entryHeight;
      }
  }

  DrawBorder( API );
}

CWBContextMenu::CWBContextMenu() : CWBItem()
{
  Target = NULL;
  Pushed = false;
}

CWBContextMenu::CWBContextMenu( CWBItem *Parent, const CRect &Pos, WBGUID trg ) : CWBItem()
{
  CWBContextMenu::Initialize( Parent, Pos, trg );
}

CWBContextMenu::~CWBContextMenu()
{
  if ( ParentMenu ) ParentMenu->SubMenu = NULL;
  Items.FreeArray();
  SAFEDELETE( SubMenu );
}

void CWBContextMenu::ResizeToContentSize()
{
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );

  CSize ContentSize = CSize( 0, 0 );

  TBOOL NeedsSubArrow = false;
  for ( TS32 x = 0; x < Items.NumItems(); x++ )
    if ( Items[ x ]->Children.NumItems() )
      NeedsSubArrow = true;

  int arbitraryValue = 0;
  CRect padding = SeparatorElements.PositionDescriptor.GetPadding( CSize( 0, arbitraryValue ), CRect( 0, 0, 0, 0 ) );
  int height = SeparatorElements.PositionDescriptor.GetHeight( CSize( 0, 0 ), CSize( 0, 0 ) );
  int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  CRect Client = GetClientRect();
  padding = CSSProperties.PositionDescriptor.GetPadding( Client.Size(), CRect( 0, 0, 0, 0 ) );

  ContentSize.y = padding.y1;
  TS32 checkColumn = Font ? GetCheckColumnWidth( Font ) : 0;
  TS32 entries = 0;
  entryPadding = EntryPaddingY;

  for ( TS32 x = 0; x < Items.NumItems(); x++ )
  {
    if ( Items[ x ]->Separator )
    {
      ContentSize.y += separatorHeight;
    }
    else
      if ( Font )
      {
        entries++;
        TS32 arrowPadding = 0;

        if ( Items[ x ]->Children.NumItems() ) //account for sub arrow
        {
          if ( CSSProperties.DisplayDescriptor.GetSkin( i, WB_ITEM_SUBSKIN ) != 0xffffffff )
          {

          }
          else
          {
            arrowPadding = Font->GetWidth( _T( ">" ) ) + EntryMarginX;
          }
        }

        ContentSize.x = max( ContentSize.x, padding.x1 + Client.x2 - padding.x2 + EntryMarginX * 2 + checkColumn + arrowPadding + Font->GetWidth( Items[ x ]->Text, true, TextTransform ) );
        ContentSize.y += GetEntryHeight( Font );
      }
  }

  ContentSize.y += Client.Size().y - padding.y2;

  CRect r = GetPosition();
  CSize Size = ContentSize + GetClientWindowSizeDifference();

  // long menus go back to compact entries so every entry stays on the screen
  if ( GetParent() && Size.y > GetParent()->GetClientRect().Height() )
  {
    Size.y -= entries * entryPadding;
    entryPadding = 0;
  }

  SetPosition( CRect( r.TopLeft(), r.TopLeft() + Size ) );
}

TBOOL CWBContextMenu::Initialize( CWBItem *Parent, const CRect &Position, WBGUID trg )
{
  ParentMenu = NULL;
  SubMenu = NULL;
  Pushed = false;
  Target = trg;
  if ( !CWBItem::Initialize( Parent, Position ) ) return false;

  SetClientPadding( 2, 0, 2, 0 );
  SetBorderSizes( 1, 1, 1, 1 );

  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1b1b1c ) );
  CSSProperties.DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff434346 ) );
  //CSSProperties.PositionDescriptor.SetValue(WB_PADDING_LEFT, 10, 1);
  //CSSProperties.PositionDescriptor.SetValue(WB_PADDING_RIGHT, 30, 1);


  SeparatorElements.PositionDescriptor.SetValue( WB_PADDING_LEFT, 0, EntryMarginX );
  SeparatorElements.PositionDescriptor.SetValue( WB_PADDING_TOP, 0, 3 );
  SeparatorElements.PositionDescriptor.SetValue( WB_HEIGHT, 0, 1 );
  SeparatorElements.PositionDescriptor.SetValue( WB_PADDING_RIGHT, 0, EntryMarginX );
  SeparatorElements.PositionDescriptor.SetValue( WB_PADDING_BOTTOM, 0, 3 );

  App->SetCapture( this );
  return true;
}

CWBContextItem *CWBContextMenu::AddItem( const TCHAR *Text, TS32 ID, TBOOL Highlighted, TBOOL closesContext, TS32 Check )
{
  return AddItem( CString( Text ), ID, Highlighted, closesContext, Check );
}

CWBContextItem * CWBContextMenu::AddItem( const CString &Text, TS32 ID, TBOOL Highlighted, TBOOL closesContext, TS32 Check )
{
  CWBContextItem *Item = new CWBContextItem();
  Item->Text = Text;
  Item->ReturnID = ID;
  Item->Highlighted = Highlighted;
  Item->closesContext = closesContext;
  Item->Check = Check;
  Items += Item;
  ResizeToContentSize();

  return Item;
}

void CWBContextMenu::AddSeparator()
{
  Items += new CWBContextItem();
  Items.Last()->Separator = true;
}

void CWBContextMenu::FlushItems()
{
  // items of open submenus are copies pointing back at the items freed here
  for ( CWBContextMenu* menu = SubMenu; menu; menu = menu->SubMenu )
    for ( TS32 x = 0; x < menu->Items.NumItems(); x++ )
      menu->Items[ x ]->ClearCopyLinks();

  Items.FreeArray();
}

TBOOL CWBContextMenu::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {
  case WBM_REPOSITION:
    if ( Message.GetTarget() == GetGuid() )
    {
      //push position inside of parent
      CRect p = Message.Rectangle;
      CRect r = GetParent()->GetClientRect();
      if ( p.x1 < 0 ) p += CPoint( -p.x1, 0 );
      if ( p.y1 < 0 ) p += CPoint( 0, -p.y1 );
      if ( p.x2 > r.x2 ) p -= CPoint( p.x2 - r.x2, 0 );
      if ( p.y2 > r.y2 ) p -= CPoint( 0, p.y2 - r.y2 );
      Message.Rectangle = p;
    }
    break;
  case WBM_MOUSEMOVE:
    if ( MouseOver() )
    {
      for ( TS32 x = 0; x < Items.NumItems(); x++ )
        if ( !Items[ x ]->Separator )
        {
          CRect EntryPos = GetItemRect( x );
          if ( EntryPos.Contains( ScreenToClient( App->GetMousePos() ) ) )
          {
            SpawnSubMenu( x );
            return true;
          }
        }
    }
    if ( SubMenu ) SubMenu->MessageProc( Message );
    return true;

  case WBM_RIGHTBUTTONDOWN:
  case WBM_LEFTBUTTONDOWN:
  case WBM_MIDDLEBUTTONDOWN:

    if ( !MouseInContextHierarchy() && !MarkedForDeletion() ) //need to close the context menu and resend the click
    {
      MarkForDeletion();
      MarkParentForDeletion();
      App->ReleaseCapture();
      App->SendMessage( Message ); //resend message
      return true;
    }

    Pushed = true;
    App->SetCapture( GetContextRoot() );
    if ( !MouseOver() )
      if ( SubMenu ) SubMenu->MessageProc( Message );
    return true;

  case WBM_RIGHTBUTTONUP:
  case WBM_LEFTBUTTONUP:

    if ( !Pushed ) return true; //original click doesn't count

    //check if any of the items are clicked on
    for ( TS32 x = 0; x < Items.NumItems(); x++ )
      if ( !Items[ x ]->Separator )
      {
        CRect EntryPos = GetItemRect( x );
        if ( MouseOver() && EntryPos.Contains( ScreenToClient( App->GetMousePos() ) ) )
        {
          App->SendMessage( CWBMessage( App, WBM_CONTEXTMESSAGE, Target, Items[ x ]->ReturnID, GetGuid() ) );
          if ( Items[ x ]->closesContext )
          {
            MarkForDeletion();
            MarkParentForDeletion();
            App->ReleaseCapture();
          }
          else
            App->SendMessage( CWBMessage( App, WBM_REBUILDCONTEXTITEM, Target, Items[ x ]->ReturnID, GetGuid() ) );

          return true;
        }
      }

    if ( MouseInContextHierarchy() )
      if ( SubMenu ) SubMenu->MessageProc( Message );

    //check for off item mouse releases
    if ( !MouseInContextHierarchy() && Pushed )
    {
      MarkForDeletion();
      MarkParentForDeletion();
      App->ReleaseCapture();
    }

    return true;
  }

  return CWBItem::MessageProc( Message );
}

void CWBContextMenu::SpawnSubMenu( TS32 itemidx )
{
  if ( itemidx < 0 || itemidx >= Items.NumItems() ) 
    return;
  if ( SubMenu && SubMenu->spawnedFromIdx == itemidx )
    return;

  SAFEDELETE( SubMenu );
  //if (SubMenu) SubMenu->MarkForDeletion();
  if ( Items[ itemidx ]->Children.NumItems() <= 0 )
    return;

  CRect p = GetItemRect( itemidx ) + GetPosition().TopLeft();
  CRect w = GetPosition();

  CRect newpos = CRect( w.x2 - 1, p.y1, w.x2 + 10, p.y1 + 10 );

  SubMenu = new CWBContextMenu( GetParent(), newpos, Target );
  SubMenu->spawnedFromIdx = itemidx;
  App->ApplyStyle( SubMenu );
  SubMenu->ParentMenu = this;
  App->SetCapture( GetContextRoot() ); //message control must stay with the root item

  for ( TS32 x = 0; x < Items[ itemidx ]->Children.NumItems(); x++ )
  {
    CWBContextItem *olditm = Items[ itemidx ]->Children[ x ];
    CWBContextItem *newitm = SubMenu->AddItem( olditm->Text.GetPointer(), olditm->ReturnID );
    newitm->CopyOf = olditm;
    newitm->Separator = olditm->Separator;
    newitm->Highlighted = olditm->Highlighted;
    newitm->closesContext = olditm->closesContext;
    newitm->Check = olditm->Check;
    newitm->CopyChildrenFrom( olditm );
  }

  SubMenu->ResizeToContentSize();

}

CRect CWBContextMenu::GetItemRect( TS32 idx )
{
  CPoint Offset = CPoint( 0, 0 );

  int arbitraryValue = 0;
  CRect padding = SeparatorElements.PositionDescriptor.GetPadding( CSize( 0, arbitraryValue ), CRect( 0, 0, 0, 0 ) );
  int height = SeparatorElements.PositionDescriptor.GetHeight( CSize( 0, 0 ), CSize( 0, 0 ) );
  int separatorHeight = padding.y1 + height + arbitraryValue - padding.y2;

  CRect Client = GetClientRect();
  padding = CSSProperties.PositionDescriptor.GetPadding( Client.Size(), CRect( 0, 0, 0, 0 ) );

  Offset.y = padding.y1;

  CWBFont *Font = GetFont( WB_STATE_HOVER );
  for ( TS32 x = 0; x < Items.NumItems(); x++ )
  {
    if ( Items[ x ]->Separator )
    {
      Offset.y += separatorHeight;
    }
    else
      if ( Font )
      {
        CRect EntryPos = CRect( GetWindowRect().x1, Offset.y, GetWindowRect().x2, Offset.y + GetEntryHeight( Font ) );
        if ( x == idx ) return EntryPos;
        Offset.y += GetEntryHeight( Font );
      }
  }

  return CRect( 0, 0, 0, 0 );
}

void CWBContextMenu::MarkParentForDeletion()
{
  if ( ParentMenu )
  {
    ParentMenu->MarkForDeletion();
    ParentMenu->MarkParentForDeletion();
  }
}

TBOOL CWBContextMenu::MouseInContextHierarchy()
{
  if ( !SubMenu ) return MouseOver();
  return MouseOver() || SubMenu->MouseInContextHierarchy();
}

CWBContextMenu * CWBContextMenu::GetContextRoot()
{
  if ( !ParentMenu ) return this;
  return ParentMenu->GetContextRoot();
}

CWBContextItem::CWBContextItem()
{
  Text = _T( "" );
  ReturnID = 0;
  Separator = false;
}

CWBContextItem::~CWBContextItem()
{
  Children.FreeArray();
}

void CWBContextItem::CopyChildrenFrom( CWBContextItem *itm )
{
  for ( TS32 x = 0; x < itm->Children.NumItems(); x++ )
  {
    CWBContextItem *olditm = itm->Children[ x ];
    CWBContextItem *newitm = AddItem( olditm->Text.GetPointer(), olditm->ReturnID );
    newitm->Separator = olditm->Separator;
    newitm->Highlighted = olditm->Highlighted;
    newitm->closesContext = olditm->closesContext;
    newitm->Check = olditm->Check;
    newitm->CopyChildrenFrom( olditm );
    newitm->CopyOf = olditm;
  }
}

CWBContextItem * CWBContextItem::AddItem( const TCHAR *Text, TS32 ID, TBOOL Highlighted, TBOOL closesContext, TS32 Check )
{
  CWBContextItem *Item = new CWBContextItem();
  Item->Text = Text;
  Item->ReturnID = ID;
  Item->Highlighted = Highlighted;
  Item->closesContext = closesContext;
  Item->Check = Check;
  Children += Item;
  return Item;
}

CWBContextItem * CWBContextItem::AddItem( const CString& Text, TS32 ID, TBOOL Highlighted, TBOOL closesContext, TS32 Check )
{
  return AddItem( Text.GetPointer(), ID, Highlighted, closesContext, Check );
}

void CWBContextItem::AddSeparator()
{
  CWBContextItem *Item = new CWBContextItem();
  Children += Item;
  Children.Last()->Separator = true;
}

// a submenu shows copies of its parent item's children and is rebuilt from the originals every time it opens again,
// so changes have to reach the originals too or reopening the submenu shows the old state
void CWBContextItem::SetText( const CString& text )
{
  Text = text;
  if ( CopyOf )
    CopyOf->SetText( text );
}

void CWBContextItem::SetHighlight( TBOOL highlighted )
{
  Highlighted = highlighted;
  if ( CopyOf )
    CopyOf->SetHighlight( highlighted );
}

void CWBContextItem::SetCheck( TS32 check )
{
  Check = check;
  if ( CopyOf )
    CopyOf->SetCheck( check );
}

void CWBContextItem::ClearCopyLinks()
{
  CopyOf = nullptr;
  for ( TS32 x = 0; x < Children.NumItems(); x++ )
    Children[ x ]->ClearCopyLinks();
}

TBOOL CWBContextMenu::ApplyStyle( CString & prop, CString & value, CStringArray &pseudo )
{
  TBOOL ElementTarget = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "separator" ) )
    {
      ElementTarget = true;
      break;
    }
  }

  //apply font styling to list item anyway
  if ( !ElementTarget ) InterpretFontString( CSSProperties, prop, value, pseudo );
  if ( !ElementTarget ) return CWBItem::ApplyStyle( prop, value, pseudo );

  TBOOL Handled = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "separator" ) )
    {
      Handled |= SeparatorElements.ApplyStyle( this, prop, value, pseudo );
      continue;
    }
  }

  return Handled;
}

CWBContextItem * CWBContextMenu::GetItem( TS32 ID )
{
  for ( TS32 x = 0; x < Items.NumItems(); x++ )
    if ( Items[ x ]->ReturnID == ID )
      return Items[ x ];
  return nullptr;
}

CWBContextMenu* CWBContextMenu::GetParentMenu()
{
  return ParentMenu;
}
