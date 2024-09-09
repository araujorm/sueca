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
