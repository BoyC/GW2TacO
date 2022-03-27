#include "TS3Control.h"
#include "TS3Connection.h"
#include "OverlayConfig.h"
#include "Language.h"


void TS3Control::OnDraw( CWBDrawAPI* API )
{
  CWBFont* f = GetFont( GetState() );
  TS32 size = f->GetLineHeight();

  if ( !teamSpeakConnection.authenticated )
  {
    if ( Config::HasString( "TS3APIKey" ) )
    {
      f->Write( API, DICT( "ts3authfail1" ), CPoint( 0, 0 ), 0xffffffff );
      f->Write( API, DICT( "ts3authfail2" ), CPoint( 0, size ), 0xffffffff );
      f->Write( API, DICT( "ts3authfail3" ), CPoint( 0, size * 2 ), 0xffffffff );
      f->Write( API, DICT( "ts3authfail4" ), CPoint( 0, size * 3 ), 0xffffffff );
    }
    else
    {
      f->Write( API, DICT( "ts3nokeyset1" ), CPoint( 0, 0 ), 0xffffffff );
      f->Write( API, DICT( "ts3nokeyset2" ), CPoint( 0, size ), 0xffffffff );
      f->Write( API, DICT( "ts3nokeyset3" ), CPoint( 0, size * 2 ), 0xffffffff );
    }
  }

  WBSKINELEMENTID playeroff = App->GetSkin()->GetElementID( CString( "ts3playeroff" ) );
  WBSKINELEMENTID playeron = App->GetSkin()->GetElementID( CString( "ts3playeron" ) );
  WBSKINELEMENTID outputoff = App->GetSkin()->GetElementID( CString( "ts3outputmuted" ) );
  WBSKINELEMENTID inputoff = App->GetSkin()->GetElementID( CString( "ts3inputmuted" ) );

  TBOOL LeftAlign = true;
  CRect r = ClientToScreen( GetClientRect() );
  LeftAlign = r.x1 < App->GetXRes() / 2 && r.x2 < App->GetXRes() / 2;

  CRect displayrect = GetClientRect();

  for ( TS32 cnt = 0; cnt < 2; cnt++ )
  {
    TS32 ypos = 0;
    for ( TS32 x = 0; x < teamSpeakConnection.handlers.NumItems(); x++ )
    {
      TS3Connection::TS3Schandler& handler = teamSpeakConnection.handlers[ x ];
      if ( handler.Connected && handler.Clients.HasKey( handler.myclientid ) )
      {
        CPoint p = f->GetTextPosition( handler.name, GetClientRect() - CRect( 0, ypos, 0, 0 ), LeftAlign ? WBTA_LEFT : WBTA_RIGHT, WBTA_TOP, WBTT_NONE, true );
        if ( cnt )
          f->Write( API, handler.name, p );
        ypos += f->GetLineHeight();

        TS32 mychannelid = handler.Clients[ handler.myclientid ].channelid;

        if ( handler.Channels.HasKey( mychannelid ) )
        {
          TS32 participants = 0;
          for ( TS32 y = 0; y < handler.Clients.NumItems(); y++ )
          {
            TS3Connection::TS3Client& cl = handler.Clients[ y ];
            if ( cl.channelid == mychannelid )
              participants++;
          }

          CString channelText = CString::Format( "%s (%d)", handler.Channels[ mychannelid ].name.GetPointer(), participants );

          CPoint p = f->GetTextPosition( channelText, GetClientRect() - CRect( size / 2, ypos, 0, 0 ), LeftAlign ? WBTA_LEFT : WBTA_RIGHT, WBTA_TOP, WBTT_NONE, true );
          if ( cnt )
            f->Write( API, channelText, p );
          ypos += f->GetLineHeight();
        }
        for ( TS32 y = 0; y < handler.Clients.NumItems(); y++ )
        {
          if ( ( ypos + f->GetLineHeight() ) > displayrect.y2 )
            break;

          TS3Connection::TS3Client& cl = handler.Clients.GetByIndex( y );
          if ( cl.channelid == mychannelid )
          {
            WBSKINELEMENTID id = playeroff;
            if ( cl.inputmuted )
              id = inputoff;
            if ( cl.outputmuted )
              id = outputoff;
            if ( cl.talkStatus )
              id = playeron;

            App->GetSkin()->RenderElement( API, id, LeftAlign ? CRect( size / 2, ypos, size / 2 + size - 1, ypos + size - 1 ) : CRect( GetClientRect().Width() - size / 2 - size + 1, ypos, GetClientRect().Width() - size / 2, ypos + size - 1 ) );

            CPoint p = f->GetTextPosition( cl.name, GetClientRect() - CRect( 2 * size, ypos, 2 * size, 0 ), LeftAlign ? WBTA_LEFT : WBTA_RIGHT, WBTA_TOP, WBTT_NONE, true );

            if ( cnt )
              f->Write( API, cl.name, p );

            ypos += f->GetLineHeight();
          }
        }
      }
      ypos += size / 2;
    }

    if ( !cnt )
      DrawBackgroundItem( API, CSSProperties.DisplayDescriptor, CRect( 0, 0, GetClientRect().Width(), ypos ), GetState() );
  }

  DrawBorder( API );
}

TS3Control::TS3Control( CWBItem* Parent, CRect Position ) : CWBItem( Parent, Position )
{}

TS3Control::~TS3Control()
{

}

CWBItem* TS3Control::Factory( CWBItem* Root, CXMLNode& node, CRect& Pos )
{
  return new TS3Control( Root, Pos );
}

TBOOL TS3Control::IsMouseTransparent( CPoint& ClientSpacePoint, WBMESSAGE MessageType )
{
  return true;
}
