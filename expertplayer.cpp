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

#include "expertplayer.hpp"

ExpertPlayer::ExpertPlayer( GamePos* gamepos ):
	MethodicPlayer( gamepos ) {}

// Simulated play policy, Smart-style.
//
// This is a full override of Methodic's simulator policy. The structure
// mirrors SmartPlayer::PlayFollowing: for a given hand, trick, trump and
// team membership, decide which card the simulated player plays. The
// decision uses only information the simulated player would plausibly
// have - no observational memory from the outer Expert is leaked into
// the sim (so simulated opponents don't gain unfair knowledge).
//
// Strategy summary:
//
//   Leading
//     Lead the ace if one is in the hand (guaranteed winner of its
//     suit); otherwise lead the lowest-value card. The 7 (bisca) is not
//     cashed here because the simulator has no ace-out history.
//
//   Following, can follow suit
//     - Partner winning: dump the highest-value card of the suit only
//       when partner's win is safe - we're the last to play, or
//       partner's card is an ace, a 7, or a trump (hard to beat).
//       Otherwise play the lowest-value card of the suit to avoid
//       gifting points if an opponent still beats partner.
//     - Opponent winning: play the cheapest card that beats the current
//       best (cheapest beater); if we can't beat it, play the lowest-
//       value card of the suit.
//
//   Following, can't follow suit
//     - Partner winning: don't trump - dump the highest-value non-trump
//       to add points to partner's trick. If we only have trumps left,
//       play the cheapest trump.
//     - Opponent winning: if the opponent hasn't trumped yet, play the
//       cheapest trump to win. If they already trumped, over-trump with
//       the cheapest trump that beats them. Otherwise discard the
//       lowest-value non-trump (or cheapest trump if only trumps
//       remain).
// Helper used by ExpertPlayer::SimPlayCard: has (type_id, suit_id)
// been played during the ongoing simulated round?
static bool SimOutHas( const CardList& sim_out, int type_id, int suit_id )
{
	CardList::Node* n = sim_out.GetFirst();
	while( n ) {
		Card* c = n->GetData();
		if( c->GetType().GetId() == type_id &&
		    c->GetSuit().GetId() == suit_id )
			return true;
		n = n->GetNext();
	}
	return false;
}

Card* ExpertPlayer::SimPlayCard( Card* hand[], int handsize,
                                 const CardList& trick,
                                 cardsuit_t trumphsuit, bool our_team,
                                 const CardList& sim_out, int lead_slot )
{
	(void)our_team;  // teams are derived from trick position below
	if( handsize == 0 )
		return NULL;

	// Trumph-threat context: identify where the trumph card sits in the
	// sim so we can avoid committing trumps the unplayed trumph owner
	// would capture. owner_slot is this sim world's player slot for the
	// trumph owner (3 = us).
	int current_slot = ( lead_slot + (int)trick.GetCount() ) % 4;
	int owner_slot;
	if( trumphowner == this )
		owner_slot = 3;
	else
		owner_slot = (int)PlayerIndex( trumphowner );
	bool trumph_alive = trumph && !sim_out.Find( trumph );
	int owner_trick_pos = ( owner_slot - lead_slot + 4 ) % 4;
	bool owner_unplayed_in_trick =
	  owner_trick_pos >= (int)trick.GetCount();
	bool trumph_threatens_us =
	  trumph_alive &&
	  owner_slot != current_slot &&
	  ( ( owner_slot % 2 ) != ( current_slot % 2 ) ) &&
	  owner_unplayed_in_trick;

	CardList::Node* first = trick.GetFirst();
	int pick_idx = -1;

	if( !first ) {
		// Leading - see strategy summary above. Cash an ace if we have
		// one; otherwise cash the 7 of any suit whose ace has already
		// come out in this simulated round (the 7 is then a guaranteed
		// winner); otherwise lead the lowest-value card.
		for( int i = 0; i < handsize; i++ ) {
			if( hand[i]->GetType().GetId() == ACE ) {
				pick_idx = i;
				break;
			}
		}
		if( pick_idx < 0 ) {
			for( int i = 0; i < handsize; i++ ) {
				if( hand[i]->GetType().GetId() == SEVEN &&
				    SimOutHas( sim_out, ACE, hand[i]->GetSuit().GetId() ) ) {
					pick_idx = i;
					break;
				}
			}
		}
		if( pick_idx < 0 ) {
			for( int i = 0; i < handsize; i++ ) {
				if( pick_idx < 0 ||
				    hand[i]->GetType().GetValue() <
				      hand[pick_idx]->GetType().GetValue() ||
				    ( hand[i]->GetType().GetValue() ==
				        hand[pick_idx]->GetType().GetValue() &&
				      hand[i]->GetType() < hand[pick_idx]->GetType() ) )
					pick_idx = i;
			}
		}
	}
	else {
		cardsuit_t leadsuit = first->GetData()->GetSuit().GetId();

		// Find the current winning card and its position in the trick.
		Card* best = first->GetData();
		int winpos = 0, pos = 0;
		CardList::Node* n = first;
		while( ( n = n->GetNext() ) != NULL ) {
			pos++;
			if( SimBeats( n->GetData(), best, trumphsuit ) ) {
				best = n->GetData();
				winpos = pos;
			}
		}
		// Positions 0,2 are one team; 1,3 the other. Our position in
		// the trick is trick.GetCount(), and partner shares parity.
		bool partner_winning =
		  ( ( winpos % 2 ) == ( (int)trick.GetCount() % 2 ) );
		bool we_are_last = ( trick.GetCount() == 3 );

		// Can we follow suit?
		bool has_suit = false;
		for( int i = 0; i < handsize; i++ )
			if( hand[i]->GetSuit().GetId() == leadsuit ) {
				has_suit = true;
				break;
			}

		if( has_suit ) {
			if( partner_winning ) {
				// Dump high only when partner's win is safe. Otherwise
				// stay cheap so that if an opponent still beats partner
				// we don't hand them extra points.
				bool dump_high = we_are_last ||
				  best->GetType().GetId() == ACE ||
				  best->GetType().GetId() == SEVEN ||
				  best->GetSuit().GetId() == trumphsuit;
				for( int i = 0; i < handsize; i++ ) {
					if( hand[i]->GetSuit().GetId() != leadsuit )
						continue;
					if( pick_idx < 0 ) {
						pick_idx = i;
						continue;
					}
					if( dump_high ) {
						if( hand[i]->GetType().GetValue() >
						    hand[pick_idx]->GetType().GetValue() )
							pick_idx = i;
					}
					else {
						if( hand[i]->GetType().GetValue() <
						      hand[pick_idx]->GetType().GetValue() ||
						    ( hand[i]->GetType().GetValue() ==
						        hand[pick_idx]->GetType().GetValue() &&
						      hand[i]->GetType() <
						        hand[pick_idx]->GetType() ) )
							pick_idx = i;
					}
				}
			}
			else {
				// Opponent winning - cheapest beater, or lowest-value if
				// we can't beat.
				int beater_idx = -1;
				for( int i = 0; i < handsize; i++ ) {
					if( hand[i]->GetSuit().GetId() != leadsuit )
						continue;
					if( SimBeats( hand[i], best, trumphsuit ) ) {
						if( beater_idx < 0 ||
						    hand[i]->GetType() <
						      hand[beater_idx]->GetType() )
							beater_idx = i;
					}
				}
				if( beater_idx >= 0 )
					pick_idx = beater_idx;
				else {
					for( int i = 0; i < handsize; i++ ) {
						if( hand[i]->GetSuit().GetId() != leadsuit )
							continue;
						if( pick_idx < 0 ||
						    hand[i]->GetType().GetValue() <
						      hand[pick_idx]->GetType().GetValue() ||
						    ( hand[i]->GetType().GetValue() ==
						        hand[pick_idx]->GetType().GetValue() &&
						      hand[i]->GetType() <
						        hand[pick_idx]->GetType() ) )
							pick_idx = i;
					}
				}
			}
		}
		else {
			// Can't follow suit.
			if( partner_winning ) {
				// Don't trump - dump highest-value non-trump to feed
				// partner's trick with points.
				for( int i = 0; i < handsize; i++ ) {
					if( hand[i]->GetSuit().GetId() == trumphsuit )
						continue;
					if( pick_idx < 0 ||
					    hand[i]->GetType().GetValue() >
					      hand[pick_idx]->GetType().GetValue() )
						pick_idx = i;
				}
				// Only trumps left - play cheapest trump.
				if( pick_idx < 0 ) {
					for( int i = 0; i < handsize; i++ ) {
						if( pick_idx < 0 ||
						    hand[i]->GetType() <
						      hand[pick_idx]->GetType() )
							pick_idx = i;
					}
				}
			}
			else {
				// Opponent winning - attempt to trump. If the unplayed
				// trumph holder is in the adversary team and their
				// trumph would beat the trump we'd commit, skip the
				// trump (a non-trump discard is preferable to feeding
				// them a trump).
				if( best->GetSuit().GetId() != trumphsuit ) {
					// They haven't trumped - cheapest trump wins.
					for( int i = 0; i < handsize; i++ ) {
						if( hand[i]->GetSuit().GetId() != trumphsuit )
							continue;
						if( pick_idx < 0 ||
						    hand[i]->GetType() <
						      hand[pick_idx]->GetType() )
							pick_idx = i;
					}
					if( pick_idx >= 0 && trumph_threatens_us &&
					    SimBeats( trumph, hand[pick_idx], trumphsuit ) )
						pick_idx = -1;
				}
				else {
					// They trumped - cheapest trump that over-trumps.
					for( int i = 0; i < handsize; i++ ) {
						if( hand[i]->GetSuit().GetId() != trumphsuit )
							continue;
						if( SimBeats( hand[i], best, trumphsuit ) ) {
							if( pick_idx < 0 ||
							    hand[i]->GetType() <
							      hand[pick_idx]->GetType() )
								pick_idx = i;
						}
					}
					if( pick_idx >= 0 && trumph_threatens_us &&
					    SimBeats( trumph, hand[pick_idx], trumphsuit ) )
						pick_idx = -1;
				}
				// Can't win - discard lowest-value non-trump.
				if( pick_idx < 0 ) {
					for( int i = 0; i < handsize; i++ ) {
						if( hand[i]->GetSuit().GetId() == trumphsuit )
							continue;
						if( pick_idx < 0 ||
						    hand[i]->GetType().GetValue() <
						      hand[pick_idx]->GetType().GetValue() ||
						    ( hand[i]->GetType().GetValue() ==
						        hand[pick_idx]->GetType().GetValue() &&
						      hand[i]->GetType() <
						        hand[pick_idx]->GetType() ) )
							pick_idx = i;
					}
				}
				// Only trumps remain - cheapest trump.
				if( pick_idx < 0 ) {
					for( int i = 0; i < handsize; i++ ) {
						if( pick_idx < 0 ||
						    hand[i]->GetType() <
						      hand[pick_idx]->GetType() )
							pick_idx = i;
					}
				}
			}
		}
	}

	if( pick_idx < 0 )
		pick_idx = 0;
	Card* result = hand[pick_idx];
	// Remove picked card from hand using the caller's swap-with-last
	// convention: caller decrements handsize after this call.
	hand[pick_idx] = hand[handsize - 1];
	return result;
}
