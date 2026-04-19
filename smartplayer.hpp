/*
sueca - An implementation of the Portuguese game "Sueca" in C++ and wxWidgets
Copyright (C) 2003-2026 Rodrigo Araujo

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

#include "observingbot.hpp"

// Smart player: rule-based decisions on top of the observational state
// tracked by ObservingBot. Leads and follows with targeted heuristics
// (cash aces, save the trump ace, dump high to partner's winners,
// etc.) without running any simulation.
class SmartPlayer: public ObservingBot
{
public:
	SmartPlayer( GamePos* gamepos );
	Card* PlayCard( const CardList* played );
protected:
	Card* PlayFirst();
	Card* PlayFollowing( const CardList* played );
};

#endif // _SMARTPLAYER_HPP_
