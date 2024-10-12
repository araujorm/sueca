/*
sueca - An implementation of the Portuguese game "Sueca" in C++ and wxWidgets
Copyright (C) 2003-2024 Rodrigo Araujo

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

#include <cstdlib>  // For abs()
#include "game.hpp"
#include "main.hpp"

// Circular player iterator node class implementation
PlayerIteratorNode::PlayerIteratorNode( Player *player, PlayerIteratorNode* next ):
  m_player( player ), m_next ( next ) {}

// Circular player iterator class implementation
PlayerIterator::PlayerIterator( Player *p1,
                                Player *p2,
                                Player *p3,
                                Player *p4 )
{
  m_first = new PlayerIteratorNode( p1, NULL );
  m_current = new
    PlayerIteratorNode( p2, new
      PlayerIteratorNode( p3, new
        PlayerIteratorNode( p4, m_first)));
  m_first->SetNext( m_current );
  m_current = m_first;
}

PlayerIterator::PlayerIterator( PlayerIterator& original )
{
  // The 'original' iterator state must be preserved
  PlayerIteratorNode* or_node = original.GetFirst();
  PlayerIteratorNode* m_prev;
  m_first = m_prev = new PlayerIteratorNode( or_node->GetData(), NULL );
  while( ( or_node = or_node->GetNext() ) != original.GetFirst() ) {
    m_current = new PlayerIteratorNode( or_node->GetData(), NULL );
    m_prev->SetNext( m_current );
    m_prev = m_current;
  }
  m_prev->SetNext( m_first );
  m_current = m_first;
}

PlayerIterator::~PlayerIterator()
{
  for( int i = 0; i < 4; i++ ) {
    m_current = m_first;
    m_first = m_first->GetNext();
    delete m_current;
  }
}

bool PlayerIterator::SetCurrent( Player* player )
{
  PlayerIteratorNode* node = m_first;
  do {
    if( node->GetData() == player ) {
      m_current = node;
      return true;
    }
    node = node->GetNext();
  } while( node != m_first );
  return false;
}

void PlayerIterator::SetCurrent( size_t n )
{
  PlayerIteratorNode *node = m_first;
  while( n ) {
    node = node->GetNext();
    n--;
  }
  m_current = node;
}

Player* PlayerIterator::GetNext()
{
  m_current = m_current->GetNext();
  return m_current->GetData();
}

Player* PlayerIterator::operator []( size_t n_index )
{
  PlayerIteratorNode *node = m_current;
  while( n_index ) {
    node = node->GetNext();
    n_index--;
  }
  return node->GetData();
}

bool PlayerIterator::Replace( Player* oldplayer, Player* newplayer )
{
  PlayerIteratorNode *prev = m_current;
  PlayerIteratorNode *node = m_current->GetNext();
  while( node->GetData() != oldplayer ) {
    prev = node;
    node = node->GetNext();
    if( prev == m_current )
      return false;
  }
  PlayerIteratorNode *newnode =
    new PlayerIteratorNode( newplayer, node->GetNext() );
  prev->SetNext( newnode );
  if( node == m_current )
    m_current = newnode;
  delete node;
  return true;
}

// EndTurnTimer implementation
EndTurnTimer::EndTurnTimer( Game* game ):
  wxTimer(), m_game( game ) {}

void EndTurnTimer::Notify()
{
  m_game->EndTurn();
}

// Game engine class implementation
Game* Game::running = NULL;

Game::Game( Player* p1,
	    Player* p2,
	    Player* p3,
	    Player* p4,
	    MyCanvas *the_canvas ):
  canvas( the_canvas ), m_trumph( NULL ),
  trumph_owner( NULL ), m_cards_to_collect( 0 ), m_endturntimer( this ),
  playtime( false )
{
  // Make sure only one instance of a game will run at a time
  if( running )
    delete running;
  running = this;
  m_players = new PlayerIterator( p1, p2, p3, p4 );
  m_roundpos = new PlayerIterator( *m_players );
  team1 = new Team( (*m_players)[0], (*m_players)[2] );
  team2 = new Team( (*m_players)[1], (*m_players)[3] );
  // Set a random player to have the trumph at the first round
  // Note that the first to play will be the one at his right
  int first = (int)( 4.0 * rand() / ( RAND_MAX + 1.0 ) );
  m_roundpos->SetCurrent( first );
  // Score dialog
  MyFrame* frame = wxGetApp().GetFrame();
  score = new ScoreDialog( frame, team1, team2, frame->score_pos );
  score->Show( frame->viewscores );
  // Trumph dialog
  trumphdlg = new TrumphDialog( frame, frame->trumph_pos );
  trumphdlg->Show( frame->viewtrumph );
  // Data initialized, we now put the name labels on the canvas
  // and on the dialogs
  RefreshNames();
  // Now we are almost set, tell the players a new game has started
  for( int i = 0; i < 4; i++ )
    m_players->GetNext()->NewGame( this );
  // Initial round result
  frame->SetStatusText( "First Round", 1 );
  // Make sure we focus on game window
  frame->SetFocus();
}

Game::~Game()
{
  delete team1;
  delete team2;
  delete m_roundpos;
  Player* first, *player;
  first = player = m_players->GetCurrent();
  do {
    delete player;
    player = m_players->GetNext();
  } while( player != first );
  delete m_players;
  canvas->ClearCards();
  canvas->ClearNames();
  canvas->ClearTrumph();
  canvas->Refresh();
  MyFrame* frame = wxGetApp().GetFrame();
  // Store dialog windows positions
  frame->score_pos = score->GetPosition();
  frame->trumph_pos = trumphdlg->GetPosition();
  // Sane window closing
  score->Close( TRUE );
  trumphdlg->Close( TRUE );
  running = NULL;
}

Player* Game::TurnWinner()
{
  CardList::Node* node = m_played.GetFirst();
  Card* biggest = node->GetData();
  Player* current;
  Player* winner;
  current = winner = m_players->GetCurrent();
  while( ( node = node->GetNext() ) != NULL ) {
    Card* card = node->GetData();
    Player* player = m_players->GetNext();
    if( ( card->GetSuit() == m_trumph->GetSuit() &&
          ( biggest->GetSuit() != m_trumph->GetSuit() ||
            card->GetType() > biggest->GetType() ) ) ||
        ( card->GetSuit() == biggest->GetSuit() &&
          card->GetType() > biggest->GetType() ) ) {
          biggest = card;
          winner = player;
    }
  }
  // Ensure the iterator is restored to its initial state,
  // so it's safe to call this method anytime (e.g. within a bot player)
  m_players->SetCurrent( current );
  return winner;
}

void Game::CardMoved( Card* card )
{
  if( m_cards_to_collect ) {
    m_cards_to_collect--;
    canvas->Remove( card );
  }
  if( !m_cards_to_collect )
    PassTurn();
}

void Game::NewRound()
{
  turns_left = 10;
  m_deck.Shuffle();
  int n = 0;
  for( int p = 1; p <= 4; p++ ) {
    Player* pl = m_roundpos->GetNext();
    for( int i = 0; i < MAX_CARDS; i++ )
      pl->AddToHand( m_deck.cards[n++], canvas );
  }
  m_trumph = m_deck.cards[39];
  // Each team instance tells its players a new round is starting
  trumph_owner = m_roundpos->GetCurrent();
  team1->NewRound( m_trumph, trumph_owner );
  team2->NewRound( m_trumph, trumph_owner );
  canvas->SetTrumphLabel( trumph_owner, m_trumph );
  trumphdlg->UpdateTrumph( m_trumph, trumph_owner );
  Player* first = m_roundpos->GetNext();
  PassTurn( first );
}

void Game::EndTurn()
{
  Player* winner = TurnWinner();
  winner->GetTeam()->AddToCapt( m_played );
  score->UpdateRoundResults();
  // Inform players about the outcome (mainly for bot AI and network clients)
  for( int i = 0; i < 4; i++ )
    m_players->GetNext()->TurnEnd( winner, m_played );
  // Collect cards
  m_cards_to_collect = 4;
  CardList::Node* node = m_played.GetFirst();
  while( node ) {
    // Add events
    canvas->MoveCardTo( node->GetData(), winner->GetCollectPos() );
    node = node->GetNext();
  }
  m_played.Clear();
  turns_left--;
  m_players->SetCurrent( winner );
}

const wxString tiedstr( "(T)" );

unsigned short Game::CalcWonGames( Team** win )
{
  static short multiplier = 1;  // Victory multiplier (for ties)
  int dif = team1->GetRoundPoints() - team2->GetRoundPoints();
  if( dif == 0 ) {  // Tied game
    multiplier *= 2;
    team1->SetWonStr( tiedstr );
    team2->SetWonStr( tiedstr );
    *win = NULL;
    return 0;
  }
  Team *winner, *loser;
  if( dif > 0 ) {
    winner = team1;
    loser = team2;
  }
  else {
    winner = team2;
    loser = team1;
  }
  unsigned short victories;
  if( winner->GetCapt().GetCount() == 40 )
    victories = 4;
  else if( abs( dif ) > 60 )
    victories = 2;
  else
    victories = 1;
  victories *= multiplier;
  winner->AddToWon( victories );
  winner->SetWonStr( wxString::Format( "(%hu)", victories ) );
  loser->SetWonStr( "" );
  multiplier = 1;
  *win = winner;
  return victories;
}

void Game::PassTurn( Player *player )
{
  if( turns_left == 0 ) {
    // Update scores, show results
    Team* winner;
    wxString showstr;
    unsigned short vict;
    if( ( vict = CalcWonGames( &winner ) ) == 0 )
      showstr = wxString( "Last Round: tied" );
    else
      showstr = wxString::Format( "Last Round: %hu for %s/%s", vict,
                                   winner->GetP1()->GetName().c_str(),
                                   winner->GetP2()->GetName().c_str() );
    wxGetApp().GetFrame()->SetStatusText( showstr, 1 );
    score->SetEndRoundResults();
    NewRound();
    return;
  }
  if( m_played.GetCount() == 4 ) {
    // Wait some time before collecting the cards
    // The 'while' is to ensure the timer starts
    while( ! m_endturntimer.Start( 750, wxTIMER_ONE_SHOT ) );
    return;
  }
  if( player )
    m_players->SetCurrent( player );
  else
    player = m_players->GetCurrent();

  playtime = true;
  player->OnMyTurn( this, m_played );
}

movestatus_t Game::PlayMove( Player *player, Card *card )
{
  if( !playtime || player != m_players->GetCurrent() )
    return MOVE_TURN;  // Not this player's turn
  if( !player->IsValidMove( card, m_played ) )
    return MOVE_INVALID;  // Invalid move
  // Tell players which card was played, including this one for confirmation
  // (mainly for network games)
  for( int i = 0; i < 4; i++ )
    m_players->GetNext()->Turn( player, card );
  // Play the move
  m_played.Append( card );
  player->Remove( card );
  card->SetPlayable( false );
  card->SetTurned( false );
  playtime = false;
  canvas->MoveCardTo( card, player->GetPlayPos() );
  m_players->GetNext();
  return MOVE_OK;
}

bool Game::ReplacePlayer( Player* oldplayer, Player* newplayer )
{
  if( m_players->Replace( oldplayer, newplayer ) ) {
    m_roundpos->Replace( oldplayer, newplayer );
    oldplayer->GetTeam()->Replace( oldplayer, newplayer );
    newplayer->NewGame( this );
    CardList& newhand = newplayer->GetHand();
    for( CardList::Node* node = oldplayer->GetHand().GetFirst();
	 node;
	 node = node->GetNext() )
      newhand.Append( node->GetData() );
    newplayer->NewRound( m_trumph, trumph_owner );
    RefreshNames();
    if( playtime && m_players->GetCurrent() == newplayer )
      newplayer->OnMyTurn( this, m_played );
    return true;
  }
  return false;
}

void Game::RefreshNames()
{
  PlayerIterator* players = GetPlayers();
  players->SetCurrent( (size_t)0 );
  for( int i = P1_NAME; i <= P4_NAME; i++ )
    canvas->SetNameLabel( i, players->GetNext() );
  delete players;
  if( trumphdlg && trumph_owner )
    trumphdlg->UpdateTrumph( m_trumph, trumph_owner );
  if( score )
    score->RefreshNames();
}

void Game::DisplayResults()
{
  score->Show( true );
}
