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

#include "netserverplayer.hpp"
#include "main.hpp"

// Server side network player implementation
NetServerPlayer::NetServerPlayer( const wxString& name,
				  GamePos* gamepos,
				  wxSocketBase* socket ):
  HumanPlayer( name, gamepos ), m_socket( socket ) {}

NetServerPlayer::~NetServerPlayer()
{
  // Unregister on server handler
  wxGetApp().servhandler->client_sockets.DeleteObject( m_socket );
  m_socket->Destroy();
}

void NetServerPlayer::NewGame( Game* game )
{
  PlayerIterator* players = game->GetPlayers();
  // First one should be this player's position
  Player* player;
  while( ( player = players->GetNext() ) != this );
  wxString players_str = player->GetNamePosStr();
  while( ( player = players->GetNext() ) != this )
    players_str +=  ":" + player->GetName() + ":" + player->GetNamePosStr();
  SocketPrintln( m_socket,
		 wxString::Format( "game:%s", players_str.c_str() ) );
  // Discard the iterator
  delete players;
}

void NetServerPlayer::NewRound( Card* trumph, Player* owner )
{
  CardList::Node* node = GetHand().GetFirst();
  wxString hand_str = node->GetData()->ShortStr();
  node = node->GetNext();
  do
    hand_str += ":" + node->GetData()->ShortStr();
  while( ( node = node->GetNext() ) );
  SocketPrintln( m_socket,
		 wxString::Format( "round:%s:%s:%s",
				   hand_str.c_str(),
				   trumph->ShortStr().c_str(),
				   owner->GetNamePosStr().c_str() ) );
}

void NetServerPlayer::NewTurn( Player* starter )
{
  SocketPrintln( m_socket,
		 wxString::Format( "turn:%s",
				   starter->GetNamePosStr().c_str() ) );
}

void NetServerPlayer::Turn( Player *player, Card* card )
{
  SocketPrintln( m_socket,
		 wxString::Format( "play:%s:%s",
				   player->GetNamePosStr().c_str(),
				   card->ShortStr().c_str() ) );
}

void NetServerPlayer::TurnEnd( const Player* winner, const CardList& played )
{
  SocketPrintln( m_socket,
		 wxString::Format( "winner:%s", winner->GetNamePosStr().c_str() ) );
}

void NetServerPlayer::OnMyTurn( Game* game, const CardList& played )
{
  SocketPrintln( m_socket, "play" );
}
