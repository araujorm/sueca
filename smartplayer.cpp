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

#include <cstring>  // For memset()
#include "smartplayer.hpp"
#include "main.hpp"  // For wxGetApp()

// Smart player implementation
SmartPlayer::SmartPlayer( GamePos* gamepos ):
	BotPlayer( gamepos ), thegame( NULL ), players( NULL ) {}

SmartPlayer::~SmartPlayer()
{
	if( players )
		delete players;
}

void SmartPlayer::NewGame( Game* game )
{
	thegame = game;
	if( players )
		delete players;
	// Meet the other players
	players = game->GetPlayers();
	players->SetCurrent( this );
	right = players->GetNext();
	partner = players->GetNext();
	left = players->GetNext();
}

void SmartPlayer::NewRound( Card* newtrumph, Player* newowner )
{
	trumph = newtrumph;
	trumphowner = newowner;
	for( int i = 0; i < 3; i++ )
		for( int j = 0; j < 4; j++ ) {
			plhasnot[i][j] = false;
			released[i][j] = 0;
		}
	out.Clear();
	for( int i = SUITMIN; i <= SUITMAX; i++ )
		n_out[i] = 0;
	CardList::Node* node = GetHand().GetFirst();
	// Group hand by suit, in ascending order
	while( node ) {
		Card* card = node->GetData();
		CardList& suitlst = bysuit[card->GetSuit().GetId()];
		CardList::Node* suitnode = suitlst.GetFirst();
		while( suitnode && card->GetType() > suitnode->GetData()->GetType() )
			suitnode = suitnode->GetNext();
		if( suitnode )
			suitlst.Insert( suitnode, card );
		else
			suitlst.Append( card );
		node = node->GetNext();
	}
}

void SmartPlayer::NewTurn( Player* starter )
{
	turnstarter = starter;
}

void SmartPlayer::TurnEnd( const Player* winner, const CardList& played )
{
	// Remove the card we played from our suit list
	bysuit[played_card->GetSuit().GetId()].DeleteObject( played_card );
	// Memorize cards that have been played
	players->SetCurrent( turnstarter );
	CardList::Node* node = played.GetFirst();
	cardsuit_t firstsuit = node->GetData()->GetSuit().GetId();
	while( node ) {
		Card* card = node->GetData();
		node = node->GetNext();
		out.Append( card );
		n_out[card->GetSuit().GetId()]++;
		plindex_t pli;
		if( ( pli = PlayerIndex( players->GetCurrent() ) ) != SBOT_THIS ) {
			released[pli][card->GetSuit().GetId()]++;
			// If a player didn't follow suit, they don't have the leading suit
			if( card->GetSuit().GetId() != firstsuit )
				plhasnot[pli][firstsuit] = true;
		}
		players->GetNext();
	}
}

// Determine current trick winner and the winning card
Player* SmartPlayer::CurrentWinner( const CardList* played, Card** best )
{
	CardList::Node* node = played->GetFirst();
	if( !node )
		return NULL;
	*best = node->GetData();
	PlayerIterator* pit = thegame->GetPlayers();
	pit->SetCurrent( turnstarter );
	Player* winner = turnstarter;
	while( ( node = node->GetNext() ) != NULL ) {
		Card* card = node->GetData();
		Player* player = pit->GetNext();
		if( Beats( card, *best ) ) {
			*best = card;
			winner = player;
		}
	}
	delete pit;
	return winner;
}

// Does 'card' beat 'best' considering trumps?
bool SmartPlayer::Beats( Card* card, Card* best )
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();
	// Trump beats non-trump
	if( card->GetSuit().GetId() == trumphsuit &&
	    best->GetSuit().GetId() != trumphsuit )
		return true;
	// Higher trump beats lower trump, or higher card of same suit
	if( card->GetSuit() == best->GetSuit() &&
	    card->GetType() > best->GetType() )
		return true;
	return false;
}

// Find the lowest card in 'candidates' that beats 'best'
Card* SmartPlayer::LowestBeater( CardList& candidates, Card* best )
{
	Card* result = NULL;
	CardList::Node* node = candidates.GetFirst();
	while( node ) {
		Card* card = node->GetData();
		if( Beats( card, best ) ) {
			if( !result || card->GetType() < result->GetType() )
	result = card;
		}
		node = node->GetNext();
	}
	return result;
}

// Find the highest value card in a list (prefer 7=10, A=11, K=4, etc.)
Card* SmartPlayer::HighestValue( CardList& candidates )
{
	Card* result = NULL;
	CardList::Node* node = candidates.GetFirst();
	while( node ) {
		Card* card = node->GetData();
		if( !result || card->GetType().GetValue() > result->GetType().GetValue() ||
		    ( card->GetType().GetValue() == result->GetType().GetValue() &&
		      card->GetType() > result->GetType() ) )
			result = card;
		node = node->GetNext();
	}
	return result;
}

// Find the lowest value card in a list
Card* SmartPlayer::LowestValue( CardList& candidates )
{
	Card* result = NULL;
	CardList::Node* node = candidates.GetFirst();
	while( node ) {
		Card* card = node->GetData();
		if( !result || card->GetType().GetValue() < result->GetType().GetValue() ||
		    ( card->GetType().GetValue() == result->GetType().GetValue() &&
		      card->GetType() < result->GetType() ) )
			result = card;
		node = node->GetNext();
				}
	return result;
}

// Find the lowest value non-trump card in hand
Card* SmartPlayer::LowestNonTrumph()
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();
	Card* result = NULL;
	for( int s = SUITMIN; s <= SUITMAX; s++ ) {
		if( s == trumphsuit )
			continue;
		Card* low = LowestValue( bysuit[s] );
		if( low && ( !result ||
		             low->GetType().GetValue() < result->GetType().GetValue() ||
		             ( low->GetType().GetValue() == result->GetType().GetValue() &&
		               low->GetType() < result->GetType() ) ) )
			result = low;
	}
	return result;
}

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
// Non-trump cashing is tried before trump cashing so we keep our trump
// winners for moments where they are most valuable (e.g. trumping an
// opponent's ace later in the game).
Card* SmartPlayer::PlayFirst()
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();
	bool endgame = GetHand().GetCount() <= 3;

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
	if( bysuit[trumphsuit].GetCount() > 0 ) {
		Card* highest = bysuit[trumphsuit].GetLast()->GetData();
		if( highest->GetType().GetId() == ACE )
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
			// Partner is winning - dump points to help the team
			if( we_are_last ) {
	// We're last, safe to dump highest value
	return HighestValue( bysuit[firstsuit] );
			}
			// Not last - one adversary still plays, be a bit careful
			// If partner's card is very strong (ace/7 of trumps, or ace of suit),
			// dump points; otherwise play conservatively
			if( best->GetType().GetId() >= SEVEN ||
			    best->GetSuit().GetId() == trumphsuit )
	return HighestValue( bysuit[firstsuit] );
			else
	return LowestValue( bysuit[firstsuit] );
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
	// card when partner's win is reasonably safe - we're the last to
	// play, or partner's card is an ace, a 7, or a trump (hard for a
	// remaining opponent to beat). Otherwise an opponent still to play
	// could capture the trick and we don't want to hand them extra
	// points, so play the cheapest non-trump instead.
	if( partner_winning ) {
		bool dump_high = we_are_last ||
		                 best->GetType().GetId() == ACE ||
		                 best->GetType().GetId() == SEVEN ||
		                 best->GetSuit().GetId() == trumphsuit;
		if( dump_high ) {
			Card* dump = NULL;
			for( int s = SUITMIN; s <= SUITMAX; s++ ) {
				if( s == trumphsuit || bysuit[s].GetCount() == 0 )
	continue;
				Card* high = HighestValue( bysuit[s] );
				if( high && ( !dump ||
				              high->GetType().GetValue() >
				                dump->GetType().GetValue() ) )
	dump = high;
			}
			if( dump )
				return dump;
		}
		else {
			Card* low = LowestNonTrumph();
			if( low )
				return low;
		}
		// Only trumps left - play lowest trump
		return LowestValue( bysuit[trumphsuit] );
	}

	// Adversary is winning and we can't follow suit - consider trumping
	if( bysuit[trumphsuit].GetCount() > 0 ) {
		if( best->GetSuit().GetId() != trumphsuit ) {
			// They haven't trumped - play lowest trump to win
			return bysuit[trumphsuit].GetFirst()->GetData();
		}
		// They already trumped - try to over-trump
		Card* over = LowestBeater( bysuit[trumphsuit], best );
		if( over )
			return over;
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

plindex_t SmartPlayer::PlayerIndex( Player* player )
{
	if( player == left )
		return SBOT_LEFT;
	if( player == right )
		return SBOT_RIGHT;
	if( player == partner )
		return SBOT_PARTNER;
	return SBOT_THIS;
}

bool SmartPlayer::IsOut( int type_id, int suit_id )
{
	CardList::Node* node = out.GetFirst();
	while( node ) {
		Card* card = node->GetData();
		if( card->GetType().GetId() == type_id &&
		    card->GetSuit().GetId() == suit_id )
			return true;
		node = node->GetNext();
	}
	return false;
}
