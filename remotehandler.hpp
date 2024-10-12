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

#ifndef _REMOTEHANDLER_HPP_
#define _REMOTEHANDLER_HPP_ 1

// Forward declarations
class RemoteHandler;

#include "remotegame.hpp"
#include "netcommon.hpp"
#include "myevents.hpp"
#include <wx/event.h>
#include <wx/hashmap.h>

#define EVT_FINISH_REMOTE_HANDLER(func) DECLARE_EVENT_TABLE_ENTRY( FINISH_REMOTE_HANDLER_TYPE, wxID_ANY, wxID_ANY, ( wxObjectEventFunction ) & func, (wxObject *) NULL ),

class FinishRemoteHandlerEvt: public wxEvent
{
public:
  RemoteHandler* handler;
  FinishRemoteHandlerEvt( RemoteHandler* the_handler ):
    wxEvent( 0, FINISH_REMOTE_HANDLER_TYPE ), handler( the_handler ) {}
  wxEvent *Clone(void) const { return new FinishRemoteHandlerEvt( *this ); }
};

#define N_RESPONSES ( RESP_SAY + 1 )
enum ResponseEnum { RESP_FIRST, RESP_POS, RESP_GAME, RESP_ROUND, RESP_PLAY,
		    RESP_WINNER, RESP_NOTURN, RESP_INVMOVE, RESP_NAME,
		    RESP_SAY };

// Class for hashing responses only once
class ResponseClass: public CommandMap
{
public:
  ResponseClass();
};

WX_DECLARE_STRING_HASH_MAP( Player*, PlayerMap );

// Remote network game handler
class RemoteHandler: public wxEvtHandler
{
public:
  RemoteHandler();
  ~RemoteHandler();
  void SetSocket( wxSocketBase* socket );
  wxSocketBase* GetSocket() { return m_socket; }
  bool IsConnected() { return m_connected; }
  void Send( wxString message );
  void TerminateConnection();
  void OnSocketEvent( wxSocketEvent& event );
private:
  static ResponseClass response_type;
  wxString m_reading;
  bool m_connected;
  bool m_authenticated;
  wxSocketBase* m_socket;
  PlayerMap pmap;
  DECLARE_EVENT_TABLE();
};

#endif // _REMOTEHANDLER_HPP_
