#pragma once
#include "GuiItem.h"

class CWBRoot : public CWBItem
{
  TBOOL MessageProc( CWBMessage &Message );
  virtual void OnDraw( CWBDrawAPI *API );

public:

  CWBRoot();
  CWBRoot( CWBItem *Parent, const CRect &Pos );
  virtual ~CWBRoot();

  void SetApplication( CWBApplication *Application );
  virtual TBOOL Initialize( CWBItem *Parent, const CRect &Position );

  WB_DECLARE_GUIITEM( _T( "root" ), CWBItem );

};