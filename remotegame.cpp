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

#include "remotegame.hpp"
#include "main.hpp"

// Remote game engine class implementation
RemoteGame::RemoteGame( LocalPlayer* p1,
			HumanPlayer* p2,
			HumanPlayer* p3,
			HumanPlayer* p4,
			MyCanvas *the_canvas,
			RemoteHandler* handler ):
  Game( p1, p2, p3, p4, the_canvas ), m_handler( handler )
{
  localplayer = p1;
  fake_players[0] = p2;
  fake_players[1] = p3;
  fake_players[2] = p4;
};

RemoteGame::~RemoteGame()
{
  // Safe remote handler deletion
  FinishRemoteHandlerEvt finevt( m_handler );
  wxGetApp().AddPendingEvent( finevt );
}

// To be called only by the local player
movestatus_t RemoteGame::PlayMove( Player *player, Card *card )
{
  m_handler->Send( "play:" + card->ShortStr() );
  return MOVE_DELAYED;
}

// Called from RemoteHandler
void RemoteGame::PlayRemoteMove( Player *player, Card *card )
{
  // Make sure only one card at a time is seen played
  while( ! playtime )
    wxGetApp().Yield();
  if( player == localplayer ) {
    player->Remove( card );
    card->SetPlayable( false );
  }
  else {
    // Pick a random fake card from the player's hand
    CardList& hand = player->GetHand();
    int rindex = (int)( hand.GetCount() * rand() / ( RAND_MAX + 1.0 ) );
    CardList::Node* node = hand.Item( rindex );
    Card* fake_card = node->GetData();
    // Move the card to the fake one's position
    wxPoint pos = fake_card->GetPosition();
    card->SetPosition( pos );
    canvas->Remove( fake_card, false );
    hand.DeleteNode( node );
    delete fake_card;
    canvas->Add( card, pos.x, pos.y, false );
  }
  m_played.Append( card );
  playtime = false; // TODO!!!!
  canvas->MoveCardTo( card, player->GetPlayPos() );
}

void RemoteGame::NewRound( Card* trumph, Player* owner, CardList& localcards )
{
  turns_left = 10;
  // Cards for localplayer
  for( CardList::Node* node = localcards.GetFirst();
       node;
       node = node->GetNext() )
    localplayer->AddToHand( node->GetData(), canvas );
  // Fake cards for the remote players, as we don't know their game
  for( int p = 0; p < 3; p++ ) {
    Player* pl = fake_players[p];
    for( int i = 0; i < MAX_CARDS; i++ )
      pl->AddToHand( new Card( *m_deck.nulcard ), canvas );
  }
  m_trumph = trumph;
  // Each team instance tells its players a new round is starting
  trumph_owner = owner;
  team1->NewRound( m_trumph, trumph_owner );
  team2->NewRound( m_trumph, trumph_owner );
  canvas->SetTrumphLabel( trumph_owner, m_trumph );
  trumphdlg->UpdateTrumph( m_trumph, trumph_owner );
  //Player* first = m_roundpos->GetNext();
  playtime = true;
  //PassTurn( first );
}

void RemoteGame::EndTurn( Player* winner )
{
  winner->GetTeam()->AddToCapt( m_played );
  score->UpdateRoundResults();
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
}

void RemoteGame::SetPlayerName( Player* player, const wxString& newname )
{
  if ( player == wxGetApp().GetLocalPlayer() ) {
    // Inform server about our name change
    m_handler->Send( "name:" + newname );
  }
  Game::SetPlayerName( player, newname );
}
