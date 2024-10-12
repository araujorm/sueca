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

#ifndef _REMOTEGAME_HPP_
#define _REMOTEGAME_HPP_ 1

// Forward declarations
class RemoteGame;

#include "game.hpp"
#include "remotehandler.hpp"

// Remote game engine class
class RemoteGame: public Game
{
public:
  RemoteGame( LocalPlayer* p1,
	      HumanPlayer* p2,
	      HumanPlayer* p3,
	      HumanPlayer* p4,
	      MyCanvas *the_canvas,
	      RemoteHandler* handler );
  virtual ~RemoteGame();
  // Called by the local player
  virtual movestatus_t PlayMove( Player *player, Card *card );
  // These are to be ignored
  virtual void NewRound() {}
  virtual void EndTurn() {}
protected:
  friend class RemoteHandler;
  void PlayRemoteMove( Player *player, Card *card );
  void NewRound( Card* trumph, Player* owner, CardList& localcards );
  void EndTurn( Player* winner );
private:
  RemoteHandler* m_handler;
  LocalPlayer* localplayer;
  HumanPlayer* fake_players[3];
};

#endif // _REMOTEGAME_HPP_
