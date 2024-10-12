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

#ifndef _NETCOMMON_HPP_
#define _NETCOMMON_HPP_ 1

#include <wx/socket.h>
#include <wx/string.h>
#include <wx/list.h>
#include <wx/arrstr.h>
#include <wx/hashmap.h>

#define PORT_MAX 65535

// ids for sockets
enum { SOCKET_ID, SERVER_ID };

class Command
{
public:
  int com;
  wxArrayString args;
  Command( int com_id, wxArrayString& argv ):
    com( com_id ), args( argv ) {}
};

extern char* freestr;

WX_DECLARE_LIST( wxSocketBase, SockBaseList );
WX_DECLARE_LIST( Command, CommandList );
WX_DECLARE_STRING_HASH_MAP( int, CommandMap );

void SocketPrint( wxSocketBase* socket, const wxString& str );
void SocketPrintln( wxSocketBase* socket, const wxString& str );
bool ValidName( wxString& name );
CommandList* GetCommands( wxSocketBase* socket, const CommandMap& comhash, wxString& reading );

#endif  // _NETCOMMON_HPP_
