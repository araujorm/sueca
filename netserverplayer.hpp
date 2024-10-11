#ifndef _NETSERVERPLAYER_HPP_
#define _NETSERVERPLAYER_HPP_ 1

// Forward declarations
class NetServerPlayer;

#include "player.hpp"
#include "serverhandler.hpp"
#include <wx/socket.h>

// Server side network player
class NetServerPlayer: public HumanPlayer {
public:
  NetServerPlayer( const wxString& name,
		   GamePos* gamepos,
		   wxSocketBase* socket );
  virtual ~NetServerPlayer();
  virtual void NewGame( Game* game );
  virtual void NewRound( Card* trumph, Player* owner );
  virtual void NewTurn( Player* starter );
  virtual void Turn( Player* player, Card* card );
  virtual void TurnEnd( const Player* winner, const CardList& played );
  virtual void OnMyTurn( Game* game, const CardList& played );
protected:
  friend class ServerHandler;
  wxSocketBase* GetSocket() { return m_socket; }
  wxString reading;
private:
  wxSocketBase* m_socket;
};

#endif // _NETSERVERPLAYER_HPP_
