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

#ifndef _OBSERVINGBOT_HPP_
#define _OBSERVINGBOT_HPP_ 1

// Forward declarations
class ObservingBot;

#include "player.hpp"
#include "cards.hpp"

// Indices into the per-opponent arrays (plhasnot, released). SBOT_THIS
// is only used as a sentinel for "this is us, skip tracking".
enum plindex_t { SBOT_RIGHT = 0, SBOT_PARTNER, SBOT_LEFT, SBOT_THIS };

// Shared base for bot players that maintain observational state during
// a round (who has played what, who's shown void of which suit, where
// the public trumph card sits, etc.) and the generic helpers that any
// concrete strategy needs on top of that state. This class does not
// implement PlayCard, so it inherits the pure-virtual declaration from
// BotPlayer and cannot be instantiated directly - it's only meant as a
// base for concrete strategies (SmartPlayer, MethodicPlayer, ...).
class ObservingBot: public BotPlayer
{
public:
	ObservingBot( GamePos* gamepos );
	~ObservingBot();
	void NewGame( Game* game );
	void NewRound( Card* newtrumph, Player* newowner );
	void NewTurn( Player* starter );
	void TurnEnd( const Player* winner, const CardList& played );
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
	// Helpers for play decisions
	Player* CurrentWinner( const CardList* played, Card** best );
	bool Beats( Card* card, Card* best );
	// Detect whether the trumph card (public info) is still held by an
	// adversary who hasn't played yet in the current trick and would
	// beat 'candidate' with it.
	bool TrumphThreatensCandidate( Card* candidate,
	                               const CardList& played );
	Card* LowestBeater( CardList& candidates, Card* best );
	Card* HighestValue( CardList& candidates );
	Card* LowestValue( CardList& candidates );
	Card* LowestNonTrumph();
};

#endif // _OBSERVINGBOT_HPP_
