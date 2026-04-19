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

#include "observingbot.hpp"

ObservingBot::ObservingBot( GamePos* gamepos ):
	BotPlayer( gamepos ), thegame( NULL ), players( NULL ) {}

ObservingBot::~ObservingBot()
{
	if( players )
		delete players;
}

void ObservingBot::NewGame( Game* game )
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

void ObservingBot::NewRound( Card* newtrumph, Player* newowner )
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

void ObservingBot::NewTurn( Player* starter )
{
	turnstarter = starter;
}

void ObservingBot::TurnEnd( const Player* winner, const CardList& played )
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
	// Count-based deduction: a suit has 10 cards total. Whatever isn't in
	// our hand and hasn't been played is spread among the three other
	// players - if that number drops to zero we can conclude all of them
	// are void of the suit, even without having observed the discard yet.
	// Setting plhasnot for all three tightens both the rule-based filters
	// (cash decisions, partner-winning safety checks, ...) and the PIMC
	// world sampler, which is the point of this bot "remembering" the
	// cards that have come out.
	for( int s = SUITMIN; s <= SUITMAX; s++ ) {
		if( 10 - n_out[s] - (int)bysuit[s].GetCount() <= 0 ) {
			plhasnot[SBOT_RIGHT][s] = true;
			plhasnot[SBOT_PARTNER][s] = true;
			plhasnot[SBOT_LEFT][s] = true;
		}
	}
}

// Determine current trick winner and the winning card
Player* ObservingBot::CurrentWinner( const CardList* played, Card** best )
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

// Would the still-unplayed adversary holding the trumph capture our
// 'candidate' with it? The trumph is public information: we know its
// identity and owner, and while it isn't in 'out' it's sitting in
// that owner's hand. If the owner is an adversary who hasn't yet
// played in the current trick and the trumph beats 'candidate', then
// the adversary will over-play our card for a trick we'd otherwise
// have won. Useful for deciding whether to invest a trump now or
// discard and wait.
bool ObservingBot::TrumphThreatensCandidate( Card* candidate,
                                             const CardList& played )
{
	if( !trumph || !candidate )
		return false;
	// Trumph already played? (Present in our 'out' list.)
	if( out.Find( trumph ) )
		return false;
	// Owner is a partner / ourselves - not a threat.
	if( trumphowner != left && trumphowner != right )
		return false;
	// Did the owner already play in this trick?
	PlayerIterator* pit = thegame->GetPlayers();
	pit->SetCurrent( turnstarter );
	Player* p = turnstarter;
	CardList::Node* n = played.GetFirst();
	while( n ) {
		if( p == trumphowner ) {
			delete pit;
			return false;
		}
		p = pit->GetNext();
		n = n->GetNext();
	}
	delete pit;
	// Still to play; would their trumph capture our candidate?
	return Beats( trumph, candidate );
}

// Does 'card' beat 'best' considering trumps?
bool ObservingBot::Beats( Card* card, Card* best )
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

// Pick a card from 'candidates' that beats 'best'.
//
// For non-trump suits, if the ace is among our beaters prefer it:
// playing it now banks 11 points we could otherwise lose to a later
// bad turn of events (a void opponent trumping, the suit never coming
// back), and it makes any 7 of the same suit we still hold a
// guaranteed winner from then on.
//
// For the trump suit the preference flips: the ace of trumps is
// unbeatable by definition, it can't be chopped later, and it's the
// single most useful card to hold back for a guaranteed win on a
// future trick - so over-trumping with a cheaper trump is the
// preferred play. This matches common Sueca practice ("cortar com o
// mais baixo que chegue").
Card* ObservingBot::LowestBeater( CardList& candidates, Card* best )
{
	cardsuit_t trumphsuit = trumph->GetSuit().GetId();
	Card* result = NULL;
	Card* ace = NULL;
	CardList::Node* node = candidates.GetFirst();
	while( node ) {
		Card* card = node->GetData();
		if( Beats( card, best ) ) {
			if( card->GetType().GetId() == ACE &&
			    card->GetSuit().GetId() != trumphsuit )
	ace = card;
			if( !result || card->GetType() < result->GetType() )
	result = card;
		}
		node = node->GetNext();
	}
	return ace ? ace : result;
}

// Find the highest value card in a list (prefer 7=10, A=11, K=4, etc.)
Card* ObservingBot::HighestValue( CardList& candidates )
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
Card* ObservingBot::LowestValue( CardList& candidates )
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
Card* ObservingBot::LowestNonTrumph()
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

plindex_t ObservingBot::PlayerIndex( Player* player )
{
	if( player == left )
		return SBOT_LEFT;
	if( player == right )
		return SBOT_RIGHT;
	if( player == partner )
		return SBOT_PARTNER;
	return SBOT_THIS;
}

bool ObservingBot::IsOut( int type_id, int suit_id )
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
