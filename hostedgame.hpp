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

#ifndef _HOSTEDGAME_HPP_
#define _HOSTEDGAME_HPP_ 1

// Forward declarations
class HostedGame;

#include "game.hpp"

// Hosted game engine class
class HostedGame: public Game
{
public:
  HostedGame( Player* p1,
	      Player* p2,
	      Player* p3,
	      Player* p4,
	      MyCanvas *the_canvas
  );
  virtual void SetPlayerName( Player* player, const wxString& newname );
};

#endif // _HOSTEDGAME_HPP_
