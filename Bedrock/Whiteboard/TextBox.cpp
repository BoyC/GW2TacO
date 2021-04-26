#include "BasePCH.h"
#include "TextBox.h"

INLINE void CWBTextBox::DrawCursor( CWBDrawAPI *API, CPoint &p )
{
  WBITEMSTATE s = GetState();
  if ( !( ( ( globalTimer.GetTime() - CursorBlinkStartTime ) / 500 ) % 2 ) ) API->DrawRect( CRect( p + CPoint( 0, 1 ), p + CPoint( 1, GetFont( s )->GetLineHeight() - 1 ) ), 0xffffffff );
}

void CWBTextBox::OnDraw( CWBDrawAPI *API )
{
  //background
  DrawBackground( API );
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
  TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;

  //API->DrawRect(GetWindowRect(),rgbBackground);

  CPoint Pos = CPoint( 0, 0 );
  CPoint Offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) + GetTextStartOffset();

  TBOOL Focus = InFocus() && !GetChildInFocus(); //speedup

  API->SetCropToClient( this );

  CColor Color = CSSProperties.DisplayDescriptor.GetColor( i, WB_ITEM_FONTCOLOR );
  CColor BackgroundColor = CColor::FromARGB( 0x00000000 );
  CColor SelectionColor = Lerp( Selection.DisplayDescriptor.GetColor( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR ),
                                Selection.DisplayDescriptor.GetColor( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR ), min( 1.0f, ( globalTimer.GetTime() - HiglightStartTime ) / 300.0f ) );
  if ( !InFocus() ) SelectionColor = Selection.DisplayDescriptor.GetColor( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR );

  for ( TS32 x = 0; x < (TS32)Text.Length(); x++ )
  {
    if ( ColoredText() ) Color = GetTextColor( x, Color ); //color coding, to be implemented in child classes

    TCHAR Char = Flags&WB_TEXTBOX_PASSWORD ? PasswordStar : Font->ApplyTextTransform( (_TUCHAR*)Text.GetPointer(), (_TUCHAR*)Text.GetPointer() + x, TextTransform );
    TS32 Width = Font->GetWidth( Char );

    if ( Char == '\t' )
      Width = ( (TS32)( Pos.x + TabWidth ) / TabWidth )*TabWidth - Pos.x;

    //draw selection
    if ( !( Flags&WB_TEXTBOX_NOSELECTION ) && x >= SelectionStart && x < SelectionEnd )
    {
      CRect Display = CRect( Pos, CPoint( Pos.x + Width, Pos.y + Font->GetLineHeight() ) ) + Offset;
      if ( Display.Intersects( GetClientRect() ) )
        API->DrawRect( Display, SelectionColor );
    }

    //draw background highlight

    if ( GetTextBackground( x, BackgroundColor ) )
    {
      CRect Display = CRect( Pos, CPoint( Pos.x + Width, Pos.y + Font->GetLineHeight() ) ) + Offset;
      if ( Display.Intersects( GetClientRect() ) )
        API->DrawRect( Display, BackgroundColor );
    }

    CPoint CPos = Pos;

    if ( Char == '\n' || Char == '\t' ) //special characters
    {
      //line feed
      if ( Char == '\n' && !( Flags&WB_TEXTBOX_SINGLELINE ) )
      {
        Pos.x = 0;
        Pos.y += Font->GetLineHeight();
      }
      if ( Char == '\t' )
      {
        Pos.x = ( (TS32)( Pos.x + TabWidth ) / TabWidth )*TabWidth;
      }
    }
    else
    {
      //draw next character
      CRect Display = CRect( Pos, CPoint( Pos.x + Width, Pos.y + Font->GetLineHeight() ) ) + Offset;
      if ( Display.Intersects( GetClientRect() ) )
        Font->WriteChar( API, Char, Pos + Offset, Color );
      Pos.x += Width;
    }

    //draw cursor if needed
    if ( Focus && CursorPos == x )
      DrawCursor( API, CPos + Offset );
  }

  //cursor at the end of text
  if ( Focus && CursorPos == Text.Length() )
    DrawCursor( API, Pos + Offset );

  DrawBorder( API );

  if ( App->GetMouseCaptureItem() && App->GetMouseCaptureItem() != this ) return;
  if ( App->GetMouseItem() == this && ClientToScreen( GetClientRect() ).Contains( App->GetMousePos() ) )
    App->SelectMouseCursor( CM_TEXT );
}

CWBTextBox::CWBTextBox() : CWBItem()
{
}

CWBTextBox::CWBTextBox( CWBItem *Parent, const CRect &Pos, TS32 flags, const TCHAR *Txt ) : CWBItem()
{
  Initialize( Parent, Pos, flags, Txt );
}

CWBTextBox::~CWBTextBox()
{
  History.FreeArray();
}

TBOOL CWBTextBox::Initialize( CWBItem *Parent, const CRect &Position, TS32 flags, const TCHAR *Txt )
{
  Flags = flags;
  Selection.DisplayDescriptor.SetValue( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, 0 );
  Selection.DisplayDescriptor.SetValue( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff2b537d ) );
  Selection.DisplayDescriptor.SetValue( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff3399ff ) );

  SetDisplayProperty( WB_STATE_NORMAL, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );
  SetDisplayProperty( WB_STATE_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );
  SetDisplayProperty( WB_STATE_HOVER, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );
  SetDisplayProperty( WB_STATE_DISABLED, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );
  SetDisplayProperty( WB_STATE_DISABLED_ACTIVE, WB_ITEM_BACKGROUNDCOLOR, CColor::FromARGB( 0xff1e1e1e ) );

  CursorBlinkStartTime = globalTimer.GetTime();
  HiglightStartTime = 0;
  PasswordStar = _T( '*' );

  CursorPos = 0;
  SelectionStart = 0;
  SelectionEnd = 0;
  SelectionOrigin = 0;
  DesiredCursorPosXinPixels = 0;

  HistoryPosition = 0;

  if ( !CWBItem::Initialize( Parent, Position ) ) return false;

  EnableHScrollbar( true, true );
  EnableVScrollbar( true, true );

  SetTextInternal( Txt, false, true );
  return true;
}

void CWBTextBox::SetCursorPos( TS32 pos, TBOOL Selecting )
{
  CWBFont *Font = GetFont( GetState() );
  TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;

  CursorBlinkStartTime = globalTimer.GetTime();

  if ( !Selecting )
    SelectionStart = SelectionEnd = SelectionOrigin = CursorPos = pos;
  else
  {
    if ( SelectionStart == SelectionEnd ) SelectionOrigin = SelectionStart;
    CursorPos = pos;
    SelectionStart = min( CursorPos, SelectionOrigin );
    SelectionEnd = max( CursorPos, SelectionOrigin );
  }

  //adjust scrollbars so cursor is visible

  //determine cursor position in pixels
  CPoint CPos;
  CPos.y = GetCursorY()*Font->GetLineHeight();
  CPos.x = 0;
  TS32 Cx = GetCursorX();

  for ( TS32 x = 0; x < Cx; x++ )
  {
    TCHAR Char = Text[ CursorPos - Cx + x ];
    TS32 Width = Font->GetWidth( Char );

    if ( Char == '\t' )
      Width = ( (TS32)( CPos.x + TabWidth ) / TabWidth )*TabWidth - CPos.x;

    CPos.x += Width;
  }

  TS32 CurrCharWidth = Font->GetWidth( _T( ' ' ) );
  if ( CursorPos != Text.Length() ) CurrCharWidth = Font->GetWidth( Text[ CursorPos ] );

  OnCursorPosChange( CursorPos );

  if ( Flags & WB_TEXTBOX_SINGLELINE ) return;

  CRect VisibleRect = GetClientRect() + CPoint( GetHScrollbarPos(), GetVScrollbarPos() );
  if ( VisibleRect.Contains( CPos ) && VisibleRect.Contains( CPos + CPoint( CurrCharWidth, Font->GetLineHeight() ) ) ) return; //no need to adjust

  if ( CPos.x < VisibleRect.x1 ) SetHScrollbarPos( GetHScrollbarPos() - ( VisibleRect.x1 - CPos.x ) );
  if ( CPos.x + CurrCharWidth > VisibleRect.x2 ) SetHScrollbarPos( GetHScrollbarPos() + ( CPos.x + CurrCharWidth - VisibleRect.x2 ) );

  if ( CPos.y < VisibleRect.y1 ) SetVScrollbarPos( GetVScrollbarPos() - ( VisibleRect.y1 - CPos.y ) );
  if ( CPos.y + Font->GetLineHeight() > VisibleRect.y2 ) SetVScrollbarPos( GetVScrollbarPos() + ( CPos.y + Font->GetLineHeight() - VisibleRect.y2 ) );
}

void CWBTextBox::SetCursorPosXpxY( TS32 x, TS32 y, TBOOL Selecting )
{
  TS32 p = 0;
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
  TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;

  if ( !( y < 0 || ( y == 0 && x < 0 ) ) )
  {
    TS32 yp = 0;
    while ( yp < y )
    {
      if ( p == Text.Length() )
      {
        SetCursorPos( p, Selecting );
        return;
      }
      if ( Text[ p ] == '\n' ) yp++;
      p++;
    }
  }

  TS32 pd = p;
  TS32 xp = 0;
  while ( xp < x )
  {
    if ( p == Text.Length() || Text[ p ] == '\n' )
    {
      //end of line
      SetCursorPos( p, Selecting );
      return;
    }

    TCHAR Char = Flags&WB_TEXTBOX_PASSWORD ? PasswordStar : Font->ApplyTextTransform( (_TUCHAR*)Text.GetPointer(), (_TUCHAR*)Text.GetPointer() + p, TextTransform );
    TS32 Width = Font->GetWidth( Char );

    if ( Char == '\t' )
      Width = ( (TS32)( xp + TabWidth ) / TabWidth )*TabWidth - xp;

    xp += Width;
    if ( xp > x ) break;
    p++;
  }

  //LOG(LOG_DEBUG,_T("[gui] Set Cursor Pos: %d (%d)"),xp,p-pd);

  SetCursorPos( p, Selecting );
}


void CWBTextBox::RemoveSelectedText()
{
  if ( SelectionStart != SelectionEnd )
  {
    RemoveText( SelectionStart, SelectionEnd - SelectionStart, SelectionStart );
  }
}

void CWBTextBox::Cut()
{
  if ( Flags&WB_TEXTBOX_NOSELECTION ) return;

  Copy();
  RemoveSelectedText();
  DesiredCursorPosXinPixels = GetCursorXinPixels();
  OnTextChange();
}

void CWBTextBox::Copy()
{
  if ( Flags&WB_TEXTBOX_NOSELECTION ) return;
  if ( Flags&WB_TEXTBOX_DISABLECOPY ) return;

  TS32 strt = SelectionStart;
  TS32 end = SelectionEnd;

  if ( SelectionStart == SelectionEnd )
  {
    //copy whole line...
  }

  if ( strt == end ) return;

  if ( OpenClipboard( (HWND)App->GetHandle() ) )
  {
    EmptyClipboard();

    TCHAR *winnewline = new TCHAR[ end - strt + 1 ];
    memcpy( winnewline, Text.GetPointer() + strt, sizeof( TCHAR )*( end - strt ) );
    winnewline[ end - strt ] = 0;
    CString out = CString( winnewline );
    out.ToWindowsNewline();
    delete[] winnewline;

    HGLOBAL clipbuffer = GlobalAlloc( GMEM_DDESHARE | GHND, ( out.Length() + 1 ) * sizeof( TCHAR ) );

    if ( clipbuffer )
    {
      TCHAR *buffer = (TCHAR*)GlobalLock( clipbuffer );
      if ( buffer )
      {
        memcpy( buffer, out.GetPointer(), sizeof( TCHAR )*out.Length() );
        buffer[ out.Length() ] = 0;
      }

      GlobalUnlock( clipbuffer );

#ifndef  UNICODE
      SetClipboardData( CF_TEXT, clipbuffer );
#else
      SetClipboardData( CF_UNICODETEXT, clipbuffer );
#endif
    }

    CloseClipboard();

    HiglightStartTime = globalTimer.GetTime();
  }
  else
    LOG( LOG_WARNING, _T( "[gui] Failed to open clipboard" ) );
}

void CWBTextBox::Paste()
{
  if ( OpenClipboard( (HWND)App->GetHandle() ) )
  {
    //#ifndef  UNICODE
    //		HANDLE Handle = GetClipboardData(CF_TEXT);
    //#else
    HANDLE Handle = GetClipboardData( CF_UNICODETEXT );
    //#endif

    WCHAR *buffer = (WCHAR*)GlobalLock( Handle );
    if ( buffer )
    {
      CString s;
#ifndef UNICODE
      TS32 len = wcslen( buffer );
      TCHAR *b2 = new TCHAR[ len + 1 ];
      memset( b2, 0, sizeof( TCHAR )*( len + 1 ) );
      for ( TS32 x = 0; x < len; x++ )
      {
        if ( buffer[ x ] >= 0 && buffer[ x ] <= 255 ) b2[ x ] = (TCHAR)buffer[ x ];
        else b2[ x ] = _T( '?' );
      }
      s = CString( b2 );
      SAFEDELETEA( b2 );
#else
      s = CString( buffer );
#endif
      s.ToUnixNewline();
      RemoveSelectedText();
      InsertText( CursorPos, s.GetPointer(), s.Length(), CursorPos + s.Length() );
      GlobalUnlock( Handle );
      DesiredCursorPosXinPixels = GetCursorXinPixels();
    }
    else
    {
      LOG( LOG_WARNING, _T( "[gui] Failed to retrieve clipboard data (%x)" ), GetLastError() );
    }

    CloseClipboard();
    OnTextChange();
    //LOG(LOG_INFO,_T("[gui] Text Size: %d"),Text.Length());
  }
  else
    LOG( LOG_WARNING, _T( "[gui] Failed to open clipboard" ) );

}

CWBTextBoxHistoryEntry *CWBTextBox::CreateNewHistoryEntry( bool Remove, int Start, int Length )
{
  for ( TS32 x = History.NumItems() - 1; x >= HistoryPosition; x-- ) History.FreeByIndex( x );

  CWBTextBoxHistoryEntry *entry = new CWBTextBoxHistoryEntry();
  entry->Remove = Remove;
  entry->StartPosition = Start;
  entry->Data = Text.Substring( Start, Length );
  History += entry;

  HistoryPosition = History.NumItems();

  return entry;
}


void CWBTextBox::Undo()
{
  if ( !HistoryPosition ) return;
  HistoryPosition--;

  CWBTextBoxHistoryEntry *e = History[ HistoryPosition ];

  if ( e->Remove )
    InsertText( e->StartPosition, e->Data.GetPointer(), e->Data.Length(), e->CursorPos_Before, false );
  else
    RemoveText( e->StartPosition, e->Data.Length(), e->CursorPos_Before, false );

  SelectionStart = e->SelectionStart_Before;
  SelectionEnd = e->SelectionEnd_Before;

  OnTextChange();
}

void CWBTextBox::Redo()
{
  if ( HistoryPosition >= History.NumItems() ) return;

  CWBTextBoxHistoryEntry *e = History[ HistoryPosition++ ];

  if ( e->Remove )
    RemoveText( e->StartPosition, e->Data.Length(), e->StartPosition, false );
  else
    InsertText( e->StartPosition, e->Data.GetPointer(), e->Data.Length(), e->StartPosition + e->Data.Length(), false );

  SelectionStart = CursorPos;
  SelectionEnd = CursorPos;

  OnTextChange();
}

TS32 CWBTextBox::GetCursorX()
{
  TS32 cnt = 0;
  for ( TS32 x = CursorPos - 1; x >= 0; x-- )
  {
    if ( Text[ x ] == '\n' ) return cnt;
    cnt++;
  }
  return cnt;
}

TS32 CWBTextBox::GetCursorY()
{
  TS32 cnt = 0;
  for ( TS32 x = CursorPos - 1; x >= 0; x-- )
    if ( Text[ x ] == '\n' ) cnt++;
  return cnt;
}

TS32 CWBTextBox::GetCursorXinPixels()
{
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
  TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;

  TS32 c = GetCursorX();
  TS32 Pixels = 0;
  for ( TS32 x = CursorPos - c; x < CursorPos; x++ )
  {
    TCHAR Char = Flags&WB_TEXTBOX_PASSWORD ? PasswordStar : Font->ApplyTextTransform( (_TUCHAR*)Text.GetPointer(), (_TUCHAR*)Text.GetPointer() + x, TextTransform );
    TS32 Width = Font->GetWidth( Char );

    if ( Char == '\t' )
      Width = ( (TS32)( Pixels + TabWidth ) / TabWidth )*TabWidth - Pixels;

    Pixels += Width;
  }

  //LOG(LOG_DEBUG,_T("[gui] Desired Cursor Pos: %d"),Pixels);

  return Pixels;
}


TS32 CWBTextBox::GetLineSize()
{
  TS32 ls = CursorPos - GetCursorX();
  TS32 cnt = 0;
  for ( TS32 x = ls; x < (TS32)Text.Length(); x++ )
  {
    if ( Text[ x ] == '\n' ) return cnt;
    cnt++;
  }
  return cnt;
}

TS32 CWBTextBox::GetLineLeadingWhiteSpaceSize()
{
  TS32 ls = CursorPos - GetCursorX();
  TS32 cnt = 0;
  for ( TS32 x = ls; x < (TS32)Text.Length(); x++ )
  {
    if ( Text[ x ] == '\n' || !_istspace( Text[ x ] ) ) return cnt;
    cnt++;
  }
  return cnt;
}

CPoint CWBTextBox::GetTextStartOffset()
{
  CPoint pos( 0, 0 );
  if ( !Flags&WB_TEXTBOX_SINGLELINE )
    return pos;

  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
  CPoint TextPos = Font->GetTextPosition( Text, GetClientRect(), CSSProperties.TextAlignX, CSSProperties.TextAlignY, TextTransform );

  if ( !IsHScrollbarEnabled() )
    pos.x = TextPos.x;
  if ( !IsVScrollbarEnabled() )
    pos.y = TextPos.y;

  return pos;
}

TS32 CWBTextBox::GetCursorPosMouse()
{
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
  TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;

  CPoint mp = ScreenToClient( App->GetMousePos() );
  CPoint Pos = CPoint( 0, 0 );
  CPoint Offset = -CPoint( GetHScrollbarPos(), GetVScrollbarPos() ) + GetTextStartOffset();
  CRect cr = GetClientRect();

  if ( mp.y < Pos.y + Offset.y ) return 0;

  for ( TS32 x = 0; x < (TS32)Text.Length(); x++ )
  {
    if ( ( Pos.y + Font->GetLineHeight() + Offset.y >= 0 && Pos.y + Offset.y < cr.Height() ) || Flags&WB_TEXTBOX_SINGLELINE )
    {
      TCHAR Char = Flags&WB_TEXTBOX_PASSWORD ? PasswordStar : Font->ApplyTextTransform( (_TUCHAR*)Text.GetPointer(), (_TUCHAR*)Text.GetPointer() + x, TextTransform );
      TS32 Width = Font->GetWidth( Char );

      if ( Char == '\t' )
        Width = ( (TS32)( Pos.x + TabWidth ) / TabWidth )*TabWidth - Pos.x;

      if ( ( Pos.y + Offset.y <= mp.y && mp.y < Pos.y + Font->GetLineHeight() + Offset.y ) || Flags&WB_TEXTBOX_SINGLELINE ) //we're in the correct line
      {
        if ( mp.x < Pos.x + Offset.x ) return x; //for line starts
        if ( Pos.x + Offset.x <= mp.x && mp.x < Pos.x + Width + Offset.x )  //we're in the correct column
          return x;
      }

      Pos.x += Width;
    }

    if ( Text[ x ] == '\n' )
    {
      if ( Pos.y + Offset.y <= mp.y && mp.y < Pos.y + Font->GetLineHeight() + Offset.y ) return x;
      Pos.x = 0;
      Pos.y += Font->GetLineHeight();
    }
  }
  return Text.Length();
}

TBOOL CWBTextBox::MessageProc( CWBMessage &Message )
{
  switch ( Message.GetMessage() )
  {

  case WBM_FOCUSGAINED:
    if ( Message.GetTarget() == GetGuid() )
    {
      OriginalText = Text;
      if ( GetChildInFocus() ) GetChildInFocus()->ClearFocus();
      return true;
    }
    break;

  case WBM_FOCUSLOST:
    if ( Message.GetTarget() == GetGuid() )
    {
      if ( GetParent() && GetParent()->GetChildInFocus() == this ) GetParent()->SetChildInFocus( NULL );
      return true;
    }
    break;

  case WBM_REPOSITION:
  {
    TBOOL b = CWBItem::MessageProc( Message );
    TS32 mi, ma, vi;
    GetHScrollbarParameters( mi, ma, vi );
    SetHScrollbarParameters( mi, ma, GetClientRect().Width() );
    GetVScrollbarParameters( mi, ma, vi );
    SetVScrollbarParameters( mi, ma, GetClientRect().Height() );
    SetCursorPos( CursorPos, false );
    return b;
  }
  break;

  case WBM_LEFTBUTTONDOWN:
    if ( App->GetMouseItem() != this ) break;
    if ( CWBItem::MessageProc( Message ) ) return true;
    App->SetCapture( this );
    SetCursorPos( GetCursorPosMouse(), App->GetShiftState() );
    DesiredCursorPosXinPixels = GetCursorXinPixels();
    return true;

  case WBM_LEFTBUTTONDBLCLK:
    if ( App->GetMouseItem() != this ) break;
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetShiftState() ) return true;
    SelectWord( GetCursorPosMouse() );
    return true;

  case WBM_MOUSEMOVE:
    if ( CWBItem::MessageProc( Message ) ) return true;
    if ( App->GetMouseCaptureItem() == this )
    {
      SetCursorPos( GetCursorPosMouse(), true );
      DesiredCursorPosXinPixels = GetCursorXinPixels();
    }
    return true;

  case WBM_MOUSEWHEEL:
  {
    CWBFont *Font = GetFont( GetState() );
    TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;
    SetVScrollbarPos( GetVScrollbarPos() - Message.Data * 3 * Font->GetLineHeight(), true );
  }
  return true;

  case WBM_LEFTBUTTONUP:
    if ( CWBItem::MessageProc( Message ) ) return true;
    App->ReleaseCapture();
    return true;

  case WBM_KEYDOWN:
  {
    if ( !InFocus() || GetChildInFocus() ) break;
    if ( Message.KeyboardState&WB_KBSTATE_ALT && Message.KeyboardState&WB_KBSTATE_CTRL )
      break; //altgr

    CWBFont *Font = GetFont( GetState() );
    TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;

    //handle cursor movement keys
    switch ( Message.Key )
    {
    case VK_LEFT:
      SetCursorPos( max( 0, CursorPos - 1 ), Message.KeyboardState&WB_KBSTATE_SHIFT );
      DesiredCursorPosXinPixels = GetCursorXinPixels();
      return true;

    case VK_RIGHT:
      SetCursorPos( min( (TS32)Text.Length(), CursorPos + 1 ), Message.KeyboardState&WB_KBSTATE_SHIFT );
      DesiredCursorPosXinPixels = GetCursorXinPixels();
      return true;

    case VK_UP:
      SetCursorPosXpxY( DesiredCursorPosXinPixels, GetCursorY() - 1, Message.KeyboardState&WB_KBSTATE_SHIFT );
      return true;

    case VK_DOWN:
      SetCursorPosXpxY( DesiredCursorPosXinPixels, GetCursorY() + 1, Message.KeyboardState&WB_KBSTATE_SHIFT );
      return true;

    case VK_PRIOR: //page up
      SetVScrollbarPos( GetVScrollbarPos() - ( (TS32)( GetClientRect().Height() / Font->GetLineHeight() ) )*Font->GetLineHeight(), true );
      SetCursorPosXpxY( DesiredCursorPosXinPixels, GetVScrollbarPos() ? GetCursorY() - GetClientRect().Height() / Font->GetLineHeight() : 0, Message.KeyboardState&WB_KBSTATE_SHIFT );
      return true;

    case VK_NEXT: //page down
      SetVScrollbarPos( GetVScrollbarPos() + ( (TS32)( GetClientRect().Height() / Font->GetLineHeight() ) )*Font->GetLineHeight(), true );
      SetCursorPosXpxY( DesiredCursorPosXinPixels, GetCursorY() + GetClientRect().Height() / Font->GetLineHeight(), Message.KeyboardState&WB_KBSTATE_SHIFT );
      return true;

    case VK_HOME:
      SetHScrollbarPos( 0 );
      if ( Message.KeyboardState&WB_KBSTATE_CTRL )
      {
        SetCursorPos( 0, Message.KeyboardState&WB_KBSTATE_SHIFT );
        DesiredCursorPosXinPixels = GetCursorXinPixels();
        return true;
      }

      //skip to the start of the line
      if ( GetCursorX() == GetLineLeadingWhiteSpaceSize() )
        SetCursorPos( CursorPos - GetCursorX(), Message.KeyboardState&WB_KBSTATE_SHIFT );
      else
        SetCursorPos( CursorPos - GetCursorX() + GetLineLeadingWhiteSpaceSize(), Message.KeyboardState&WB_KBSTATE_SHIFT );
      DesiredCursorPosXinPixels = GetCursorXinPixels();
      return true;

    case VK_END:
      if ( Message.KeyboardState&WB_KBSTATE_CTRL )
      {
        SetCursorPos( Text.Length(), Message.KeyboardState&WB_KBSTATE_SHIFT );
        DesiredCursorPosXinPixels = GetCursorXinPixels();
        return true;
      }

      //skip to the end of the line
      for ( ; CursorPos < (TS32)Text.Length(); CursorPos++ )
        if ( Text[ CursorPos ] == _T( '\n' ) )
        {
          SetCursorPos( CursorPos, Message.KeyboardState&WB_KBSTATE_SHIFT );
          DesiredCursorPosXinPixels = GetCursorXinPixels();
          break;
        }
      SetCursorPos( CursorPos, Message.KeyboardState&WB_KBSTATE_SHIFT );
      DesiredCursorPosXinPixels = GetCursorXinPixels();
      return true;

    case VK_RETURN:
      if ( Flags&WB_TEXTBOX_SINGLELINE )
      {
        App->SendMessage( CWBMessage( App, WBM_COMMAND, GetGuid() ) );
        ClearFocus();
        return true;
      }
      break;

    case VK_DELETE:
      if ( Message.KeyboardState&WB_KBSTATE_SHIFT ) Cut();
      else
      {
        if ( CursorPos < 0 || CursorPos >= (TS32)Text.Length() )
        {
          RemoveSelectedText();
          OnTextChange();
          return true;
        }

        if ( SelectionStart == SelectionEnd )
          RemoveText( CursorPos, 1, CursorPos );
        else
          RemoveSelectedText();
        DesiredCursorPosXinPixels = GetCursorXinPixels();
        OnTextChange();
      }
      return true;

    case VK_ESCAPE:
      if ( ( Flags&WB_TEXTBOX_SINGLELINE ) )
      {
        SetTextInternal( OriginalText, false, false );
        App->SendMessage( CWBMessage( App, WBM_COMMAND, GetGuid() ) );
        ClearFocus();
        return true;
      }

      return true;

    case VK_INSERT:
      if ( Message.KeyboardState&WB_KBSTATE_CTRL ) Copy();
      else
        if ( Message.KeyboardState&WB_KBSTATE_SHIFT ) Paste();
      return true;

    case 'X':
      if ( Message.KeyboardState&WB_KBSTATE_CTRL ) Cut();
      return true;

    case 'C':
      if ( Message.KeyboardState&WB_KBSTATE_CTRL ) Copy();
      return true;

    case 'V':
      if ( Message.KeyboardState&WB_KBSTATE_CTRL ) Paste();
      return true;

    case 'A': //select all
      if ( Message.KeyboardState&WB_KBSTATE_CTRL )
      {
        SetCursorPos( 0, false );
        SetCursorPos( Text.Length(), true );
      }
      return true;

    case 'Z':
      if ( Message.KeyboardState&WB_KBSTATE_CTRL ) Undo();
      return true;

    case 'Y':
      if ( Message.KeyboardState&WB_KBSTATE_CTRL ) Redo();
      return true;
    }

    return false;

    return true; //this captures all keydowns. might not be a good idea all the time
  }
  case WBM_CHAR:
    if ( !InFocus() || GetChildInFocus() ) break;
    if ( !( Message.KeyboardState&WB_KBSTATE_CTRL && Message.KeyboardState&WB_KBSTATE_ALT ) ) //altgr
      if ( Message.KeyboardState&( WB_KBSTATE_ALT | WB_KBSTATE_CTRL ) ) return true; //these should be handled by the keydown messages

    if ( Message.Key == VK_BACK )
    {
      if ( SelectionStart == SelectionEnd )
      {
        if ( !CursorPos ) return true;
        RemoveText( CursorPos - 1, 1, CursorPos - 1 );
      }
      else
        RemoveSelectedText();
      OnTextChange();

      return true;
    }

    if ( Message.Key == VK_ESCAPE )
    {
      SelectionStart = SelectionEnd = SelectionOrigin = CursorPos;
      return true; //shouldn't produce text
    }

    if ( ( Flags&WB_TEXTBOX_SINGLELINE ) && Message.Key == VK_RETURN ) return true; //already handled in the keydown message

    //translate VK_RETURN to '\n':

    TS32 Key = Message.Key;

    if ( Message.Key == VK_RETURN )
      Key = _T( '\n' );

    //insert character
    RemoveSelectedText();
    InsertText( CursorPos, (TCHAR*)&Key, 1, CursorPos + 1 );

    PostCharInsertion( CursorPos, Message.Key );

    DesiredCursorPosXinPixels = GetCursorXinPixels();
    OnTextChange();

    return true; //this captures all character inputs which should be fine

  }

  return CWBItem::MessageProc( Message );
}

void CWBTextBox::SetText( CString val, TBOOL EnableUndo )
{
  SetTextInternal( val, EnableUndo, true );
}

void CWBTextBox::SetTextInternal( CString val, TBOOL EnableUndo, TBOOL nonHumanInteraction )
{
  if ( !EnableUndo )
  {
    OriginalText = Text = val;
    Text.ToUnixNewline();
    History.FreeArray();
    HistoryPosition = 0;
  }
  else
  {
    SelectionStart = 0;
    SelectionEnd = Text.Length();
    RemoveSelectedText();
    val.ToUnixNewline();
    InsertText( 0, val.GetPointer(), val.Length(), val.Length() );
  }

  SetCursorPos( Text.Length(), false );
  OnTextChange( nonHumanInteraction != 0 );
}

void CWBTextBox::OnTextChange( bool nonHumanInteraction/* = false*/ )
{
  //calculate new scrollbar data
  WBITEMSTATE i = GetState();
  CWBFont *Font = GetFont( i );
  WBTEXTTRANSFORM TextTransform = (WBTEXTTRANSFORM)CSSProperties.DisplayDescriptor.GetValue( i, WB_ITEM_TEXTTRANSFORM );
  TS32 TabWidth = Font->GetWidth( _T( ' ' ) ) * 4;

  App->SendMessage( CWBMessage( App, WBM_TEXTCHANGED, GetGuid(), App->GetFocusItem() == this && !nonHumanInteraction ) );

  CPoint Size = CPoint( 0, Font->GetLineHeight() );
  TS32 XSize = 0;

  for ( TS32 x = 0; x < (TS32)Text.Length(); x++ )
  {
    TCHAR Char = Flags&WB_TEXTBOX_PASSWORD ? PasswordStar : Font->ApplyTextTransform( (_TUCHAR*)Text.GetPointer(), (_TUCHAR*)Text.GetPointer() + x, TextTransform );
    TS32 Width = Font->GetWidth( Char );

    if ( Char == '\t' )
      Width = ( (TS32)( XSize + TabWidth ) / TabWidth )*TabWidth - XSize;

    XSize += Width;

    if ( Text[ x ] == '\n' )
    {
      Size.x = max( Size.x, XSize );
      Size.y += Font->GetLineHeight();
      XSize = 0;
    }
  }

  SetHScrollbarParameters( 0, max( XSize, Size.x ), GetClientRect().Width() );
  SetVScrollbarParameters( 0, Size.y, GetClientRect().Height() );

  if ( GetClientRect().Width() >= max( XSize, Size.x ) )	SetHScrollbarPos( 0, true );
  if ( GetClientRect().Height() >= Size.y )	SetVScrollbarPos( 0, true );

  DoSyntaxHighlight();
}

void CWBTextBox::InsertText( TS32 Position, TCHAR *txt, TS32 Length, TS32 CursorPosAfter, TBOOL ChangeHistory )
{
  if ( ChangeHistory )
  {
    CWBTextBoxHistoryEntry *e = CreateNewHistoryEntry( false, Position, Length );
    e->CursorPos_Before = CursorPos;
    e->SelectionStart_Before = SelectionStart;
    e->SelectionEnd_Before = SelectionEnd;
    e->Data = CString( txt, Length );
  }
  //LOG_DEBUG("[inbox] Inserted %s (%d) to %d",txt,Length,Position);

  Text.Insert( Position, txt );

  SetCursorPos( CursorPosAfter, false );
}

void CWBTextBox::RemoveText( TS32 Position, TS32 Length, TS32 CursorPosAfter, TBOOL ChangeHistory )
{
  if ( ChangeHistory )
  {
    CWBTextBoxHistoryEntry *e = CreateNewHistoryEntry( true, Position, Length );
    e->CursorPos_Before = CursorPos;
    e->SelectionStart_Before = SelectionStart;
    e->SelectionEnd_Before = SelectionEnd;
    e->Data = Text.Substring( Position, Length );
  }
  //LOG_DEBUG("[inbox] Removed %s (%d) from %d",Text.Substring(Position,Length).GetPointer(),Length,Position);

  Text.DeleteRegion( Position, Length );

  SetCursorPos( CursorPosAfter, false );
}

void CWBTextBox::SetSelection( TS32 start, TS32 end )
{
  SelectionStart = max( 0, min( start, end ) );
  SelectionEnd = min( (TS32)Text.Length(), max( start, end ) );
}

CColor CWBTextBox::GetTextColor( TS32 Index, CColor &DefaultColor )
{
  return DefaultColor;
}

TBOOL CWBTextBox::ApplyStyle( CString & prop, CString & value, CStringArray &pseudo )
{
  TBOOL ElementTarget = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "selection" ) )
    {
      ElementTarget = true;
      break;
    }
  }

  if ( !ElementTarget ) return CWBItem::ApplyStyle( prop, value, pseudo );

  TBOOL Handled = false;

  for ( TS32 x = 1; x < pseudo.NumItems(); x++ )
  {
    if ( pseudo[ x ] == _T( "selection" ) )
    {
      Handled |= Selection.ApplyStyle( this, prop, value, pseudo );
      continue;
    }
  }

  return Handled;
}

CWBItem * CWBTextBox::Factory( CWBItem *Root, CXMLNode &node, CRect &Pos )
{
  TS32 Flags = 0;
  if ( node.HasAttribute( _T( "singleline" ) ) )
  {
    TS32 b = 0;
    node.GetAttributeAsInteger( _T( "singleline" ), &b );
    Flags |= b*WB_TEXTBOX_SINGLELINE;
  }

  if ( node.HasAttribute( _T( "password" ) ) )
  {
    TS32 b = 0;
    node.GetAttributeAsInteger( _T( "password" ), &b );
    Flags |= b*WB_TEXTBOX_PASSWORD;
  }

  if ( node.HasAttribute( _T( "linenumbers" ) ) )
  {
    TS32 b = 0;
    node.GetAttributeAsInteger( _T( "linenumbers" ), &b );
    Flags |= b*WB_TEXTBOX_LINENUMS;
  }

  if ( node.HasAttribute( _T( "selection" ) ) )
  {
    TS32 b = 0;
    node.GetAttributeAsInteger( _T( "selection" ), &b );
    Flags |= ( !b )*WB_TEXTBOX_NOSELECTION;
  }

  CWBTextBox * textbox = new CWBTextBox( Root, Pos, Flags );
  if ( node.HasAttribute( _T( "text" ) ) )
    textbox->SetTextInternal( node.GetAttribute( _T( "text" ) ), false, true );

  if ( Flags&WB_TEXTBOX_SINGLELINE )
  {
    textbox->EnableHScrollbar( false, false );
    textbox->EnableVScrollbar( false, false );
  }

  return textbox;
}

void CWBTextBox::SelectWord( TS32 CharacterInWord )
{
  if ( CharacterInWord < 0 || CharacterInWord >= (TS32)Text.Length() ) return;

  TBOOL IsWhiteSpace = _istspace( Text[ CharacterInWord ] );
  TBOOL IsAlNum = _istalnum( Text[ CharacterInWord ] );
  if ( !IsWhiteSpace && !IsAlNum )
  {
    SetCursorPos( CharacterInWord, false );
    SetCursorPos( CharacterInWord + 1, true );
    return;
  }

  TS32 Start = CharacterInWord;
  TS32 End = CharacterInWord;

  if ( IsWhiteSpace )
  {
    while ( Start >= 0 && _istspace( Text[ Start ] ) && Text[ Start ] != _T( '\n' ) && Text[ Start ] != _T( '\r' ) ) { Start--; }
    while ( End < (TS32)Text.Length() && _istspace( Text[ End ] ) && Text[ End ] != _T( '\n' ) && Text[ End ] != _T( '\r' ) ) { End++; }
    End = min( (TS32)Text.Length() - 1, End );
  }
  else
  {
    while ( Start >= 0 && _istalnum( Text[ Start ] ) || Text[ Start ] == _T( '_' ) ) { Start--; }
    while ( End < (TS32)Text.Length() && _istalnum( Text[ End ] ) || Text[ End ] == _T( '_' ) ) { End++; }
    End = min( (TS32)Text.Length() - 1, End );
  }

  Start++;
  SetCursorPos( Start, false );
  SetCursorPos( End, true );
}

CWBTextBoxHistoryEntry::CWBTextBoxHistoryEntry()
{

}

CWBTextBoxHistoryEntry::~CWBTextBoxHistoryEntry()
{

}
