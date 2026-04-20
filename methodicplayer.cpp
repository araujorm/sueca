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

#include <cstdlib>
#include "methodicplayer.hpp"

MethodicPlayer::MethodicPlayer( GamePos* gamepos ):
	ObservingBot( gamepos ) {}

bool MethodicPlayer::SimBeats( Card* card, Card* best, cardsuit_t trumphsuit )
{
	if( card->GetSuit().GetId() == trumphsuit &&
	    best->GetSuit().GetId() != trumphsuit )
		return true;
	if( card->GetSuit() == best->GetSuit() &&
	    card->GetType() > best->GetType() )
		return true;
	return false;
}

int MethodicPlayer::SimTrickWinner( const CardList& trick, cardsuit_t trumphsuit )
{
	CardList::Node* node = trick.GetFirst();
	Card* best = node->GetData();
	int winpos = 0, pos = 0;
	while( ( node = node->GetNext() ) != NULL ) {
		pos++;
		if( SimBeats( node->GetData(), best, trumphsuit ) ) {
			best = node->GetData();
			winpos = pos;
		}
	}
	return winpos;
}

int MethodicPlayer::SimTrickPoints( const CardList& trick )
{
	int pts = 0;
	CardList::Node* node = trick.GetFirst();
	while( node ) {
		pts += node->GetData()->GetType().GetValue();
		node = node->GetNext();
	}
	return pts;
}

// Heuristic play for simulated players.
// our_team: true if this simulated player is on our team (us or partner).
// sim_out and lead_slot are available to more elaborate overrides; the
// Methodic policy is intentionally naive and ignores them.
Card* MethodicPlayer::SimPlayCard( Card* hand[], int handsize,
                                 const CardList& trick,
                                 cardsuit_t trumphsuit, bool our_team,
                                 const CardList& sim_out, int lead_slot )
{
	(void)sim_out;
	(void)lead_slot;
	if( handsize == 0 )
		return NULL;

	CardList::Node* first = trick.GetFirst();
	int pick_idx = -1;

	if( !first ) {
		// Leading in the simulation.
		//
		// A credible leading policy is essential: Monte Carlo averages
		// final team scores across many sampled worlds, and unrealistic
		// simulated leads skew the evaluation (simulated players dumping
		// aces and biscas as leads inflates our projected score and
		// hides the real cost of making a bad lead ourselves).
		//
		// The policy:
		//   - If the hand contains an ace, lead it. It's a guaranteed
		//     winner of its suit; the risk of being trumped by a void
		//     opponent is captured naturally by averaging across worlds.
		//   - Otherwise lead the lowest-value card (preferring value-0
		//     cards 2/3/4/5/6, tie-broken by card rank) so that losing
		//     the trick costs as little as possible.
		// We don't cash the 7 (bisca) here because the simulator doesn't
		// carry the global "is the ace out?" signal needed to do it
		// safely, and leading a bisca into an unprotected suit is a
		// cardinal sin.
		for( int i = 0; i < handsize; i++ ) {
			if( hand[i]->GetType().GetId() == ACE ) {
				pick_idx = i;
				break;
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

		// Find current best card in trick
		Card* best = first->GetData();
		CardList::Node* n = first;
		while( ( n = n->GetNext() ) != NULL )
			if( SimBeats( n->GetData(), best, trumphsuit ) )
				best = n->GetData();

		// Is current winner on the same team as this player?
		// Trick positions: 0=leader. Winner position mod 2 tells team.
		// Leader's team is 'our_team' only if we're the leader, but we
		// don't know who the leader is here. Use a simpler heuristic:
		// just determine if the winning card was played at an even or odd
		// offset from the leader. Even offsets = same team as leader,
		// odd = opposite team. We know if the current player is on our_team.
		int winpos = 0, pos = 0;
		n = first;
		Card* wcard = first->GetData();
		while( ( n = n->GetNext() ) != NULL ) {
			pos++;
			if( SimBeats( n->GetData(), wcard, trumphsuit ) ) {
				wcard = n->GetData();
				winpos = pos;
			}
		}
		// In a 4-player trick, positions 0,2 are one team and 1,3 are the other.
		// The current player is at position trick.GetCount().
		// If (winpos % 2) == (trick.GetCount() % 2), winner is on same team.
		bool partner_winning = ( ( winpos % 2 ) == ( (int)trick.GetCount() % 2 ) );

		// Check if we have the leading suit
		bool has_suit = false;
		for( int i = 0; i < handsize; i++ )
			if( hand[i]->GetSuit().GetId() == leadsuit )
				has_suit = true;

		if( has_suit ) {
			if( partner_winning ) {
				// Dump highest value card of this suit
				for( int i = 0; i < handsize; i++ ) {
					if( hand[i]->GetSuit().GetId() != leadsuit )
						continue;
					if( pick_idx < 0 ||
					    hand[i]->GetType().GetValue() > hand[pick_idx]->GetType().GetValue() )
						pick_idx = i;
				}
			}
			else {
				// Try to beat with cheapest winner
				int beater_idx = -1;
				for( int i = 0; i < handsize; i++ ) {
					if( hand[i]->GetSuit().GetId() != leadsuit )
						continue;
					if( SimBeats( hand[i], best, trumphsuit ) ) {
						if( beater_idx < 0 ||
						    hand[i]->GetType() < hand[beater_idx]->GetType() )
							beater_idx = i;
					}
				}
				if( beater_idx >= 0 ) {
					pick_idx = beater_idx;
				}
				else {
					// Can't beat - play lowest value
					for( int i = 0; i < handsize; i++ ) {
						if( hand[i]->GetSuit().GetId() != leadsuit )
							continue;
						if( pick_idx < 0 ||
						    hand[i]->GetType().GetValue() < hand[pick_idx]->GetType().GetValue() )
							pick_idx = i;
					}
				}
			}
		}
		else {
			// Can't follow suit
			if( partner_winning ) {
				// Don't trump - dump highest value non-trump
				for( int i = 0; i < handsize; i++ ) {
					if( hand[i]->GetSuit().GetId() == trumphsuit )
						continue;
					if( pick_idx < 0 ||
					    hand[i]->GetType().GetValue() > hand[pick_idx]->GetType().GetValue() )
						pick_idx = i;
				}
				// Only trumps left - play lowest
				if( pick_idx < 0 ) {
					for( int i = 0; i < handsize; i++ ) {
						if( pick_idx < 0 ||
						    hand[i]->GetType() < hand[pick_idx]->GetType() )
							pick_idx = i;
					}
				}
			}
			else {
				// Opponent winning - try to trump
				if( best->GetSuit().GetId() != trumphsuit ) {
					// Play lowest trump
					for( int i = 0; i < handsize; i++ ) {
						if( hand[i]->GetSuit().GetId() != trumphsuit )
							continue;
						if( pick_idx < 0 ||
						    hand[i]->GetType() < hand[pick_idx]->GetType() )
							pick_idx = i;
					}
				}
				else {
					// They trumped - try to over-trump
					for( int i = 0; i < handsize; i++ ) {
						if( hand[i]->GetSuit().GetId() != trumphsuit )
							continue;
						if( SimBeats( hand[i], best, trumphsuit ) ) {
							if( pick_idx < 0 ||
							    hand[i]->GetType() < hand[pick_idx]->GetType() )
								pick_idx = i;
						}
					}
				}
				// Can't trump - discard lowest value
				if( pick_idx < 0 ) {
					for( int i = 0; i < handsize; i++ ) {
						if( pick_idx < 0 ||
						    hand[i]->GetType().GetValue() < hand[pick_idx]->GetType().GetValue() )
							pick_idx = i;
					}
				}
			}
		}
	}

	if( pick_idx < 0 )
		pick_idx = 0;
	Card* result = hand[pick_idx];
	hand[pick_idx] = hand[handsize - 1];
	return result;
}

bool MethodicPlayer::SampleWorld( Card** simhands, int* simsizes )
{
	// Collect unknown cards: not in our hand and not played
	Card* unknown[40];
	int n_unknown = 0;
	Deck& deck = thegame->GetDeck();
	for( int i = 0; i < 40; i++ ) {
		Card* c = deck.cards[i];
		if( GetHand().Find( c ) || out.Find( c ) )
			continue;
		unknown[n_unknown++] = c;
	}

	int each = GetHand().GetCount();
	simsizes[0] = simsizes[1] = simsizes[2] = each;

	// Pre-place the trumph card in its known owner's hand. The trumph
	// is revealed to everyone at deal time, so every world should have
	// it there until it's actually played. Without this the sampler
	// would distribute the trumph at random, skewing the simulation.
	int pre_counts[3] = { 0, 0, 0 };
	if( trumph && trumphowner != this && ! out.Find( trumph ) ) {
		plindex_t pli = PlayerIndex( trumphowner );
		if( pli != SBOT_THIS ) {
			simhands[pli * 10 + 0] = trumph;
			pre_counts[pli] = 1;
			for( int i = 0; i < n_unknown; i++ )
				if( unknown[i] == trumph ) {
					unknown[i] = unknown[--n_unknown];
					break;
				}
		}
	}

	// Shuffle unknown cards
	for( int i = n_unknown - 1; i > 0; i-- ) {
		int j = (int)( (double)( i + 1 ) * rand() / ( RAND_MAX + 1.0 ) );
		Card* t = unknown[i];
		unknown[i] = unknown[j];
		unknown[j] = t;
	}

	// Distribute respecting plhasnot constraints
	for( int attempt = 0; attempt < 10; attempt++ ) {
		if( attempt > 0 ) {
			for( int i = n_unknown - 1; i > 0; i-- ) {
				int j = (int)( (double)( i + 1 ) * rand() / ( RAND_MAX + 1.0 ) );
				Card* t = unknown[i];
				unknown[i] = unknown[j];
				unknown[j] = t;
			}
		}

		int counts[3] = { pre_counts[0], pre_counts[1], pre_counts[2] };
		bool ok = true;

		for( int i = 0; i < n_unknown && ok; i++ ) {
			Card* c = unknown[i];
			cardsuit_t suit = c->GetSuit().GetId();
			bool placed = false;
			int start = (int)( 3.0 * rand() / ( RAND_MAX + 1.0 ) );
			for( int tries = 0; tries < 3 && !placed; tries++ ) {
				int p = ( start + tries ) % 3;
				if( counts[p] < each && !plhasnot[p][suit] ) {
					simhands[p * 10 + counts[p]] = c;
					counts[p]++;
					placed = true;
				}
			}
			if( !placed ) {
				ok = false;
				break;
			}
		}
		if( ok && counts[0] == each && counts[1] == each && counts[2] == each )
			return true;
	}

	// Fallback: ignore plhasnot but keep the trumph placement
	int idx = 0;
	for( int p = 0; p < 3; p++ )
		for( int i = pre_counts[p]; i < each && idx < n_unknown; i++ )
			simhands[p * 10 + i] = unknown[idx++];
	return true;
}

// Simulate the rest of the game.
// Players: 0=right(opp), 1=partner, 2=left(opp), 3=us
// Turn order: 3 -> 0 -> 1 -> 2 -> 3 ...
// Our team: 1 and 3.
int MethodicPlayer::SimulateGame( Card* mycard, const CardList* played,
                                Card** simhands, int* simsizes )
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();
	int team_points = 0;

	// 4 hands: 0=right, 1=partner, 2=left, 3=us
	Card* hands[4][10];
	int hsizes[4];
	for( int p = 0; p < 3; p++ ) {
		hsizes[p] = simsizes[p];
		for( int i = 0; i < hsizes[p]; i++ )
			hands[p][i] = simhands[p * 10 + i];
	}
	// Our hand (excluding the card we play)
	hsizes[3] = 0;
	CardList::Node* hnode = GetHand().GetFirst();
	while( hnode ) {
		if( hnode->GetData() != mycard )
			hands[3][hsizes[3]++] = hnode->GetData();
		hnode = hnode->GetNext();
	}

	// Build the play order for the current turn.
	// played->GetCount() cards were played before us.
	// Turn order from us: us(3), right(0), partner(1), left(2)
	// The turn starter is (4 - played->GetCount()) positions before us.
	// Full turn order: starter, ..., us, ..., ender
	// Map: who plays at each position in the current turn
	int cur_order[4];
	// Positions before us were played by: left, partner, right (backwards)
	// played=0: order is 3,0,1,2 (we lead)
	// played=1: order is 2,3,0,1 (left led)
	// played=2: order is 1,2,3,0 (partner led)
	// played=3: order is 0,1,2,3 (right led)
	int n_played = played->GetCount();
	int lead_player;
	switch( n_played ) {
	case 0: lead_player = 3; break;
	case 1: lead_player = 2; break;
	case 2: lead_player = 1; break;
	default: lead_player = 0; break;
	}
	for( int i = 0; i < 4; i++ )
		cur_order[i] = ( lead_player + i ) % 4;

	// sim_out starts with every card we've actually seen played in the
	// real game and grows with every card we play inside the simulation.
	// Together with lead_slot it lets the per-player policy see sim-
	// specific state (e.g. "has the ace of this suit come out yet in
	// this simulated future?") without exposing other players' hands.
	CardList sim_out;
	CardList::Node* onode = out.GetFirst();
	while( onode ) {
		sim_out.Append( onode->GetData() );
		onode = onode->GetNext();
	}

	// Finish current turn: we already decided mycard, simulate the rest
	CardList curtrick;
	CardList::Node* pnode = played->GetFirst();
	while( pnode ) {
		curtrick.Append( pnode->GetData() );
		pnode = pnode->GetNext();
	}
	curtrick.Append( mycard );

	int lead_slot = cur_order[0];

	// Remaining players in this turn play after us
	for( int i = n_played + 1; i < 4; i++ ) {
		int p = cur_order[i];
		bool our_team = ( p == 1 || p == 3 );
		Card* c = SimPlayCard( hands[p], hsizes[p], curtrick, trumphsuit,
		                       our_team, sim_out, lead_slot );
		if( c ) {
			hsizes[p]--;
			curtrick.Append( c );
		}
	}

	// Score first trick
	int winpos = SimTrickWinner( curtrick, trumphsuit );
	int winner = cur_order[winpos];
	if( winner == 1 || winner == 3 )
		team_points += SimTrickPoints( curtrick );
	// Move the whole trick into sim_out - these cards are no longer in
	// anyone's sim hand and the policy should know that for the rest of
	// the simulated round.
	CardList::Node* tnode = curtrick.GetFirst();
	while( tnode ) {
		sim_out.Append( tnode->GetData() );
		tnode = tnode->GetNext();
	}

	// Remaining tricks: winner leads
	int tricks_left = hsizes[3];
	for( int t = 0; t < tricks_left; t++ ) {
		// Build turn order starting from winner
		int order[4];
		for( int i = 0; i < 4; i++ )
			order[i] = ( winner + i ) % 4;
		lead_slot = order[0];

		CardList simtrick;
		for( int i = 0; i < 4; i++ ) {
			int p = order[i];
			bool our_team = ( p == 1 || p == 3 );
			Card* c = SimPlayCard( hands[p], hsizes[p], simtrick, trumphsuit,
			                       our_team, sim_out, lead_slot );
			if( c ) {
				hsizes[p]--;
				simtrick.Append( c );
			}
		}

		winpos = SimTrickWinner( simtrick, trumphsuit );
		winner = order[winpos];
		if( winner == 1 || winner == 3 )
			team_points += SimTrickPoints( simtrick );
		CardList::Node* st = simtrick.GetFirst();
		while( st ) {
			sim_out.Append( st->GetData() );
			st = st->GetNext();
		}
	}

	return team_points;
}

Card* MethodicPlayer::PlayCard( const CardList* played )
{
	// If only one valid card, play it immediately
	CardList valid;
	CardList::Node* node = GetHand().GetFirst();
	while( node ) {
		Card* c = node->GetData();
		if( IsValidMove( c, *played ) )
			valid.Append( c );
		node = node->GetNext();
	}
	if( valid.GetCount() <= 1 ) {
		played_card = valid.GetFirst()->GetData();
		return played_card;
	}

	// Prune strictly-dominated candidates before running PIMC so the
	// simulation budget is spent differentiating real alternatives.
	//
	// Over-trumping: when a non-trump lead has already been trumped by
	// another player and we can over-trump with several of our trumps,
	// all those cards win exactly the same trick against the same
	// opponents' cards. The cheapest of them is strictly better - we
	// win this trick and keep the higher trumps for later - so drop
	// the other over-trumpers from the PIMC candidate set.
	if( played->GetCount() > 0 ) {
		cardsuit_t leadsuit =
		  played->GetFirst()->GetData()->GetSuit().GetId();
		cardsuit_t trumphsuit = trumph->GetSuit().GetId();
		Card* best;
		CurrentWinner( played, &best );
		if( leadsuit != trumphsuit &&
		    best->GetSuit().GetId() == trumphsuit ) {
			Card* cheapest = NULL;
			for( CardList::Node* m = valid.GetFirst(); m; m = m->GetNext() ) {
				Card* c = m->GetData();
				if( c->GetSuit().GetId() == trumphsuit &&
				    Beats( c, best ) &&
				    ( !cheapest || c->GetType() < cheapest->GetType() ) )
					cheapest = c;
			}
			if( cheapest ) {
				CardList::Node* m = valid.GetFirst();
				while( m ) {
					Card* c = m->GetData();
					CardList::Node* next = m->GetNext();
					if( c->GetSuit().GetId() == trumphsuit &&
					    Beats( c, best ) && c != cheapest )
						valid.DeleteNode( m );
					m = next;
				}
			}
		}

		// Trumph threat prune: any trump candidate that the still-
		// unplayed trumph-owning adversary would capture with their
		// trumph is strictly worse than discarding a non-trump - we'd
		// lose the trick either way, but giving up a trump makes it
		// worse. Drop such trumps from the PIMC pool as long as there
		// is a non-trump alternative left to play. (If our only legal
		// plays are threatened trumps - e.g. we must follow a trump
		// lead - we don't prune, because we have no alternative.)
		bool has_non_trump_alt = false;
		for( CardList::Node* m = valid.GetFirst(); m; m = m->GetNext() )
			if( m->GetData()->GetSuit().GetId() != trumphsuit ) {
				has_non_trump_alt = true;
				break;
			}
		if( has_non_trump_alt ) {
			CardList::Node* m = valid.GetFirst();
			while( m ) {
				Card* c = m->GetData();
				CardList::Node* next = m->GetNext();
				if( c->GetSuit().GetId() == trumphsuit &&
				    TrumphThreatensCandidate( c, *played ) )
					valid.DeleteNode( m );
				m = next;
			}
		}
	}
	// Save-the-ace-of-trumps lead filter: when we're the leader and an
	// adversary still holds a point-bearing trumph card, leading the ace
	// of trumps transfers trump control to them (they follow with a low
	// trump and their trumph becomes the top remaining trump). The ace
	// is worth far more as an over-trump that captures that trumph later,
	// so drop it from the PIMC lead candidates as long as we have another
	// card to lead. Endgame plays cash the ace unconditionally - then
	// there's no "later" to save it for.
	bool endgame = GetHand().GetCount() <= 3;
	if( played->GetCount() == 0 && !endgame &&
	    AdversaryHoldsValuableTrumph() ) {
		cardsuit_t ts = trumph->GetSuit().GetId();
		Card* ace_trump = NULL;
		bool has_other = false;
		for( CardList::Node* m = valid.GetFirst(); m; m = m->GetNext() ) {
			Card* c = m->GetData();
			if( c->GetSuit().GetId() == ts &&
			    c->GetType().GetId() == ACE )
				ace_trump = c;
			else
				has_other = true;
		}
		if( ace_trump && has_other )
			valid.DeleteObject( ace_trump );
	}
	if( valid.GetCount() == 1 ) {
		played_card = valid.GetFirst()->GetData();
		return played_card;
	}

	// PIMC: for each valid card, simulate many random worlds
	Card* best_card = NULL;
	int best_score = -1;

	node = valid.GetFirst();
	while( node ) {
		Card* candidate = node->GetData();
		int total_score = 0;

		for( int w = 0; w < PIMC_WORLDS; w++ ) {
			Card* simhands[30];
			int simsizes[3];
			if( SampleWorld( simhands, simsizes ) )
				total_score += SimulateGame( candidate, played, simhands, simsizes );
		}

		if( total_score > best_score ) {
			best_score = total_score;
			best_card = candidate;
		}
		node = node->GetNext();
	}

	played_card = best_card ? best_card : valid.GetFirst()->GetData();
	return played_card;
}
