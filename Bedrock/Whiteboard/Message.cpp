#include "BasePCH.h"
#include "Message.h"
#include "Application.h"

CWBMessage::CWBMessage()
{
  App = NULL;
  Message = WBM_NONE;
  Target = 0;
}

CWBMessage::~CWBMessage()
{
}

CWBMessage::CWBMessage( CWBApplication *app, WBMESSAGE message, WBGUID target )
{
  App = app;
  Message = message;
  Target = target;
}

CWBMessage::CWBMessage( CWBApplication *app, WBMESSAGE message, WBGUID target, TS32 x )
{
  App = app;
  Message = message;
  Target = target;
  Data = x;
}

CWBMessage::CWBMessage( CWBApplication *app, WBMESSAGE message, WBGUID target, TS32 x, TS32 y )
{
  App = app;
  Message = message;
  Target = target;
  Position[ 0 ] = x;
  Position[ 1 ] = y;
}

TBOOL CWBMessage::IsMouseMessage()
{
  return Message >= WBM_MOUSEMOVE && Message <= WBM_MOUSEWHEEL;
}

TBOOL CWBMessage::IsTargetID( const CString& Name )
{
  if ( !App ) return false;
  CWBItem *i = App->FindItemByGuid( Target );
  if ( !i ) return false;
  return i->GetID() == Name;
}

CString CWBMessage::GetTargetID()
{
  if ( !App ) return _T( "" );
  CWBItem *i = App->FindItemByGuid( Target );
  if ( !i ) return _T( "" );
  return i->GetID();
}
