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
  // Remove the card we played from our list
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
      cardsuit_t suit_i = card->GetSuit().GetId();
      released[pli][suit_i]++;
      if( suit_i != firstsuit )
	plhasnot[pli][suit_i] = true;
    }
    players->GetNext();
  }
}

Card* SmartPlayer::PlayCard( const CardList* played )
{
  Card *crdpartner, *crdleft, *crdright, *first;
  cardsuit_t trumphsuit = trumph->GetSuit().GetId();

  // See which card belongs to whom
  CardList::Node* node = played->GetFirst();
  if( !node ) {
    // We are the first to play
    // Start by trying our best non-trumph cards
    for( int thissuit = SUITMIN; thissuit <= SUITMAX; thissuit++ ) {
      if( thissuit == trumphsuit ||
	  released[SBOT_LEFT][thissuit] > 1 ||
	  released[SBOT_RIGHT][thissuit] > 1 ||
	  n_out[thissuit] > 4 )
	continue;
      if( ( node = bysuit[thissuit].GetLast() ) ) {
	Card* card = node->GetData();
	switch( cardtype_t cardtp = card->GetType().GetId() ) {
	case ACE:
	  if(  !plhasnot[SBOT_LEFT][cardtp] && !plhasnot[SBOT_RIGHT][cardtp] )
	    return ( played_card = card );
	  break;
	case SEVEN:
	  if( !plhasnot[SBOT_LEFT][cardtp] && !plhasnot[SBOT_RIGHT][cardtp] &&
	      IsOut( ACE, thissuit ) )
	    ;
	    return ( played_card = card );
	  break;
	default:
	  break;
	}
      }
    }
    // Try a trumph card
    if( ( node = bysuit[trumphsuit].GetLast() ) ) {
      return ( played_card = node->GetData() );
    }
  }
  else {
    //Player *winner = thegame->TurnWinner();
    first = node->GetData();
    switch( played->GetCount() ) {
    case 1:
      crdleft = first;
      crdpartner = crdright = NULL;
      break;
    case 2:
      crdpartner = first;
      crdleft = node->GetNext()->GetData();
      crdright = NULL;
      break;
    case 3:
    default:
      crdright = first;
      crdpartner = node->GetNext()->GetData();
      crdleft = node->GetNext()->GetNext()->GetData();
      break;
    }
    // TODO
  }
  // Awful game right now... play anything, like a dumb player
  Card *toplay;
  node = GetHand().GetFirst();
  while( !IsValidMove( ( toplay = node->GetData() ), *played ) )
    node = node->GetNext();
  return ( played_card = toplay );
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
