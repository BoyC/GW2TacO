#include "TS3Connection.h"
#include "OverlayConfig.h"

//currentschandlerid
//serverconnectionhandlerlist
//clientnotifyregister

TS3Connection teamSpeakConnection;

TS3Connection::TS3Connection()
{
  InitWinsock();
}

TS3Connection::~TS3Connection()
{
  DeinitWinsock();
}

TBOOL TS3Connection::TryConnect()
{
  if ( connection.IsConnected() )
    return true;

  handlers.Flush();

  TBOOL connected = connection.Connect( "localhost", 25639 );
  if ( !connected )
    return false;

  ReadLine();
  ReadLine();
  ReadLine();

  InitConnection();

  return true;
}

void TS3Connection::TryValidateClientID()
{
  for ( TS32 x = 0; x < handlers.NumItems(); x++ )
    if ( handlers[ x ].Connected && handlers[ x ].clientIDInvalid )
    {
      currentHandlerID = handlers[ x ].id;
      CommandResponse use = SendCommand( CString::Format( "use %d", currentHandlerID ) );
      if ( use.ErrorCode )
        continue;
      CommandResponse whoami = SendCommand( CString::Format( "whoami" ) );
      handlers[ currentHandlerID ].Connected = whoami.ErrorCode != 1794;

      if ( whoami.ErrorCode == 512 )
        handlers[ currentHandlerID ].clientIDInvalid = true;

      if ( !whoami.ErrorCode )
      {
        handlers[ currentHandlerID ].clientIDInvalid = false;
        currentHandlerID = currentHandlerID;
        TS32 clid = 0, cid = 0;
        whoami.Lines[ 0 ].Scan( "clid=%d cid=%d", &clid, &cid );
        handlers[ currentHandlerID ].Clients[ clid ].clientid = clid;
        handlers[ currentHandlerID ].Clients[ clid ].channelid = cid;
        handlers[ currentHandlerID ].myclientid = clid;

        CommandResponse serverName = SendCommand( "servervariable virtualserver_name" );
        if ( !serverName.ErrorCode )
          if ( serverName.Lines[ 0 ].Find( "virtualserver_name=" ) == 0 )
            handlers[ currentHandlerID ].name = unescape( serverName.Lines[ 0 ].Substring( 19 ) );

        CommandResponse channelList = SendCommand( "channellist" );
        if ( !channelList.ErrorCode )
          ProcessChannelList( channelList.Lines[ 0 ], currentHandlerID );

        CommandResponse clientList = SendCommand( "clientlist -voice" );
        if ( !clientList.ErrorCode )
          ProcessClientList( clientList.Lines[ 0 ], currentHandlerID );
      }

    }
}

void TS3Connection::Tick()
{
  if ( !connection.IsConnected() )
  {
    if ( FindWindow( NULL, "TeamSpeak 3" ) )
    {
      if ( !TryConnect() )
        return;
    }
    else
      return;
  }
  else
  {
    TryValidateClientID();
  }

  if ( !authenticated )
  {
    if ( GetTime() - LastPingTime > 1000 )
    {
      InitConnection();
      LastPingTime = GetTime();
    }

    if ( !authenticated )
      return;
  }

  ProcessNotifications();

  if ( GetTime() - LastPingTime > 5000 )
  {
    CommandResponse res = SendCommand( "whoami" );
    LastPingTime = GetTime();
  }
}

void TS3Connection::InitConnection()
{
  if ( Config::HasString( "TS3APIKey" ) )
  {
    CString apiKey = Config::GetString( "TS3APIKey" );
    auto response = SendCommand( CString::Format( "auth apikey=" ) + apiKey ); // 3P9O-GWJ8-1TKI-OY1F-AX0T-BPQK
    if ( response.ErrorCode )
      authenticated = false;
    else
      authenticated = true;
  }

  if ( !authenticated )
    return;

  SendCommand( CString::Format( "clientnotifyregister schandlerid=0 event=any" ) );

  currentHandlerID = 1;

  CommandResponse response = SendCommand( "serverconnectionhandlerlist" );
  if ( !response.ErrorCode && response.Lines.NumItems() )
  {
    CStringArray schandlers = response.Lines[ 0 ].Explode( "|" );
    for ( TS32 x = 0; x < schandlers.NumItems(); x++ )
      if ( schandlers[ x ].Find( "schandlerid=" ) == 0 )
      {
        TS3Schandler handler;
        schandlers[ x ].Scan( "schandlerid=%d", &handler.id );
        handlers[ handler.id ] = handler;
        CommandResponse use = SendCommand( CString::Format( "use %d", handler.id ) );
        if ( use.ErrorCode )
          continue;
        CommandResponse whoami = SendCommand( CString::Format( "whoami" ) );
        handlers[ handler.id ].Connected = whoami.ErrorCode != 1794;

        if ( whoami.ErrorCode == 512 )
          handlers[ handler.id ].clientIDInvalid = true;

        if ( !whoami.ErrorCode )
        {
          handlers[ handler.id ].clientIDInvalid = false;
          currentHandlerID = handler.id;
          TS32 clid = 0, cid = 0;
          whoami.Lines[ 0 ].Scan( "clid=%d cid=%d", &clid, &cid );
          handlers[ handler.id ].Clients[ clid ].clientid = clid;
          handlers[ handler.id ].Clients[ clid ].channelid = cid;
          handlers[ handler.id ].myclientid = clid;

          CommandResponse serverName = SendCommand( "servervariable virtualserver_name" );
          if ( !serverName.ErrorCode )
            if ( serverName.Lines[ 0 ].Find( "virtualserver_name=" ) == 0 )
              handlers[ handler.id ].name = unescape( serverName.Lines[ 0 ].Substring( 19 ) );

          CommandResponse channelList = SendCommand( "channellist" );
          if ( !channelList.ErrorCode )
            ProcessChannelList( channelList.Lines[ 0 ], handler.id );

          CommandResponse clientList = SendCommand( "clientlist -voice" );
          if ( !clientList.ErrorCode )
            ProcessClientList( clientList.Lines[ 0 ], handler.id );
        }
      }
  }

  SendCommand( CString::Format( "use %d", currentHandlerID ) );
}

TS3Connection::CommandResponse TS3Connection::SendCommand( CString& message )
{
  CommandResponse response;

  if ( !connection.IsConnected() )
    return response;

  ProcessNotifications();

  connection.Write( message.GetPointer(), message.Length() );
  connection.Write( "\n", 1 );

  while ( 1 )
  {
    if ( !connection.IsConnected() )
    {
      response.ErrorCode = -1;
      response.Message = "Disconnected";
      return response;
    }

    CString nextline = ReadLine();
    response.Lines += nextline;
    if ( nextline.Find( "error" ) == 0 )
    {
      CStringArray msg = nextline.ExplodeByWhiteSpace();
      if ( msg.NumItems() >= 2 )
        msg[ 1 ].Scan( "id=%d", &response.ErrorCode );

      if ( response.ErrorCode == 1796 )
        authenticated = false;

      if ( msg.NumItems() >= 3 )
        response.Message = msg[ 2 ].Substring( 4 );

      if ( response.ErrorCode )
        LOG_DBG( "[GW2TacO] command %s response: %d %s", message.GetPointer(), response.ErrorCode, response.Message.GetPointer() );
      break;
    }
  }

  return response;
}

TS3Connection::CommandResponse TS3Connection::SendCommand( TCHAR* message )
{
  return SendCommand( CString( message ) );
}

void TS3Connection::ProcessNotifications()
{
  while ( connection.GetLength() )
    ProcessNotification( ReadLine() );
}

int ClientTalkTimeSorter( const TS3Connection::TS3Client& a, const TS3Connection::TS3Client& b )
{
  return (int)( b.lastTalkTime - a.lastTalkTime );
}

void TS3Connection::ProcessNotification( CString& s )
{
  CStringArray cmd = s.ExplodeByWhiteSpace();

  TS32 schandlerid = 0;
  for ( TS32 x = 1; x < cmd.NumItems(); x++ )
    if ( cmd[ x ].Find( "schandlerid=" ) == 0 )
      cmd[ x ].Scan( "schandlerid=%d", &schandlerid );

  if ( cmd[ 0 ] == "notifytalkstatuschange" )
  {
    TS32 clientid = -1;
    TS32 status = -1;
    for ( TS32 x = 1; x < cmd.NumItems(); x++ )
    {
      if ( cmd[ x ].Find( "status=" ) == 0 )
        cmd[ x ].Scan( "status=%d", &status );
      if ( cmd[ x ].Find( "clid=" ) == 0 )
        cmd[ x ].Scan( "clid=%d", &clientid );
    }

    if ( clientid >= 0 && status >= 0 )
    {
      if ( handlers[ schandlerid ].Clients[ clientid ].talkStatus != status && status > 0 )
      {
        handlers[ schandlerid ].Clients[ clientid ].lastTalkTime = GetTime();
        handlers[ schandlerid ].Clients.SortByValue( ClientTalkTimeSorter );
      }

      handlers[ schandlerid ].Clients[ clientid ].talkStatus = status;
    }
    return;
  }

  if ( cmd[ 0 ] == "notifyclientids" )
  {
    //LOG_DBG("client ids");
    return;
  }

  if ( cmd[ 0 ] == "notifyclientmoved" )
  {
    TS32 clientid = -1;
    TS32 channelid = -1;
    for ( TS32 x = 1; x < cmd.NumItems(); x++ )
    {
      if ( cmd[ x ].Find( "ctid=" ) == 0 )
        cmd[ x ].Scan( "ctid=%d", &channelid );
      if ( cmd[ x ].Find( "clid=" ) == 0 )
        cmd[ x ].Scan( "clid=%d", &clientid );
    }

    if ( clientid >= 0 && channelid >= 0 )
    {
      handlers[ schandlerid ].Clients[ clientid ].channelid = channelid;
      if ( channelid == handlers[ schandlerid ].Clients[ handlers[ schandlerid ].myclientid ].channelid )
      {
        handlers[ schandlerid ].Clients[ clientid ].lastTalkTime = GetTime();
        handlers[ schandlerid ].Clients.SortByValue( ClientTalkTimeSorter );
      }
    }
    return;
  }

  if ( cmd[ 0 ] == "notifycurrentserverconnectionchanged" )
  {
    currentHandlerID = schandlerid;
    return;
  }

  if ( cmd[ 0 ] == "notifyconnectstatuschange" )
  {
    for ( TS32 x = 1; x < cmd.NumItems(); x++ )
    {
      if ( cmd[ x ].Find( "status=disconnected" ) == 0 )
      {
        handlers[ schandlerid ].Connected = false;
        handlers[ schandlerid ].Channels.Flush();
        handlers[ schandlerid ].Clients.Flush();
      }

      if ( cmd[ x ].Find( "status=connecting" ) == 0 )
      {
        handlers[ schandlerid ].Connected = false;
        handlers[ schandlerid ].Channels.Flush();
        handlers[ schandlerid ].Clients.Flush();
      }

      if ( cmd[ x ].Find( "status=connected" ) == 0 )
      {
        handlers[ schandlerid ].Connected = true;
        CommandResponse use = SendCommand( CString::Format( "use %d", schandlerid ) );
        if ( !use.ErrorCode )
        {
          currentHandlerID = schandlerid;
          CommandResponse whoami = SendCommand( "whoami" );
          if ( !whoami.ErrorCode )
          {
            TS32 clid = 0, cid = 0;
            whoami.Lines[ 0 ].Scan( "clid=%d cid=%d", &clid, &cid );
            handlers[ schandlerid ].Clients[ clid ].clientid = clid;
            handlers[ schandlerid ].Clients[ clid ].channelid = cid;
            handlers[ schandlerid ].myclientid = clid;
          }
          CommandResponse serverName = SendCommand( "servervariable virtualserver_name" );
          if ( !serverName.ErrorCode )
            if ( serverName.Lines[ 0 ].Find( "virtualserver_name=" ) == 0 )
              handlers[ schandlerid ].name = unescape( serverName.Lines[ 0 ].Substring( 19 ) );
        }
      }
    }
    return;
  }

  if ( cmd[ 0 ] == "channellist" )
  {
    ProcessChannelList( s, schandlerid );
    return;
  }

  if ( cmd[ 0 ] == "channellistfinished" )
  {
    //LOG_DBG("channellist notifications over");
    return;
  }

  if ( cmd[ 0 ] == "notifycliententerview" )
  {
    ProcessClientList( s, schandlerid );
    //LOG_DBG("client entered view");
    return;
  }

  if ( cmd[ 0 ] == "notifyclientleftview" )
  {
    TS32 clientid = -1;
    for ( TS32 x = 1; x < cmd.NumItems(); x++ )
    {
      if ( cmd[ x ].Find( "clid=" ) == 0 )
        cmd[ x ].Scan( "clid=%d", &clientid );
    }
    if ( clientid >= 0 )
      handlers[ schandlerid ].Clients.Delete( clientid );
    return;
  }

  if ( cmd[ 0 ] == "notifychannelgrouplist" )
  {
    //LOG_DBG("channel group list");
    return;
  }

  if ( cmd[ 0 ] == "notifyservergrouplist" )
  {
    //LOG_DBG("server group list");
    return;
  }

  if ( cmd[ 0 ] == "notifyclientneededpermissions" )
  {
    //LOG_DBG("client needed permissions");
    return;
  }

  if ( cmd[ 0 ] == "notifyclientupdated" )
  {
    TS32 clientid = -1;
    for ( TS32 x = 1; x < cmd.NumItems(); x++ )
    {
      if ( cmd[ x ].Find( "clid=" ) == 0 )
        cmd[ x ].Scan( "clid=%d", &clientid );
    }

    if ( clientid >= 0 )
    {
      for ( TS32 x = 1; x < cmd.NumItems(); x++ )
      {
        if ( cmd[ x ].Find( "client_input_muted=" ) == 0 )
          cmd[ x ].Scan( "client_input_muted=%d", &handlers[ schandlerid ].Clients[ clientid ].inputmuted );
        if ( cmd[ x ].Find( "client_output_muted=" ) == 0 )
          cmd[ x ].Scan( "client_output_muted=%d", &handlers[ schandlerid ].Clients[ clientid ].outputmuted );
      }
    }
    return;
  }

  if ( cmd[ 0 ] == "notifychannelsubscribed" )
  {
    //LOG_DBG("channel subscribed");
    return;
  }

  if ( cmd[ 0 ] == "notifychanneledited" )
  {
    //LOG_DBG("channel edited");
    return;
  }

  if ( cmd[ 0 ] == "notifyclientchannelgroupchanged" )
  {
    //LOG_DBG("client channel group changed");
    return;
  }

  //LOG_DBG("unhandled notification %s", cmd[0].GetPointer());

}

CString TS3Connection::ReadLine()
{
  if ( !connection.IsConnected() )
    return "";

  CString lne = connection.ReadLine();
  if ( connection.GetLength() )
  {
    char c;
    if ( connection.Peek( &c, 1 ) )
      if ( c == '\r' )
        c = connection.ReadByte();
  }
  return lne;
}

void TS3Connection::ProcessChannelList( CString& channeldata, TS32 handler )
{
  CStringArray channels = channeldata.Explode( "|" );
  for ( TS32 x = 0; x < channels.NumItems(); x++ )
  {
    CStringArray channelData = channels[ x ].ExplodeByWhiteSpace();
    TS3Channel channel;
    for ( TS32 y = 0; y < channelData.NumItems(); y++ )
    {
      if ( channelData[ y ].Find( "cid=" ) == 0 )
      {
        channelData[ y ].Scan( "cid=%d", &channel.id );
        continue;
      }
      if ( channelData[ y ].Find( "pid=" ) == 0 )
      {
        channelData[ y ].Scan( "pid=%d", &channel.parentid );
        continue;
      }
      if ( channelData[ y ].Find( "cpid=" ) == 0 )
      {
        channelData[ y ].Scan( "cpid=%d", &channel.parentid );
        continue;
      }
      if ( channelData[ y ].Find( "channel_order=" ) == 0 )
      {
        channelData[ y ].Scan( "channel_order=%d", &channel.order );
        continue;
      }
      if ( channelData[ y ].Find( "channel_name=" ) == 0 )
      {
        channel.name = unescape( channelData[ y ].Substring( 13 ) );
        continue;
      }
    }
    handlers[ handler ].Channels[ channel.id ] = channel;
  }
}

void TS3Connection::ProcessClientList( CString& clientdata, TS32 handler )
{
  bool needsSort = false;

  CStringArray channels = clientdata.Explode( "|" );
  for ( TS32 x = 0; x < channels.NumItems(); x++ )
  {
    CStringArray clientData = channels[ x ].ExplodeByWhiteSpace();
    TS3Client client;
    for ( TS32 y = 0; y < clientData.NumItems(); y++ )
    {
      if ( clientData[ y ].Find( "cid=" ) == 0 )
      {
        clientData[ y ].Scan( "cid=%d", &client.channelid );
        continue;
      }
      if ( clientData[ y ].Find( "ctid=" ) == 0 )
      {
        clientData[ y ].Scan( "ctid=%d", &client.channelid );
        continue;
      }
      if ( clientData[ y ].Find( "client_channel_group_inherited_channel_id=" ) == 0 )
      {
        clientData[ y ].Scan( "client_channel_group_inherited_channel_id=%d", &client.channelid );
        continue;
      }
      if ( clientData[ y ].Find( "clid=" ) == 0 )
      {
        clientData[ y ].Scan( "clid=%d", &client.clientid );
        continue;
      }
      if ( clientData[ y ].Find( "client_input_muted=" ) == 0 )
      {
        clientData[ y ].Scan( "client_input_muted=%d", &client.inputmuted );
        continue;
      }
      if ( clientData[ y ].Find( "client_output_muted=" ) == 0 )
      {
        clientData[ y ].Scan( "client_output_muted=%d", &client.outputmuted );
        continue;
      }
      if ( clientData[ y ].Find( "client_nickname=" ) == 0 )
      {
        client.name = unescape( clientData[ y ].Substring( 16 ) );
        //LOG_DBG("New cliend: %s (%d) in channel %d", client.name.GetPointer(), client.clientid, client.channelid);
        continue;
      }
    }
    handlers[ handler ].Clients[ client.clientid ] = client;
    handlers[ handler ].Clients[ client.clientid ].lastTalkTime = GetTime();
    if ( handlers[ handler ].myclientid && handlers[ handler ].Clients[ client.clientid ].channelid == handlers[ handler ].Clients[ handlers[ handler ].myclientid ].channelid )
      needsSort = true;
  }

  if ( needsSort )
    handlers[ handler ].Clients.SortByValue( ClientTalkTimeSorter );
}

CString TS3Connection::unescape( CString string )
{
  CString result;
  for ( TU32 x = 0; x < string.Length(); x++ )
  {
    if ( string[ x ] == '\\' )
    {
      if ( x == string.Length() - 1 )
        break;

      if ( x )
        result += CString( string, x );

      switch ( string[ x + 1 ] )
      {
      case '\\':
        result += "\\";
      case '/':
        result += "/";
      case 's':
        result += " ";
        break;
      case 'p':
        result += "|";
        break;
      case 'a':
        result += "\a";
        break;
      case 'b':
        result += "\b";
        break;
      case 'f':
        result += "\f";
        break;
      case 'n':
        result += "\n";
        break;
      case 'r':
        result += "\r";
        break;
      case 't':
        result += "\t";
        break;
      case 'v':
        result += "\v";
        break;
      default:
        break;
      }

      string = string.Substring( x + 2 );
      x = -1;
    }
  }

  return result + string;
}

TBOOL TS3Connection::IsConnected()
{
  return connection.IsConnected();
}

//CString TS3Connection::GetIncoming()
//{
//	TS32 dataLength = (TS32)connection.GetLength();
//
//	if (!dataLength)
//		return "";
//
//	TU8 *data = new TU8[dataLength];
//	if (connection.Read(data, dataLength) != dataLength)
//	{
//		SAFEDELETE(data);
//		return "";
//	}
//
//	CString dataString((TCHAR*)data, dataLength);
//	SAFEDELETE(data);
//
//	return dataString;
//}
