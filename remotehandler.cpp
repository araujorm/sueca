/*
sueca - An implementation of the Portuguese game "Sueca" in C++ and wxWidgets
Copyright (C) 2003-2024 Rodrigo Araujo

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "remotehandler.hpp"
#include "main.hpp"
#include <wx/msgdlg.h>

DEFINE_EVENT_TYPE( FINISH_REMOTE_HANDLER_TYPE );

// Responses recognized
ResponseClass::ResponseClass():
  CommandMap( N_RESPONSES )
{
  ResponseClass& me = *this;
  // Commands
  me[VERSION_STRING] = RESP_FIRST;
  me["position"] = RESP_POS;
  me["game"] = RESP_GAME;
  me["round"] = RESP_ROUND;
  me["play"] = RESP_PLAY;
  me["winner"] = RESP_WINNER;
  me["noturn"] = RESP_NOTURN;
  me["invalid"] = RESP_INVMOVE;
  me["name"] = RESP_NAME;
  me["say"] = RESP_SAY;
}

// Remote network game handler implementation
ResponseClass RemoteHandler::response_type;

BEGIN_EVENT_TABLE( RemoteHandler, wxEvtHandler )
  EVT_SOCKET( SOCKET_ID, RemoteHandler::OnSocketEvent )
END_EVENT_TABLE();

RemoteHandler::RemoteHandler(): wxEvtHandler(), m_socket( NULL ) {}

RemoteHandler::~RemoteHandler()
{
  if( m_socket )
    m_socket->Destroy();
}

void RemoteHandler::SetSocket( wxSocketBase* socket )
{
  m_authenticated = false;
  if( m_socket )
    m_socket->Destroy();
  if( ( m_socket = socket ) ) {
    m_connected = m_socket->IsConnected();
    m_socket->SetEventHandler( *this, SOCKET_ID );
    m_socket->SetNotify( wxSOCKET_CONNECTION_FLAG |
			 wxSOCKET_INPUT_FLAG |
			 wxSOCKET_LOST_FLAG );
    m_socket->Notify( true );
  }
  else
    m_connected = false;
}

void RemoteHandler::TerminateConnection()
{
  RemoteDialog* diag = wxGetApp().rmtdlg;
  if( diag ) {
    if( m_connected )
      diag->ConnectionEndWarn( "Connection lost.");
    else
      diag->ConnectionEndWarn( "Connection failed.");
    SetSocket( NULL );
  }
  else if( m_connected ) {
    wxMessageBox( "Connection lost.", "Game Aborted", wxOK | wxICON_EXCLAMATION );
    wxGetApp().EndGame();
  }
}

void RemoteHandler::OnSocketEvent( wxSocketEvent& event )
{
  RemoteDialog* diag = wxGetApp().rmtdlg;
  switch( event.GetSocketEvent() ) {
  case wxSOCKET_CONNECTION:
    if( diag ) {
      // Enable connected related controls
      for( wxWindowList::Node* node = diag->connectedlist.GetFirst(); node; node = node->GetNext() )
	node->GetData()->Enable( true );
      diag->chat->Write( "*** Connection established.");
      diag->connect_button->SetLabel( "Disc&onnect" );
      diag->ReLayout();
    }
    m_connected = true;
    break;
  case wxSOCKET_LOST:
    TerminateConnection();
    break;
  case wxSOCKET_INPUT:
    {
      CommandList* comlist = GetCommands( m_socket, response_type, m_reading );
      RemoteGame* game = (RemoteGame*)wxGetApp().GetGame();
      for( CommandList::Node* node = comlist->GetFirst(); node; node = node->GetNext() ) {
	Command* com = node->GetData();
	wxArrayString& args = com->args;
	if( m_authenticated )
	  switch( com->com ) {
	  case RESP_POS:
	    if( args.GetCount() > 7 ) {
	      // Arg 1 is our position
	      RemotePosition* pos = diag->SetPosition( wxGetApp().GetLocalPlayerName(), args[1] );
	      if( ! pos ) {
		// Something went wrong with the server
		TerminateConnection();
		return;
	      }
	      pos->button->SetValue( true );
	      pos->button->Enable( true );
	      for( int i = 2; i < 8; i += 2 )
		if( ! diag->SetPosition( args[i], args[i+1] ) ) {
		  // Something is wrong with this server indeed!
		  TerminateConnection();
		  return;
		}
	      diag->ReLayout();
	    }
	    break;
	  case RESP_GAME:
	    if( args.GetCount() > 7 && diag ) {
	      Sueca& app = wxGetApp();
	      LocalPlayer* p1;
	      HumanPlayer* p2; HumanPlayer* p3; HumanPlayer* p4;
	      pmap[args[1]] = p1 = new LocalPlayer( app.GetLocalPlayerName(), new GamePosP1() );
	      pmap[args[3]] = p2 = new HumanPlayer( args[2], new GamePosP2() );
	      pmap[args[5]] = p3 = new HumanPlayer( args[4], new GamePosP3() );
	      pmap[args[7]] = p4 = new HumanPlayer( args[6], new GamePosP4() );
	      app.NewGame( new RemoteGame( p1, p2, p3, p4,
					   app.GetFrame()->canvas, this ),
			   p1 );
	      diag->Done( false );
	    }
	    break;
	  case RESP_ROUND:
	    if( args.GetCount() > 12 && game ) {
	      CardMap& cardmap = game->GetDeck().cardmap;
	      CardList cards;
	      CardMap::iterator cardit;
	      for( int i = 1; i <= 10; i++ ) {
		cardit = cardmap.find( args[i] );
		if( cardit == cardmap.end() ) {
		  TerminateConnection();
		  return;
		}
		cards.Append( cardit->second );
	      }
	      cardit = cardmap.find( args[11] );
	      PlayerMap::iterator plit = pmap.find( args[12] );
	      if( cardit == cardmap.end() || plit == pmap.end() ) {
		TerminateConnection();
		return;
	      }
	      game->NewRound( cardit->second, plit->second, cards );
	    }
	    break;
	  case RESP_PLAY:
	    if( game && args.GetCount() > 2) {
	      CardMap& cardmap = game->GetDeck().cardmap;
	      PlayerMap::iterator plit = pmap.find( args[1] );
	      CardMap::iterator cardit = cardmap.find( args[2] );
	      if( cardit == cardmap.end() || plit == pmap.end() ) {
		// Invalid strings
		TerminateConnection();
		return;
	      }
	      game->PlayRemoteMove( plit->second, cardit->second );
	    }
	    break;
	  case RESP_WINNER:
	    if( game && args.GetCount() > 1) {
	      PlayerMap::iterator plit = pmap.find( args[1] );
	      if( plit == pmap.end() ) {
		// Invalid player identifier
		TerminateConnection();
		return;
	      }
	      game->EndTurn( plit->second );
	    }
	    break;
	  case RESP_NOTURN:
	    wxGetApp().GetFrame()->canvas->NotTurnWarning();
	    break;
	  case RESP_INVMOVE:
	    wxGetApp().GetFrame()->canvas->InvalidLocalMove();
	    break;
	  case RESP_NAME:  // Empty name means player has left
	    if( args.GetCount() > 2 ) {
	      if( diag ) {
	        // In lobby
	        diag->SetPosition( args[2], args[1] );
	        diag->ReLayout();
	      }
	      else if ( game ) {
	        // In-game
	        PlayerMap::iterator plit = pmap.find( args[1] );
	        if( plit == pmap.end() ) {
		  // Invalid player identifier
		  TerminateConnection();
		  return;
	        }
		Player* player = plit->second;
		// Ignore remote attempts to change our name
		if ( player != wxGetApp().GetLocalPlayer() )
		  game->SetPlayerName( player, args[2] );
	      }
	    }
	    break;
	  case RESP_SAY:
	    if( diag && args.GetCount() > 2 ) {
	      wxString &who = args[1];
	      wxString text = args[2];
	      for( size_t i = 3; i < args.GetCount(); i++ )
		text += ":" + args[i];
	      diag->chat->SayInChat( who, text );
	    }
	    // TODO: implement chat while in game
	    break;
	  }
	else if( com->com == RESP_FIRST ) {
	  if( args.GetCount() != 1 || args[0] != VERSION_STRING ) {
	    // Different server version
	    TerminateConnection();
	    return;
	  }
	  // Send our name
	  Send( "name:" + wxGetApp().GetLocalPlayerName() );
	  m_authenticated = true;
	}
      }
    }
    break;
  default:
    break;
  }
}

void RemoteHandler::Send( wxString message )
{
  if( m_socket )
    SocketPrintln( m_socket, message );
}
