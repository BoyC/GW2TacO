#pragma once
#include "Bedrock/WhiteBoard/WhiteBoard.h"

class COverlayApp : public CWBApplication
{

protected:

  virtual LRESULT WindowProc( UINT uMsg, WPARAM wParam, LPARAM lParam );
  virtual TBOOL DeviceOK();

public:

  virtual TBOOL Initialize( const CCoreWindowParameters &WindowParams );

  COverlayApp();
  virtual ~COverlayApp();

  virtual void TakeScreenshot();

  CCoreBlendState *holePunchBlendState = nullptr;

};