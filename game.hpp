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

#ifndef _GAME_HPP_
#define _GAME_HPP_ 1

// Forward declarations
class PlayerIteratorNode;
class PlayerIterator;
class EndTurnTimer;
class Game;

#include <wx/timer.h>
#include "cards.hpp"
#include "mycanvas.hpp"
#include "player.hpp"
#include "scoredialog.hpp"
#include "trumphdialog.hpp"

// Circular player iterator node class
class PlayerIteratorNode
{
public:
  PlayerIteratorNode( Player* player, PlayerIteratorNode* next );
  Player* GetData() const { return m_player; }
  PlayerIteratorNode* GetNext() const { return m_next; }
  void SetNext( PlayerIteratorNode* next ) { m_next = next; }
private:
  Player* m_player;
  PlayerIteratorNode* m_next;
};

// Circular player iterator class
class PlayerIterator
{
public:
  PlayerIterator( Player *p1, Player *p2, Player *p3, Player *p4 );
  PlayerIterator( PlayerIterator& original );
  ~PlayerIterator();
  bool SetCurrent( Player *player );
  void SetCurrent( size_t n );
  Player* GetCurrent() { return m_current->GetData(); }
  Player* GetNext();
  Player* operator []( size_t n_index );
  PlayerIteratorNode* GetFirst() { return m_first; }
  bool Replace( Player* oldplayer, Player* newplayer );
private:
  PlayerIteratorNode* m_first;
  PlayerIteratorNode* m_current;
};

// Timer to wait when the turn ends before collecting cards
class EndTurnTimer: public wxTimer
{
public:
  EndTurnTimer( Game* game );
  void Notify();
private:
  Game* m_game;
};

// Game engine class
enum movestatus_t { MOVE_OK, MOVE_TURN, MOVE_INVALID, MOVE_DELAYED };
class Game
{
public:
  ScoreDialog* score;
  TrumphDialog* trumphdlg;

  Game( Player* p1,
	Player* p2,
	Player* p3,
	Player* p4,
	MyCanvas *the_canvas );
  virtual ~Game();
  Deck& GetDeck() const { return (Deck&)m_deck; }
  PlayerIterator* GetPlayers() { return new PlayerIterator( *m_players ); }
  Player* TurnWinner();
  void CardMoved( Card* card );
  virtual void NewRound();
  virtual void EndTurn();
  unsigned short CalcWonGames( Team** win );
  void PassTurn( Player* player = NULL );
  virtual movestatus_t PlayMove( Player *player, Card *card );
  CardList& GetPlayed() const { return (CardList&)m_played; }
  Card* GetTrumph() const { return m_trumph; }
  bool ReplacePlayer( Player* oldplayer, Player* newplayer );
  virtual void SetPlayerName( Player* player, const wxString& newname );
  void RefreshNames();
  void DisplayResults();

protected:
  MyCanvas *canvas;
  // Ensure deck is initialized after the wxApp derived class has started,
  // or wxBitmap objects creation may cause segfaults under wxGTK.
  Deck m_deck;
  PlayerIterator *m_players;
  PlayerIterator *m_roundpos;
  Team *team1;
  Team *team2;
  CardList m_played;
  Card* m_trumph;
  Player* trumph_owner;
  unsigned short m_cards_to_collect;
  EndTurnTimer m_endturntimer;
  unsigned short turns_left;
  bool playtime;

private:
  static Game* running;
};

#endif // _GAME_HPP_
