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
