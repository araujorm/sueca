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

#ifndef _SMARTPLAYER_HPP_
#define _SMARTPLAYER_HPP_ 1

// Forward declarations
class SmartPlayer;

#include "player.hpp"
#include "cards.hpp"

// Smart player: tries to be a good player
enum plindex_t { SBOT_RIGHT = 0, SBOT_PARTNER, SBOT_LEFT, SBOT_THIS };
class SmartPlayer: public BotPlayer
{
public:
  SmartPlayer( GamePos* gamepos );
  ~SmartPlayer();
  void NewGame( Game* game );
  void NewRound( Card* newtrumph, Player* newowner );
  void NewTurn( Player* starter );
  void TurnEnd( const Player* winner, const CardList& played );
  Card* PlayCard( const CardList* played );
protected:
  Game* thegame;
  PlayerIterator* players;
  Player* turnstarter;
  CardList out;
  int n_out[4];
  CardList bysuit[4];
  Player* trumphowner;
  Card* trumph;
  Card* played_card;
  bool plhasnot[3][4];
  unsigned short released[3][4];
  Player* left;
  Player* partner;
  Player* right;
  plindex_t PlayerIndex( Player* player );
  bool IsOut( int type_id, int suit_id );
};

#endif // _SMARTPLAYER_HPP_

