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

#ifndef _NETSERVERPLAYER_HPP_
#define _NETSERVERPLAYER_HPP_ 1

// Forward declarations
class NetServerPlayer;

#include "player.hpp"
#include "serverhandler.hpp"
#include <wx/socket.h>

// Server side network player
class NetServerPlayer: public HumanPlayer {
public:
  NetServerPlayer( const wxString& name,
		   GamePos* gamepos,
		   wxSocketBase* socket );
  virtual ~NetServerPlayer();
  virtual void NewGame( Game* game );
  virtual void NewRound( Card* trumph, Player* owner );
  virtual void NewTurn( Player* starter );
  virtual void Turn( Player* player, Card* card );
  virtual void TurnEnd( const Player* winner, const CardList& played );
  virtual void OnMyTurn( Game* game, const CardList& played );
protected:
  friend class ServerHandler;
  wxSocketBase* GetSocket() { return m_socket; }
  wxString reading;
private:
  wxSocketBase* m_socket;
};

#endif // _NETSERVERPLAYER_HPP_
