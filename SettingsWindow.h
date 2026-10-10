#pragma once
#include "OverlayWindow.h"
#include <functional>
#include <vector>

class GW2TacO;

// one line of the settings page; get/set talk to the config, set also runs button actions
struct SettingsRow
{
  enum Type { Header, Text, Toggle, Choice, Chips, Buttons };

  struct Option
  {
    CString label;
    TS32 value = 0;
    CString config; // chips toggle their own config value
  };

  Type type = Text;
  CString label;
  TS32 indent = 0;
  TBOOL highlight = false; // label drawn in the accent color, like the API key in use
  std::function<TS32()> get;
  std::function<void( TS32 )> set;
  std::vector<Option> options;
};

// the tabbed TacO settings window that replaces the nested settings menus
class GW2SettingsPanel : public CWBItem
{
  GW2TacO* taco;
  TS32 tab = 0;
  TS32 scroll = 0;
  TS32 removeMarkersClickTime = -10000;
  TS32 labelColumn = 0; // where the option boxes start, the same for every row of a tab

  std::vector<SettingsRow> BuildRows();
  CRect GetContentRect( TBOOL scrollbar );
  CRect PrepareLayout( CWBFont* font, std::vector<SettingsRow>& rows, TS32& maxScroll );
  TS32 LayoutRow( CWBFont* font, SettingsRow& row, TS32 y, const CRect& content, std::vector<CRect>& optionRects );
  TS32 GetRowHeight( CWBFont* font );
  TS32 GetContentHeight( CWBFont* font, std::vector<SettingsRow>& rows, const CRect& content );
  void OnClick( CPoint pos );

public:

  GW2SettingsPanel( CWBItem* Parent, CRect Position, GW2TacO* taco );
  virtual ~GW2SettingsPanel();

  virtual void OnDraw( CWBDrawAPI* API );
  virtual TBOOL MessageProc( CWBMessage& Message );

  WB_DECLARE_GUIITEM( _T( "settingspanel" ), CWBItem );
};

class GW2SettingsWindow : public OverlayWindow
{
public:

  GW2SettingsWindow( CWBItem* Parent, CRect Position, GW2TacO* taco );
  virtual ~GW2SettingsWindow();

  virtual void OnDraw( CWBDrawAPI* API );
  virtual TBOOL IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType );

  WB_DECLARE_GUIITEM( _T( "settingswindow" ), OverlayWindow );
};
