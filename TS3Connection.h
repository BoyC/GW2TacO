#pragma once
#include "Bedrock/BaseLib/BaseLib.h"

class TS3Connection
{
  CSocket connection;

  struct CommandResponse
  {
    CStringArray Lines;
    TS32 ErrorCode = -1;
    CString Message;
  };

  TS32 currentHandlerID = 1;

  void ProcessNotification( CString &s );
  void ProcessChannelList( CString &channeldata, TS32 handler );
  void ProcessClientList( CString &clientdata, TS32 handler );
  CString ReadLine();

  TS32 LastPingTime = 0;

public:

  class TS3Client
  {
  public:
    TS32 clientid = 0;
    TS32 channelid = 0;
    CString name;
    TS32 talkStatus = 0;
    TS32 inputmuted = 0;
    TS32 outputmuted = 0;

    TU64 lastTalkTime = 0;
  };

  class TS3Channel
  {
  public:
    TS32 id = 0;
    TS32 parentid = 0;
    TS32 order = 0;
    CString name;
  };

  class TS3Schandler
  {
  public:
    TS32 id = 0;
    TBOOL Connected = false;
    TS32 myclientid = 0;
    TBOOL clientIDInvalid = true;
    CDictionaryEnumerable<TS32, TS3Channel> Channels;
    CDictionaryEnumerable<TS32, TS3Client> Clients;
    CString name;
  };

  CDictionary<TS32, TS3Schandler> handlers;

  TS3Connection();
  virtual ~TS3Connection();

  TBOOL TryConnect();
  void TryValidateClientID();

  void Tick();
  void InitConnection();

  CommandResponse SendCommand( CString &message );
  CommandResponse SendCommand( TCHAR *message );

  void ProcessNotifications();

  void WaitForResponse();

  TBOOL IsConnected();

  CString unescape( CString string );

  bool authenticated = true;

};

extern TS3Connection teamSpeakConnection;