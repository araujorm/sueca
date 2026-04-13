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

#ifndef _EXPERTPLAYER_HPP_
#define _EXPERTPLAYER_HPP_ 1

// Forward declarations
class ExpertPlayer;

#include "smartplayer.hpp"

#define PIMC_WORLDS 100

// Expert player: uses Perfect Information Monte Carlo sampling
// Players in simulation: 0=right(opp), 1=partner, 2=left(opp), 3=us
// Turn order: 3 -> 0 -> 1 -> 2 -> 3 ...
// Our team: 1 and 3. Their team: 0 and 2.
class ExpertPlayer: public SmartPlayer
{
public:
	ExpertPlayer( GamePos* gamepos );
	Card* PlayCard( const CardList* played );
private:
	int SimulateGame( Card* mycard, const CardList* played,
	                  Card** simhands, int* simsizes );
	bool SampleWorld( Card** simhands, int* simsizes );
	// Heuristic play for simulation - knows about teams and trumps
	Card* SimPlayCard( Card* hand[], int handsize, const CardList& trick,
	                   cardsuit_t trumphsuit, bool our_team );
	// Determine winner position and card in a 4-card trick
	int SimTrickWinner( const CardList& trick, cardsuit_t trumphsuit );
	int SimTrickPoints( const CardList& trick );
	// Check if card beats current best
	bool SimBeats( Card* card, Card* best, cardsuit_t trumphsuit );
};

#endif // _EXPERTPLAYER_HPP_
