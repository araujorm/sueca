#include "serverhandler.hpp"
#include "main.hpp"
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>

WX_DEFINE_LIST( SockServList );

// Commands accepted
CommandClass::CommandClass():
  CommandMap( N_COMMANDS )
{
  CommandClass& me = *this;
  // Commands
  me["position"] = COM_POS;
  me["play"] = COM_PLAY;
  me["name"] = COM_NAME;
  me["say"] = COM_SAY;
}

// Server sockets handler implementation
CommandClass ServerHandler::command_type;

BEGIN_EVENT_TABLE( ServerHandler, wxEvtHandler )
  EVT_SOCKET( SERVER_ID, ServerHandler::OnServerEvent )
  EVT_SOCKET( SOCKET_ID, ServerHandler::OnSocketEvent )
END_EVENT_TABLE();

/*
ServerHandler::ServerHandler():
  wxEvtHandler()
{}
*/

ServerHandler::~ServerHandler()
{
  StopServer();
  CloseClients();
}

bool ServerHandler::StartServer( wxSockAddress& address )
{
  wxSocketServer* sock = new wxSocketServer( address );
  if( ! sock->Ok() ) {
    sock->Destroy();
    return false;
  }
  serv_sockets.Append( sock );
  // Watch activity
  sock->SetEventHandler( *this, SERVER_ID );
  sock->SetNotify( wxSOCKET_CONNECTION_FLAG |
		   wxSOCKET_LOST_FLAG );
  sock->Notify( true );
  return true;
}

void ServerHandler::StopServer()
{
  for( SockServList::Node* node = serv_sockets.GetFirst(); node; node = node->GetNext() )
    node->GetData()->Destroy();
  serv_sockets.Clear();
}

void ServerHandler::CloseClients()
{
  for( SockBaseList::Node* node = client_sockets.GetFirst(); node; node = node->GetNext() ) {
    NetServerPlayer* player = (NetServerPlayer*)node->GetData()->GetClientData();
    delete player;  // Destroys socket and removes it from client_sockets
  }
}

void ServerHandler::ToAll( const wxString& msg )
{
  for( SockBaseList::Node* node = client_sockets.GetFirst(); node; node = node->GetNext() )
    SocketPrintln( node->GetData(), msg );
}

NetServerPlayer* ServerHandler::NewPlayer( wxSocketBase* socket,
					   wxString& name )
{
  // First time telling the name
  ServerDialog* diag = wxGetApp().servdlg;
  if( ! ValidName( name ) || ! diag )
    // TODO: players joining in the middle of the game
    return NULL;
  wxString posstr;
  Position* pos = diag->GetNotMatchingPos( posstr, NULL );
  if( ! pos ) {
    SocketPrintln( socket, "full" );
    return NULL;
  }
  NetServerPlayer* player =
    new NetServerPlayer( name, pos->gpos->Clone(), socket );
  diag->SetPosition( pos, player );
  socket->SetClientData( player );
  // Tell about the positions
  SocketPrintln( socket, "position:" + posstr );
  ToAll( wxString::Format( "name:%s:%s", pos->gpos->GetName().c_str(), name.c_str() ) );
  // From now on this socket will be a valid one
  client_sockets.Append( socket );
  // Allow game to begin
  diag->ok_button->Enable( true );
  diag->ReLayout();
  return player;
}

void ServerHandler::OnSocketEvent( wxSocketEvent& event )
{
  wxSocketBase* socket = event.GetSocket();
  NetServerPlayer* player = (NetServerPlayer*)event.GetClientData();
  ServerDialog* diag = wxGetApp().servdlg;
  switch( event.GetSocketEvent() ) {
  case wxSOCKET_LOST:
    if( player ) {
      // We can't delete the player right now but need to clear client_sockets
      // from the socket
      client_sockets.DeleteObject( socket );
      Position* pos;
      if( diag && ( pos = diag->GetPlayerPosition( player ) ) ) {
	if( ! client_sockets.GetCount() ) {
	  // Prevent game from begining
	  diag->ok_button->Enable( false );
	  // Disable connected related controls if not listening and this
	  // client is the last one to leave
	  if( ! serv_sockets.GetCount() )
	    for( wxWindowList::Node* node = diag->connectedlist.GetFirst(); node; node = node->GetNext() )
	      node->GetData()->Enable( false );
	}
	diag->SetPosition( pos, NULL );
	diag->ReLayout();
      }
      Game* game = wxGetApp().GetGame();
      if( game )
	game->ReplacePlayer( player, wxGetApp().GetBotPlayer( player->GetGamePosCopy() ) );
      // Tell other players
      ToAll( wxString::Format( "name:%s:", player->GetNamePosStr().c_str() ) );
      delete player;  // Only destroys the socket we have already removed
    }
    break;
  case wxSOCKET_INPUT:
    {
      wxString temp;  // for new players
      CommandList* comlist = GetCommands( socket, command_type, player ? player->reading : temp );
      for( CommandList::Node* node = comlist->GetFirst(); node; node = node->GetNext() ) {
	Command* com = node->GetData();
	wxArrayString& args = com->args;
	if( ! client_sockets.Find( socket ) || ! player ) {
	  if( com->com == COM_NAME && args.GetCount() > 1 &&
	      ( player = NewPlayer( socket, args[1] ) ) ) {
	    // A valid player was found and is now ready
	    player->reading = temp;
	    continue;
	  }
	  // Either we are full or we have a clown here ;)
	  socket->Destroy();
	  return;
	}
	switch( com->com ) {
	case COM_POS:
	  if( args.GetCount() > 1 && diag ) {
	    // Check if position is valid and free
	    Position* pos, *oldpos = diag->GetPlayerPosition( player );
	    PosMap::iterator it = diag->positions.find( args[1] );
	    if( it != diag->positions.end() && ( pos = it->second ) &&
		oldpos && ! pos->player ) {
	      diag->SetPosition( pos, oldpos->player );
	      diag->SetPosition( oldpos, NULL );
	      diag->ReLayout();
	      // Send new positions to everyone
	      for( PosMap::iterator i = diag->positions.begin();
		   i != diag->positions.end();
		   i++ ) {
		NetServerPlayer* player = i->second->player;
		if( player )
		  SendPositions( player );
	      }
	      diag->ReLayout();
	    }
	    else
	      // Tell this client the position hasn't changed by telling the
	      // current position information
	      SendPositions( player );
	  }
	  break;
	case COM_PLAY:
	  if( args.GetCount() > 1 ) {
	    Game* game = wxGetApp().GetGame();
	    Card* card = game->GetDeck().cardmap[args[1]];
	    // Ignore invalid card strings
	    if( game && card )
	      switch( wxGetApp().GetGame()->PlayMove( player, card ) ) {
	      case MOVE_INVALID:
		SocketPrintln( socket, "invalid" );
		continue;
	      case MOVE_TURN:
		SocketPrintln( socket, "noturn" );
		continue;
	      default:  // MOVE_OK
		break;
	      }
	  }
	  break;
	case COM_NAME:
	  if( args.GetCount() > 1 ) {
	    wxString& name = args[1];
	    // Ignore invalid names
	    if( ValidName( name ) ) {
	      player->SetName( name );
	      // Check if we are still on the server creation dialog
	      Position* pos;
	      if( diag && ( pos = diag->GetPlayerPosition( player ) ) ) {
		pos->label->SetLabel( name );
		diag->ReLayout();
		wxString& identifier = pos->gpos->GetName();
		ToAll( wxString::Format( "name:%s:%s", identifier.c_str(), name.c_str() ) );
	      }
	      // TODO: Check if we are in the middle of a game
	    }
	  }
	  break;
	case COM_SAY:
	  if( diag && args.GetCount() > 1 ) {
	    wxString &who = player->GetName();
	    wxString text = args[1];
	    for( size_t i = 2; i < args.GetCount(); i++ )
	      text += ":" + args[i];
	    diag->chat->SayInChat( who, text );
	    // (Re)send to the clients
	    ToAll( wxString::Format( "say:%s:%s", who.c_str(), text.c_str() ) );
	  }
	  break;
	}
      }
    }
    break;
  default:
    break;
  }
}

void ServerHandler::OnServerEvent( wxSocketEvent& event )
{
  wxSocketServer* serv = (wxSocketServer*)event.GetSocket();
  // Check for socket validity
  if( ! serv_sockets.Find( serv ) )
    return;
  ServerDialog* diag = wxGetApp().servdlg;
  switch( event.GetSocketEvent() ) {
  case wxSOCKET_CONNECTION:
    {
      wxSocketBase* client = serv->Accept( false );
      if( client ) {
	// Greeting message
	SocketPrintln( client, VERSION_STRING );
	// Watch activity
	client->SetEventHandler( *this, SOCKET_ID );
	client->SetNotify( wxSOCKET_INPUT_FLAG |
			   wxSOCKET_LOST_FLAG );
	client->Notify( true );
        client->SetFlags( wxSOCKET_NOWAIT );
      }
    }
    break;
  case wxSOCKET_LOST:
    serv_sockets.DeleteObject( serv );
    serv->Destroy();
    if( ! serv_sockets.GetCount() && diag )
      diag->EndListen();
    break;
  default:
    break;
  }
}

void ServerHandler::SendPositions( NetServerPlayer* player )
{
  wxString str;
  wxGetApp().servdlg->GetNotMatchingPos( str, player );
  SocketPrintln( player->GetSocket(), "position:" + str );
}
