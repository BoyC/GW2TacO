#pragma once
#include "Application.h"

#define WB_TEXTBOX_SINGLELINE		0x0000001
#define WB_TEXTBOX_PASSWORD		0x0000002
#define WB_TEXTBOX_NOSELECTION		0x0000004
#define WB_TEXTBOX_LINENUMS		0x0000008
#define WB_TEXTBOX_DISABLECOPY 	0x0000010

class CWBTextBoxHistoryEntry
{
  friend class CWBTextBox;

  TBOOL Remove;
  TS32 StartPosition;
  CString Data;

  TS32 CursorPos_Before, SelectionStart_Before, SelectionEnd_Before;

public:

  CWBTextBoxHistoryEntry();
  virtual ~CWBTextBoxHistoryEntry();
};


class CWBTextBox : public CWBItem
{
  CArray<CWBTextBoxHistoryEntry*> History;
  TS32 HistoryPosition;

  TS32 HiglightStartTime;
  TS32 CursorBlinkStartTime;

  TS32 Flags;
  TCHAR PasswordStar; //character to display instead of characters

  TS32 CursorPos;
  TS32 DesiredCursorPosXinPixels; //used when moving up and down

  TS32 SelectionStart, SelectionEnd, SelectionOrigin;

  CWBCSSPropertyBatch Selection;

  INLINE void DrawCursor( CWBDrawAPI *API, CPoint &p );
  void SetCursorPosXpxY( TS32 x, TS32 y, TBOOL Selecting );
  void RemoveSelectedText();
  void Copy();
  void Cut();
  void Paste();
  void Undo();
  void Redo();

  TS32 GetCursorX();
  TS32 GetCursorXinPixels();
  TS32 GetCursorY();
  TS32 GetLineSize();
  TS32 GetCursorPosMouse();
  TS32 GetLineLeadingWhiteSpaceSize();

  CPoint GetTextStartOffset();

  virtual void DoSyntaxHighlight() {};
  virtual void OnTextChange( bool nonHumanInteraction = false );
  CWBTextBoxHistoryEntry *CreateNewHistoryEntry( bool Remove, int Start, int Length );

  virtual CColor GetTextColor( TS32 Index, CColor &DefaultColor );
  virtual TBOOL GetTextBackground( TS32 Index, CColor &Result ) { return false; }
  virtual TBOOL ColoredText() { return false; }
  virtual void OnCursorPosChange( TS32 CursorPos ) {}
  virtual void PostCharInsertion( TS32 CursorPos, TS32 Key ) {};

  virtual void SelectWord( TS32 CharacterInWord );

  void SetTextInternal( CString val, TBOOL EnableUndo = false, TBOOL nonHumanInteraction = false );

protected:

  CString Text;

  CString OriginalText; //for escape cancel

  virtual void OnDraw( CWBDrawAPI *API );
  virtual TBOOL MessageProc( CWBMessage &Message );
  void InsertText( TS32 Position, TCHAR *Text, TS32 Length, TS32 CursorPosAfter, TBOOL ChangeHistory = true );
  void RemoveText( TS32 Position, TS32 Length, TS32 CursorPosAfter, TBOOL ChangeHistory = true );

public:

  CWBTextBox();
  CWBTextBox( CWBItem *Parent, const CRect &Pos, TS32 flags = WB_TEXTBOX_SINGLELINE, const TCHAR *txt = _T( "" ) );
  virtual ~CWBTextBox();

  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position, TS32 flags = WB_TEXTBOX_SINGLELINE, const TCHAR *txt = _T( "" ) );
  virtual TBOOL ApplyStyle( CString & prop, CString & value, CStringArray &pseudo );

  CString GetText() const { return Text; } //text returned here will always be in unix newline format ('\n' instead of the windows '\r\n')
  void SetText( CString val, TBOOL EnableUndo = false );

  static CWBItem *Factory( CWBItem *Root, CXMLNode &node, CRect &Pos );
  WB_DECLARE_GUIITEM( _T( "textbox" ), CWBItem );

  virtual void SetSelection( TS32 start, TS32 end );
  void SetCursorPos( TS32 pos, TBOOL Selecting );
};