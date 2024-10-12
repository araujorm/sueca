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

#include <cstdlib>  // for rand() and srand()
#include <ctime>  // time()
#include "player.hpp"

int GamePos::xgap = (MC_X_SIZE-9*CARDBMP_INCR-CARDBMP_W)/2;
int GamePos::ygap = (MC_Y_SIZE-9*CARDBMP_INCR-CARDBMP_H)/2;

void GamePos::ResetCheck()
{
  if( i == MAX_CARDS )
    i = 0;
}

// Posiotioning methods
GamePosP1::GamePosP1(): GamePos()
{
  m_name = "bottom";
  xstart = xgap;
  ystart = pos.y = MC_Y_SIZE - CARDBMP_GAP - CARDBMP_H;
};

wxPoint& GamePosP1::NextPosition()
{
  pos.x = xstart + ( i++ ) * CARDBMP_INCR;
  ResetCheck();
  return (wxPoint&)pos;
}

GamePosP2::GamePosP2(): GamePos()
{
  m_name = "right";
  xstart = pos.x = MC_X_SIZE - CARDBMP_GAP - CARDBMP_W;
  ystart = MC_Y_SIZE - ygap - CARDBMP_H;
}

wxPoint& GamePosP2::NextPosition()
{
  pos.y = ystart - (i++) * CARDBMP_INCR;
  ResetCheck();
  return (wxPoint&)pos;
}

GamePosP3::GamePosP3(): GamePos()
{
  m_name = "top";
  xstart = MC_X_SIZE - xgap - CARDBMP_W;
  ystart = pos.y = CARDBMP_GAP;
};

wxPoint& GamePosP3::NextPosition()
{
  pos.x = xstart - (i++) * CARDBMP_INCR;
  ResetCheck();
  return (wxPoint&)pos;
}

GamePosP4::GamePosP4(): GamePos()
{
  m_name = "left";
  xstart = pos.x = CARDBMP_GAP;
  ystart = ygap;
};

wxPoint& GamePosP4::NextPosition()
{
  pos.y = ystart + (i++) * CARDBMP_INCR;
  ResetCheck();
  return (wxPoint&)pos;
}

// Base player class implementation
Player::Player( GamePos* gamepos ):
  m_gamepos( gamepos ), m_hidden_cards( true ), m_team( NULL ) {}

void Player::AddToHand( Card* newcard, MyCanvas* canvas )
{
  m_hand.Append( newcard );
  wxPoint pos = m_gamepos->NextPosition();
  canvas->Add( newcard, pos.x, pos.y, AreCardsHidden() );
}

bool Player::IsValidMove( const Card* card, const CardList& played )
{
  if( ! GetHand().Find( card ) )
    return false;  // Trying to play a card you don't have
  if( played.GetCount() == 0 )
    return true;  // First to play
  CardSuit current_suit = played.GetFirst()->GetData()->GetSuit();
  if( card->GetSuit() == current_suit )
    return true;
  CardList::Node* node = GetHand().GetFirst();
  while( node ) {
    if( node->GetData()->GetSuit() == current_suit )
      return false;  // Trying to cheat :)
    node = node->GetNext();
  }
  return true;
}

void Player::SetGamePos( GamePos* newpos )
{
  delete m_gamepos;
  m_gamepos = newpos;
}

// Team class implementation
Team::Team( Player* np1, Player* np2 ):
  m_won( 0 ), m_points( 0 ), p1( np1 ), p2( np2 ), wonstr( "" )
{
  p1->SetTeam( this );
  p2->SetTeam( this );
}

void Team::AddToCapt( CardList& cards )
{
  CardList::Node* node = cards.GetFirst();
  while( node ) {
    Card* crd = node->GetData();
    m_points += crd->GetType().GetValue();
    captured.Append( crd );
    node = node->GetNext();
  }
}

void Team::NewRound( Card* trumph, Player* owner )
{
  m_points = 0;
  captured.Clear();
  p1->NewRound( trumph, owner );
  p2->NewRound( trumph, owner );
}

bool Team::Replace( Player* oldplayer, Player* newplayer )
{
  if( oldplayer == p1 )
    p1 = newplayer;
  else if( oldplayer == p2 )
    p2 = newplayer;
  else
    return false;
  newplayer->SetTeam( this );
  return true;
}

// Human players class implementations
HumanPlayer::HumanPlayer( const wxString& name, GamePos* gamepos ):
  Player( gamepos )
{
  SetName( name );
}

LocalPlayer::LocalPlayer( const wxString& name, GamePos* gamepos ):
  HumanPlayer( name, gamepos )
{
  m_hidden_cards = false;
}

void LocalPlayer::AddToHand( Card* newcard, MyCanvas* canvas )
{
  // Sort cards as we add them to the player's hand
  CardList::Node *node = GetHand().GetFirst();
  while( node && newcard->GetSuit() > node->GetData()->GetSuit() )
    node = node->GetNext();
  if ( !node )
    GetHand().Append( newcard );
  else {
    while( node &&
           newcard->GetSuit() == node->GetData()->GetSuit() &&
           newcard->GetType() > node->GetData()->GetType() )
      node = node->GetNext();
    if ( !node )
      GetHand().Append( newcard );
    else
      GetHand().Insert( node, newcard );
  }
  // Delay canvas placing until we reach the last card
  // because of the sorting
  if ( GetHand().GetCount() == 10 ) {
    node = GetHand().GetFirst();
    while( node ) {
      Card* card = node->GetData();
      wxPoint pos = m_gamepos->NextPosition();
      canvas->Add( card, pos.x, pos.y, AreCardsHidden() );
      card->SetPlayable();
      node = node->GetNext();
    }
  }
}

// Computer players class implementations
char* BotPlayer::botnames[N_BOT_NAMES] = {
  "George", "Tony", "Saddam", "Bin",
  "Bill", "Steve", "Linus", "Alan",
  "Lula", "Jorge", "Durao", "Ferro",
  "Paulo", "Carlos"
};

bool BotPlayer::nameused[N_BOT_NAMES] = { false };

BotPlayer::BotPlayer( GamePos* gamepos ):
  Player( gamepos )
{
  // Random name, avoiding repeated ones
  do {
    nameind = (int)( (double)N_BOT_NAMES * rand() / ( RAND_MAX + 1.0 ) );
  } while( nameused[nameind] );  // N_BOT_NAMES should be >= 4 ;)
  nameused[nameind] = true;
  SetName( botnames[nameind] );
}

BotPlayer::~BotPlayer()
{
  // "Free" the name
  nameused[nameind] = false;
}

void BotPlayer::OnMyTurn( Game* game, const CardList& played )
{
  Card *toplay;
  do {
    toplay = PlayCard( & played );
  } while( game->PlayMove( this, toplay ) != MOVE_OK );
}

// Dumb player implementation
DumbPlayer::DumbPlayer( GamePos* gamepos ):
  BotPlayer( gamepos ) {}

Card* DumbPlayer::PlayCard( const CardList* played )
{
  Card *toplay;
  CardList::Node* node = GetHand().GetFirst();
  while( !IsValidMove( ( toplay = node->GetData() ), *played ) )
    node = node->GetNext();
  return toplay;
}

