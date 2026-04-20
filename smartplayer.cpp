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

#include "smartplayer.hpp"
#include "smarttactics.hpp"

SmartPlayer::SmartPlayer( GamePos* gamepos ):
	ObservingBot( gamepos ) {}

// Strategy when we are the first to play (leading the trick).
//
// Leading is the most strategic moment in sueca: we choose which suit to
// engage and a bad choice throws point cards away. Scoring recap: ace 11,
// 7 (bisca) 10, K 4, J 3, Q 2, others 0. The sueca ranking from highest
// to lowest is: A > 7 > K > J > Q > 6 > 5 > 4 > 3 > 2.
//
// Two rules of thumb drive this heuristic:
//
//   1. Cash winners: if we have a card guaranteed to win this trick
//      (ace, or the 7 when the ace of that suit is already out), lead it
//      and bank the points for our team.
//
//   2. Otherwise lead cheap: if we don't have a sure winner, lead a
//      worthless card (value 0, typically 2/3/4/5/6) so that even if we
//      lose the trick the opponents gain at most a couple of points. A
//      cardinal sin of sueca is leading a point card into an unprotected
//      suit - it will be captured.
//
// Cashing is always safe in the trump suit (a trump can't be trumped),
// but for non-trump suits we guard against having our ace/bisca chopped
// off by an opponent who is already void of that suit:
//   - plhasnot[opp][suit]: we observed the opponent discarding on this
//     suit before, so they can trump - a hard rule;
//   - n_out[suit] > 7: more than 7 of the 10 cards of the suit are out,
//     so someone being void is likely - a soft probability heuristic.
// The soft heuristic is dropped once we enter the endgame (3 or fewer
// cards in hand): an uncashed ace that stays in hand will likely be
// forced out on a trick we didn't pick, capturing no points - it's
// better to attempt the cash even under slightly higher void risk than
// to hold the ace until it becomes useless. The observed-void filter
// still applies, because a known trump threat doesn't go away just
// because time is running short.
// Non-trump cashing is normally tried before trump cashing so we keep
// our trump winners for moments where they are most valuable (e.g.
// trumping an opponent's ace later in the game). That ordering is
// inverted in one specific case - see the "destrunfe" block below.
Card* SmartPlayer::PlayFirst()
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();
	bool endgame = GetHand().GetCount() <= 3;

	// Count cashable side winners (aces, or 7s with the ace already out)
	// in non-trump suits, applying the same void-risk filters the cash
	// loop below will use. Two or more such winners trigger the
	// "destrunfe" (pulling trumps) strategy.
	int side_winners = 0;
	for( int s = SUITMIN; s <= SUITMAX; s++ ) {
		if( s == trumphsuit || bysuit[s].GetCount() == 0 )
			continue;
		if( plhasnot[SBOT_LEFT][s] || plhasnot[SBOT_RIGHT][s] )
			continue;
		if( !endgame && n_out[s] > 7 )
			continue;
		Card* highest = bysuit[s].GetLast()->GetData();
		if( highest->GetType().GetId() == ACE )
			side_winners++;
		else if( highest->GetType().GetId() == SEVEN && IsOut( ACE, s ) )
			side_winners++;
	}

	// "Destrunfe" (pulling trumps): when two or more side winners are
	// waiting to be cashed and the round still has enough tricks for the
	// effect to pay off, try to clear opponents' trumps first so those
	// side winners become safer on the next rounds.
	//   - If we hold a trump winner (ace, or 7 with the trump ace out),
	//     cash it: scores immediately and pulls one trump from every
	//     other player.
	//   - Otherwise, if we're loaded with trumps (4+), lead our cheapest
	//     trump. We'll likely lose this trick but opponents have to burn
	//     a trump each, and our side winners stop being trumpable soon
	//     after.
	// The endgame guard keeps the strategy out of the last three tricks,
	// where there isn't enough runway for the effect to pay back.
	if( !endgame && side_winners >= 2 ) {
		int our_trumps = bysuit[trumphsuit].GetCount();
		if( our_trumps > 0 ) {
			Card* highest = bysuit[trumphsuit].GetLast()->GetData();
			if( highest->GetType().GetId() == ACE )
				return highest;
			if( highest->GetType().GetId() == SEVEN &&
			    IsOut( ACE, trumphsuit ) )
				return highest;
			if( our_trumps >= 4 )
				return LowestValue( bysuit[trumphsuit] );
		}
	}

	// 1. Non-trump suits: cash the ace, or the 7 when the ace is out.
	// Apply void-risk filters to avoid being trumped.
	for( int s = SUITMIN; s <= SUITMAX; s++ ) {
		if( s == trumphsuit || bysuit[s].GetCount() == 0 )
			continue;
		if( plhasnot[SBOT_LEFT][s] || plhasnot[SBOT_RIGHT][s] )
			continue;
		if( !endgame && n_out[s] > 7 )
			continue;
		Card* highest = bysuit[s].GetLast()->GetData();
		if( highest->GetType().GetId() == ACE )
			return highest;
		if( highest->GetType().GetId() == SEVEN && IsOut( ACE, s ) )
			return highest;
	}

	// 2. Trump suit: same cash idea without void-risk filters. A trump
	// lead can't be chopped - anyone holding trump must follow suit, and
	// nothing else beats trump.
	//
	// One caveat: if an adversary still holds the public trumph card and
	// it's worth points (the 7, K, J, or Q of trumps), our ace of trumps
	// is worth far more as an over-trump than as a lead. Cashing it now
	// lets the adversary save the point-bearing trumph (they just play
	// a lower trump to follow our ace) and become the top remaining
	// trump. Save the ace in that case, except in endgame where we
	// can't afford to hold on to it.
	if( bysuit[trumphsuit].GetCount() > 0 ) {
		Card* highest = bysuit[trumphsuit].GetLast()->GetData();
		bool save_trump_ace = !endgame && AdversaryHoldsValuableTrumph();
		if( highest->GetType().GetId() == ACE && !save_trump_ace )
			return highest;
		if( highest->GetType().GetId() == SEVEN &&
		    IsOut( ACE, trumphsuit ) )
			return highest;
	}

	// 3. No guaranteed winner - lead the cheapest card. LowestNonTrumph
	// picks the lowest-value card across all non-trump suits (preferring
	// value-0 cards: 2/3/4/5/6). Only if our hand is all trumps do we
	// lead the cheapest trump.
	Card* lowest = LowestNonTrumph();
	if( lowest )
		return lowest;
	return LowestValue( bysuit[trumphsuit] );
}

// Strategy when we are not the first to play
Card* SmartPlayer::PlayFollowing( const CardList* played )
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();
	cardsuit_t firstsuit = played->GetFirst()->GetData()->GetSuit().GetId();

	Card* best;
	Player* winner = CurrentWinner( played, &best );
	bool partner_winning = ( winner == partner );
	bool we_are_last = ( played->GetCount() == 3 );

	// Case: we can follow suit
	if( bysuit[firstsuit].GetCount() > 0 ) {
		if( partner_winning ) {
			// Partner is winning - dump points if the win looks safe,
			// otherwise play a cheap card in case an opponent still to
			// play can capture the trick.
			return smarttactics::ShouldDumpHigh(
			         best, we_are_last, trumphsuit ) ?
			       HighestValue( bysuit[firstsuit] ) :
			       LowestValue( bysuit[firstsuit] );
		}
		else {
			// Adversary is winning - try to beat them
			Card* beater = LowestBeater( bysuit[firstsuit], best );
			if( beater )
	return beater;
			// Can't beat - play lowest value card
			return LowestValue( bysuit[firstsuit] );
		}
	}

	// Case: can't follow suit, partner is winning.
	// Mirror the "can follow suit" logic above: only dump a high-value
	// card when partner's win is reasonably safe. Otherwise an opponent
	// still to play could capture the trick and we don't want to hand
	// them extra points, so play the cheapest non-trump instead.
	if( partner_winning ) {
		Card* dump = smarttactics::ShouldDumpHigh(
		               best, we_are_last, trumphsuit ) ?
		  HighestNonTrumph() : LowestNonTrumph();
		if( dump )
			return dump;
		// Only trumps left - play lowest trump
		return LowestValue( bysuit[trumphsuit] );
	}

	// Adversary is winning and we can't follow suit - consider trumping.
	// In both "they haven't trumped" and "over-trump" paths, skip the
	// trump play if an unplayed adversary holds the trumph card and it
	// would capture what we'd play - better to discard than feed them
	// a trump.
	if( bysuit[trumphsuit].GetCount() > 0 ) {
		if( best->GetSuit().GetId() != trumphsuit ) {
			// They haven't trumped - play lowest trump to win
			Card* low = bysuit[trumphsuit].GetFirst()->GetData();
			if( !TrumphThreatensCandidate( low, *played ) )
				return low;
		}
		else {
			// They already trumped - try to over-trump
			Card* over = LowestBeater( bysuit[trumphsuit], best );
			if( over && !TrumphThreatensCandidate( over, *played ) )
				return over;
		}
	}

	// Can't win - discard lowest value card
	Card* discard = LowestNonTrumph();
	if( discard )
		return discard;
	return LowestValue( bysuit[trumphsuit] );
}

Card* SmartPlayer::PlayCard( const CardList* played )
{
	if( played->GetCount() == 0 )
		played_card = PlayFirst();
	else
		played_card = PlayFollowing( played );
	return played_card;
}
