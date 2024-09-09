#ifndef _SERVERHANDLER_HPP_
#define _SERVERHANDLER_HPP_ 1

// Forward declarations
class ServerHandler;

#include "netserverplayer.hpp"
#include "netcommon.hpp"
#include <wx/socket.h>
#include <wx/list.h>
#include <wx/event.h>

WX_DECLARE_LIST( wxSocketServer, SockServList );

#define N_COMMANDS ( COM_SAY + 1 )
enum CommandEnum { COM_POS, COM_PLAY, COM_NAME, COM_SAY };

// Class for hashing commands only once
class CommandClass: public CommandMap
{
public:
  CommandClass();
};

// Server sockets handler
class ServerHandler: public wxEvtHandler
{
public:
  SockBaseList client_sockets;
  SockServList serv_sockets;
  //ServerHandler();
  ~ServerHandler();
  bool StartServer( wxSockAddress& address );
  void StopServer();
  void CloseClients();
  void ToAll( const wxString& msg );
  NetServerPlayer* NewPlayer( wxSocketBase* socket, wxString& name );
  void OnSocketEvent( wxSocketEvent& event );
  void OnServerEvent( wxSocketEvent& event );
  void SendPositions( NetServerPlayer* player );
private:
  static CommandClass command_type;
  wxSocketBase* m_socket;
  DECLARE_EVENT_TABLE();
};

#endif // _SERVERHANDLER_HPP_
