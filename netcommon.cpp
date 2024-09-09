#include "netcommon.hpp"
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include "definitions.hpp"

WX_DEFINE_LIST( SockBaseList );
WX_DEFINE_LIST( CommandList );

char* freestr = "<free>";

void SocketPrint( wxSocketBase* socket, const wxString& str )
{
  socket->Write( str.c_str(), str.Len() );
}

void SocketPrintln( wxSocketBase* socket, const wxString& str )
{
  socket->Write( ( str + '\n' ).c_str(), str.Len() + 1 );
}

CommandList* GetCommands( wxSocketBase* socket,
			  const CommandMap& comhash,
			  wxString& reading )
{
  CommandList* comlist = new CommandList();
  // Check incoming data from socket
  wxUint32 count;
  char buf[1024];
  // Get several lines
  while( ( count = socket->Read( buf, sizeof( buf ) - 1 ).LastCount() ) &&
	 ! socket->Error() ) {
    buf[count] = 0;
    bool incomplete = buf[count - 1] == '\n' ? false : true;
    wxStringTokenizer tkz( buf, "\r\n", wxTOKEN_STRTOK );
    int nlines = tkz.CountTokens();
    if( incomplete )
      nlines--;
    for( int i = 0; i < nlines; i++ ) {
      reading += tkz.GetNextToken();
      wxStringTokenizer argtkz( reading, ":" );
      if( argtkz.CountTokens() ) {
	wxString comstr = argtkz.GetNextToken();
	// Check for valid command
	CommandMap::const_iterator it = comhash.find( comstr );
	if( it != comhash.end() ) {
	  wxArrayString arguments;
	  arguments.Add( comstr );
	  // Get args
	  while( argtkz.HasMoreTokens() )
	    arguments.Add( argtkz.GetNextToken() );
	  comlist->Append( new Command( it->second, arguments ) );
	}
      }
      reading = "";
    }
    if( incomplete )
      reading += tkz.GetNextToken();    
  }
  return comlist;
}

bool ValidName( wxString& name )
{
  name.Trim( true );
  name.Trim( false );
  name.Truncate( PLAYER_NAME_MAX );
  if( ! name.Len() || name.Length() > PLAYER_NAME_MAX ||
      name.Find( ':' ) >= 0 )
    return false;
  return true;
}
