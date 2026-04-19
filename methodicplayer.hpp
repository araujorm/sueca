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

#ifndef _METHODICPLAYER_HPP_
#define _METHODICPLAYER_HPP_ 1

// Forward declarations
class MethodicPlayer;

#include "observingbot.hpp"

#define PIMC_WORLDS 100

// Methodic player: Perfect Information Monte Carlo (PIMC) with a simple,
// rule-based simulator. "Textbook" approach - trusts the law of large
// numbers. For each of our valid moves we sample many possible worlds
// (random distributions of the cards we don't know) and, in each world,
// simulate the rest of the game with a naive but plausible policy for
// every player. The move whose sampled average team score is highest
// wins.
//
// Inherits the observational state (plhasnot, out, released, trumph,
// ...) from ObservingBot, same as SmartPlayer - they're siblings in
// the bot hierarchy, neither derives from the other.
//
// Players in simulation: 0=right(opp), 1=partner, 2=left(opp), 3=us
// Turn order: 3 -> 0 -> 1 -> 2 -> 3 ...
// Our team: 1 and 3. Their team: 0 and 2.
class MethodicPlayer: public ObservingBot
{
public:
	MethodicPlayer( GamePos* gamepos );
	Card* PlayCard( const CardList* played );
protected:
	int SimulateGame( Card* mycard, const CardList* played,
	                  Card** simhands, int* simsizes );
	bool SampleWorld( Card** simhands, int* simsizes );
	// Simulator play policy. Virtual so subclasses can supply a richer
	// heuristic (ExpertPlayer does this to sharpen the Monte Carlo
	// evaluation without touching the surrounding PIMC machinery).
	// sim_out accumulates every card played during the simulation so far
	// (starting from the observed real-game 'out' list), and lead_slot
	// tells the policy which sim slot led the current trick - together
	// they let a subclass know things like "has the ace of this suit
	// come out during the simulation?" or "is the trumph owner still to
	// play?" without peeking into anyone else's sim hand.
	virtual Card* SimPlayCard( Card* hand[], int handsize,
	                           const CardList& trick,
	                           cardsuit_t trumphsuit, bool our_team,
	                           const CardList& sim_out, int lead_slot );
	// Determine winner position and card in a 4-card trick
	int SimTrickWinner( const CardList& trick, cardsuit_t trumphsuit );
	int SimTrickPoints( const CardList& trick );
	// Check if card beats current best
	bool SimBeats( Card* card, Card* best, cardsuit_t trumphsuit );
};

#endif // _METHODICPLAYER_HPP_
