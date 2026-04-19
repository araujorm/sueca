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

#include "methodicplayer.hpp"

// Expert player: blends the Methodic PIMC machinery (inherited) with a
// Smart-style simulator. The Monte Carlo sampling and world distribution
// are reused as-is; only the per-decision simulator policy is replaced.
//
// The rationale: Methodic's evaluation is only as good as the players it
// simulates, and a simple "grab highest / dump highest" policy misleads
// the Monte Carlo into unrealistic outcomes. Expert swaps in a policy
// that mirrors the Smart player's follow-suit rulebook (cheapest beater,
// conditional dump when partner wins, cheapest trump, etc.) so that
// sampled worlds reflect plausible play.
//
// Importantly, the simulator still only uses information available to a
// simulated player (their hand, the current trick, the trump suit, which
// team they're on). It does not leak the Expert's observational memory
// (plhasnot, n_out, out) to simulated opponents - doing so would be
// cheating and would bias the evaluation.
class ExpertPlayer: public MethodicPlayer
{
public:
	ExpertPlayer( GamePos* gamepos );
protected:
	Card* SimPlayCard( Card* hand[], int handsize,
	                   const CardList& trick,
	                   cardsuit_t trumphsuit, bool our_team,
	                   const CardList& sim_out, int lead_slot );
};

#endif // _EXPERTPLAYER_HPP_
