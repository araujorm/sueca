#ifndef _REMOTEGAME_HPP_
#define _REMOTEGAME_HPP_ 1

// Forward declarations
class RemoteGame;

#include "game.hpp"
#include "remotehandler.hpp"

// Remote game engine class
class RemoteGame: public Game
{
public:
  RemoteGame( LocalPlayer* p1,
	      HumanPlayer* p2,
	      HumanPlayer* p3,
	      HumanPlayer* p4,
	      MyCanvas *the_canvas,
	      RemoteHandler* handler );
  virtual ~RemoteGame();
  // Called by the local player
  virtual movestatus_t PlayMove( Player *player, Card *card );
  // These are to be ignored
  virtual void NewRound() {}
  virtual void EndTurn() {}
protected:
  friend class RemoteHandler;
  void PlayRemoteMove( Player *player, Card *card );
  void NewRound( Card* trumph, Player* owner, CardList& localcards );
  void EndTurn( Player* winner );
private:
  RemoteHandler* m_handler;
  LocalPlayer* localplayer;
  HumanPlayer* fake_players[3];
};

#endif // _REMOTEGAME_HPP_
