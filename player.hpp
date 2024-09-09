#ifndef _PLAYER_HPP_
#define _PLAYER_HPP_ 1

// Forward declarations
class GamePos;
class GamePosP1;
class GamePosP2;
class GamePosP3;
class GamePosP4;
class Player;
class Team;
class HumanPlayer;
class LocalPlayer;
class BotPlayer;
class DumbPlayer;

#ifndef MAX_CARDS
#define MAX_CARDS 10
#endif  // MAX_CARDS

#include "mycanvas.hpp"
#include "cards.hpp"
#include "game.hpp"

// Game Layout
// Canvas dimensions MC_X_SIZE, MC_Y_SIZE defined in mycanvas.hpp
#define CARDBMP_W 71
#define CARDBMP_H 96
#define CARDBMP_INCR 15
#define CARDBMP_GAP 10
#define PLAYED_CARD_GAP 20
#define NAME_GAP 15
#define TRUMPH_GAP 10
// Game layout positioners
class GamePos
{
public:
  GamePos(): i( 0 ) {}
  wxString& GetName() { return m_name; }
  virtual ~GamePos() {}
  virtual wxPoint& NextPosition() = 0;
  virtual wxPoint NamePosition( wxSize& size ) = 0;
  virtual wxPoint PlayedCardPos() const = 0;
  virtual wxPoint CollectPos() const = 0;
  virtual wxPoint TrumphPos( wxSize& size ) const = 0;
  virtual GamePos* Clone() const = 0;
protected:
  void ResetCheck();
  static int xgap;
  static int ygap;
  int i;
  int xstart;
  int ystart;
  wxPoint pos;
  wxString m_name;
};

class GamePosP1: public GamePos
{
public:
  GamePosP1();
  wxPoint& NextPosition();
  wxPoint NamePosition( wxSize& size )
    { return wxPoint( xstart - size.GetWidth() - NAME_GAP, ystart + ( CARDBMP_H - size.GetHeight() ) / 2); }
  wxPoint PlayedCardPos() const { return wxPoint( ( MC_X_SIZE - CARDBMP_W ) / 2, MC_Y_SIZE / 2 + PLAYED_CARD_GAP ); }
  wxPoint CollectPos() const { return wxPoint( ( MC_X_SIZE - CARDBMP_W ) / 2, MC_Y_SIZE + CARDBMP_H ); }
  wxPoint TrumphPos( wxSize& size ) const { return wxPoint( xstart + 9 * CARDBMP_INCR + CARDBMP_W + TRUMPH_GAP, ystart + CARDBMP_H - size.GetHeight() ); }
  GamePos* Clone() const { return new GamePosP1( *this ); }
};

class GamePosP2: public GamePos
{
public:
  GamePosP2();
  wxPoint& NextPosition();
  wxPoint NamePosition( wxSize& size )
    { return wxPoint( xstart + ( CARDBMP_W - size.GetWidth() ) / 2, ystart + NAME_GAP + CARDBMP_H ); }
  wxPoint PlayedCardPos() const { return wxPoint( MC_X_SIZE / 2 + PLAYED_CARD_GAP, ( MC_Y_SIZE - CARDBMP_H ) / 2 ); }
  wxPoint CollectPos() const { return wxPoint( MC_X_SIZE + CARDBMP_W, ( MC_Y_SIZE - CARDBMP_H ) / 2 ); }
  wxPoint TrumphPos( wxSize& size ) const { return wxPoint( xstart + CARDBMP_W - size.GetWidth(), ystart - 9 * CARDBMP_INCR - TRUMPH_GAP - size.GetHeight() ); }
  GamePos* Clone() const { return new GamePosP2( *this ); }
};

class GamePosP3: public GamePos
{
public:
  GamePosP3();
  wxPoint& NextPosition();
  wxPoint NamePosition( wxSize& size )
    { return wxPoint( xstart + NAME_GAP + CARDBMP_W, ystart + ( CARDBMP_H - size.GetHeight() ) / 2); }
  wxPoint PlayedCardPos() const { return wxPoint( ( MC_X_SIZE - CARDBMP_W ) / 2, MC_Y_SIZE / 2 - PLAYED_CARD_GAP - CARDBMP_H ); }
  wxPoint CollectPos() const { return wxPoint( ( MC_X_SIZE - CARDBMP_W ) / 2, - CARDBMP_H ); }
  wxPoint TrumphPos( wxSize& size ) const { return wxPoint( xstart - 9 * CARDBMP_INCR - TRUMPH_GAP - size.GetWidth(), ystart ); }
  GamePos* Clone() const { return new GamePosP3( *this ); }
};

class GamePosP4: public GamePos
{
public:
  GamePosP4();
  wxPoint& NextPosition();
  wxPoint NamePosition( wxSize& size )
    { return wxPoint( xstart + ( CARDBMP_W - size.GetWidth() ) / 2, ystart - size.GetHeight() - NAME_GAP); }
  wxPoint PlayedCardPos() const { return wxPoint( MC_X_SIZE / 2 - PLAYED_CARD_GAP - CARDBMP_W, ( MC_Y_SIZE - CARDBMP_H ) / 2 ); }
  wxPoint CollectPos() const { return wxPoint( - CARDBMP_W, ( MC_Y_SIZE - CARDBMP_H ) / 2 ); }
  wxPoint TrumphPos( wxSize& size ) const { return wxPoint( xstart, ystart + 9 * CARDBMP_INCR + CARDBMP_H + TRUMPH_GAP ); }
  GamePos* Clone() const { return new GamePosP4( *this ); }
};

// Generic player (abstract class)
class Player
{
public:
  Player( GamePos* gamepos );
  virtual ~Player() { delete m_gamepos; }
  virtual void NewGame( Game* game ) {}
  virtual void NewRound( Card* trumph, Player* owner ) {}
  virtual void NewTurn( Player* starter ) {}
  virtual void Turn( Player* player, Card* card ) {}
  virtual void TurnEnd( const Player* winner, const CardList& played ) {}
  virtual void OnMyTurn( Game* game, const CardList& played ) = 0;
  virtual void AddToHand( Card* newcard, MyCanvas* canvas );
  wxString& GetName() { return m_name; }
  void SetName( const wxString& name ) { m_name = name; }
  void SetTeam( Team* newteam ) { m_team = newteam; }
  Team* GetTeam() { return m_team; }
  bool AreCardsHidden() { return m_hidden_cards; }
  wxPoint GetNamePos( wxSize labelsize ) { return m_gamepos->NamePosition( labelsize ); }
  wxPoint GetTrumphPos( wxSize labelsize ) { return m_gamepos->TrumphPos( labelsize ); }
  wxString GetNamePosStr() const { return m_gamepos->GetName(); }
  wxPoint GetPlayPos() { return m_gamepos->PlayedCardPos(); }
  wxPoint GetCollectPos() { return m_gamepos->CollectPos(); }
  bool IsValidMove( const Card* card, const CardList& played );
  void Remove( Card* card ) { GetHand().DeleteObject( card ); }
  CardList& GetHand() const { return (CardList&)m_hand; }
  void SetGamePos( GamePos* newpos );
  GamePos* GetGamePosCopy() const { return m_gamepos->Clone(); }
protected:
  GamePos* m_gamepos;
  bool m_hidden_cards;
  wxString m_name;
private:
  Team* m_team;
  CardList m_hand;
};

// Player team
class Team {
public:
  Team( Player* np1, Player* np2 );
  unsigned short GetRoundPoints() const { return m_points; }
  void AddToWon( unsigned short won ) { m_won += won; }
  unsigned short GetWon() const { return m_won; }
  void AddToCapt( CardList& cards );
  const CardList& GetCapt() const { return captured; }
  void NewRound( Card* trumph, Player* owner );
  bool Belongs( Player *player ) const { return player == p1 || player == p2; }
  Player* GetP1() const { return p1; }
  Player* GetP2() const { return p2; }
  wxString& GetWonStr() const { return (wxString&)wonstr; }
  void SetWonStr( wxString str ) { wonstr = str; }
  bool Replace( Player* oldplayer, Player* newplayer );
private:
  unsigned short m_won;
  unsigned short m_points;
  Player* p1;
  Player* p2;
  CardList captured;
  wxString wonstr;
};

// Human player
class HumanPlayer: public Player {
public:
  HumanPlayer( const wxString& name, GamePos* gamepos );
  virtual ~HumanPlayer() {}
  virtual void OnMyTurn( Game* game, const CardList& played ) {}
};

class LocalPlayer: public HumanPlayer {
public:
  LocalPlayer( const wxString& name, GamePos* gamepos );
  void AddToHand( Card* newcard, MyCanvas* canvas );
};

// Computer player (abstract class)
// N_BOT_NAMES should match the number of bot names in player.cpp
#define N_BOT_NAMES 14
class BotPlayer: public Player {
public:
  BotPlayer( GamePos* gamepos );
  virtual ~BotPlayer();
  void OnMyTurn( Game* game, const CardList& played );
  virtual Card* PlayCard( const CardList* played ) = 0;
private:
  int nameind;
  static char* botnames[N_BOT_NAMES];
  static bool nameused[N_BOT_NAMES];
};

// Specific computer players
// Dumb player: plays first available card
class DumbPlayer: public BotPlayer {
public:
  DumbPlayer( GamePos* gamepos );
  Card* PlayCard( const CardList* played );
};

#endif // _PLAYER_HPP_
