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

// Strategy when we are the first to play
Card* SmartPlayer::PlayFirst()
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();

	// Try our best non-trump cards first
	for( int s = SUITMIN; s <= SUITMAX; s++ ) {
		if( s == trumphsuit || bysuit[s].GetCount() == 0 )
			continue;
		// Skip suits where opponents have shown they don't have it (they'd trump)
		if( plhasnot[SBOT_LEFT][s] || plhasnot[SBOT_RIGHT][s] )
			continue;
		// Skip suits where many cards are already out (less predictable)
		if( n_out[s] > 6 )
			continue;
		CardList::Node* node = bysuit[s].GetLast();
		if( node ) {
			Card* card = node->GetData();
			// Play ace if we have it
			if( card->GetType().GetId() == ACE )
	return card;
			// Play 7 if ace of this suit is already out
			if( card->GetType().GetId() == SEVEN && IsOut( ACE, s ) )
	return card;
		}
	}

	// Try leading with a trump if we have one
	if( bysuit[trumphsuit].GetCount() > 0 )
		return bysuit[trumphsuit].GetLast()->GetData();

	// Fall back: play the lowest value card we have
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

	// Case: can't follow suit
	if( partner_winning ) {
		// Partner is winning - don't trump! Dump highest value non-trump
		Card* dump = NULL;
		for( int s = SUITMIN; s <= SUITMAX; s++ ) {
			if( s == trumphsuit || bysuit[s].GetCount() == 0 )
	continue;
			Card* high = HighestValue( bysuit[s] );
			if( high && ( !dump ||
			              high->GetType().GetValue() > dump->GetType().GetValue() ) )
	dump = high;
		}
		if( dump )
			return dump;
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
