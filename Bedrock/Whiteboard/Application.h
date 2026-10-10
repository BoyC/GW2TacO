#pragma once

#include "../CoRE2/Core2.h"
#include "../UtilLib/XMLDocument.h"
#include "Message.h"
#include "DrawAPI.h"
#include "GuiItem.h"
#include "Root.h"
#include "ContextMenu.h"
#include "StyleManager.h"

enum WBMOUSECLICKREPEATMODE
{
  WB_MCR_OFF = 0,
  WB_MCR_LEFT = 1,
  WB_MCR_RIGHT = 2,
  WB_MCR_MIDDLE = 3
};

typedef CWBItem* ( __cdecl *WBFACTORYCALLBACK )( CWBItem *Root, CXMLNode &node, CRect &Pos );

typedef CString( __cdecl *WBLOCALIZERCALLBACK )( const CString &Token );

struct WBFONTFALLBACK // see CWBFontDescription::AddSystemFontGlyphs()
{
  const wchar_t *FaceName;
  TU16 FirstChar, LastChar;
  TS32 MinBodyHeight;
};

class CWBApplication : public CCoreWindowHandlerWin
{
  friend class CWBItem;
  CArray<CWBItem*> MessagePath;

  CRingBuffer<TS32> *FrameTimes;
  TS32 LastFrameTime;

  CDictionary<WBGUID, CWBItem*> Items;
  CArrayThreadSafe<CWBMessage> MessageBuffer;
  CArrayThreadSafe<CWBItem*> Trash;

  CWBItem *MouseCaptureItem;
  CWBItem *MouseItem;

  CWBSkin *Skin;
  CDictionaryEnumerable<CString, CWBFont*> Fonts;
  CWBFont *DefaultFont;
  CArray<WBFONTFALLBACK> FontFallbacks;
  WBLOCALIZERCALLBACK Localizer = nullptr;

  TBOOL Alt, Ctrl, Shift, Left, Middle, Right;

  TBOOL Vsync;

  CString ScreenShotName;
  CString PendingScreenshot;

  TBOOL SendMessageToItem( CWBMessage &Message, CWBItem *Target );
  void ProcessMessage( CWBMessage &Message );

  virtual void UpdateMouseItem();
  virtual void CleanTrash();
  virtual TS32 GetKeyboardState();

  WBMOUSECLICKREPEATMODE ClickRepeaterMode;
  TS64 NextRepeatedClickTime;

  //////////////////////////////////////////////////////////////////////////
  // gui layout and style

  CDictionary<CString, CXMLDocument*> LayoutRepository;
  CStyleManager StyleManager;

  //////////////////////////////////////////////////////////////////////////
  // ui generator

  CDictionary<CString, WBFACTORYCALLBACK> FactoryCallbacks;

  TBOOL ProcessGUIXML( CWBItem *Root, CXMLNode & node );
  TBOOL GenerateGUIFromXMLNode( CWBItem * Root, CXMLNode & node, CRect &Pos );
  TBOOL GenerateGUITemplateFromXML( CWBItem *Root, CXMLDocument *doc, CString &TemplateID );
  CWBItem *GenerateUIItem( CWBItem *Root, CXMLNode &node, CRect &Pos );

  CColor ClearColor = CColor( 0, 0, 0, 255 );

protected:

  CAtlas *Atlas;
  CWBRoot *Root;

  CWBDrawAPI *DrawAPI;

  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
  TBOOL GenerateGUIFromXML( CWBItem *Root, CXMLDocument *doc );
  virtual TBOOL Initialize();

public:

  virtual TBOOL Initialize( const CCoreWindowParameters &WindowParams );

  CWBApplication();
  virtual ~CWBApplication();

  CWBItem *GetRoot();
  CWBItem *GetFocusItem();
  virtual TBOOL HandleMessages();
  virtual TBOOL IsDone() { return Done; }
  virtual void SetDone( TBOOL d );
  virtual void Display();
  virtual void Display( CWBDrawAPI *DrawAPI );

  CWBItem* GetItemUnderMouse( CPoint& Point, WBMESSAGE w );

  void RegisterItem( CWBItem *Item );
  void UnRegisterItem( CWBItem *Item );
  CWBItem *FindItemByGuid( WBGUID Guid, const TCHAR *type = NULL );

  template< typename... Args>
  CWBItem *FindItemByGuids( WBGUID Guid, Args ... args )
  {
    int len = sizeof...( Args );
    const TCHAR* vals[] = { args... };

    if ( !len )
      return FindItemByGuid( Guid );

    for ( int x = 0; x < len; x++ )
    {
      auto item = FindItemByGuid( Guid, vals[ x ] );
      if ( item )
        return item;
    }

    return nullptr;
  }

  virtual void SendMessage( CWBMessage &Message );
  CWBItem *SetCapture( CWBItem *Capturer );
  TBOOL ReleaseCapture();

  CWBItem *GetMouseItem();
  CWBItem *GetMouseCaptureItem();

  TBOOL CreateFont( CString FontName, CWBFontDescription *Font );
  // characters of [FirstChar,LastChar] missing from fonts created afterwards get rasterized from this system font
  void AddFontFallback( const wchar_t *FaceName, TU16 FirstChar, TU16 LastChar, TS32 MinBodyHeight );
  CWBFont *GetFont( CString FontName );
  void SetLocalizer( WBLOCALIZERCALLBACK Callback ) { Localizer = Callback; }
  CString LocalizeText( const CString &Text ); // layout texts starting with '@' are tokens handed to the localizer
  CWBFont *GetDefaultFont();
  TBOOL SetDefaultFont( CString FontName );
  void AddToTrash( CWBItem *item );

  TBOOL GetCtrlState() { return Ctrl; }
  TBOOL GetAltState() { return Alt; }
  TBOOL GetShiftState() { return Shift; }
  TBOOL GetLeftButtonState() { return Left; }
  TBOOL GetRightButtonState() { return Right; }
  TBOOL GetMiddleButtonState() { return Middle; }

  TBOOL LoadXMLLayout( CString & XML );
  TBOOL LoadXMLLayoutFromFile( CString FileName );
  TBOOL LoadCSS( CString & CSS, TBOOL ResetStyleManager = true );
  TBOOL LoadCSSFromFile( CString FileName, TBOOL ResetStyleManager = true );
  TBOOL GenerateGUI( CWBItem *Root, const TCHAR *layout );
  TBOOL GenerateGUI( CWBItem *Root, CString &Layout );
  TBOOL GenerateGUITemplate( CWBItem *Root, CString &Layout, CString &TemplateID );
  TBOOL GenerateGUITemplate( CWBItem *Root, TCHAR *Layout, TCHAR *TemplateID );
  TBOOL LoadSkin( CString &XML, CArray<int>& enabledGlyphs = CArray<int>() );
  TBOOL LoadSkinFromFile( CString FileName, CArray<int>& enabledGlyphs = CArray<int>() );

  CAtlas *GetAtlas();
  CWBSkin *GetSkin();

  void ApplyStyle( CWBItem *Target );
  void ReApplyStyle();

  void RegisterUIFactoryCallback( TCHAR *ElementName, WBFACTORYCALLBACK FactoryCallback );
  void RegisterUIFactoryCallback( CString &ElementName, WBFACTORYCALLBACK FactoryCallback );

  virtual void TakeScreenshot();
  void SetScreenshotName( CString s ) { ScreenShotName = s; }
  void RequestScreenshot( CString FileName ) { PendingScreenshot = FileName; } // saves the next rendered frame as a png

  void SetVSync( TBOOL vs ) { Vsync = vs; }
  void SetClearColor( CColor color ) { ClearColor = color; }

  TF32 GetFrameRate();
  TS32 GetInitialKeyboardDelay();
  TS32 GetKeyboardRepeatTime();

  virtual void HandleResize();

  LRESULT InjectMessage( UINT uMsg, WPARAM wParam, LPARAM lParam );
  virtual void UpdateControlKeyStates(); //update ctrl alt shift states
};