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

#include "serverdialog.hpp"
#include <wx/statbox.h>
#include <wx/checkbox.h>
#include <wx/msgdlg.h>
#include "main.hpp"
#include <wx/tokenzr.h>

enum { ID_IP_CHECKBOX, ID_LISTEN_BUTTON };

const char* listen_caption = "&Listen";

// Server creation dialog implementation
BEGIN_EVENT_TABLE( ServerDialog, wxDialog )
  EVT_CHECKBOX( ID_IP_CHECKBOX, ServerDialog::OnIpEnabled )
  EVT_BUTTON( ID_LISTEN_BUTTON, ServerDialog::OnListen )
  EVT_BUTTON( wxID_OK, ServerDialog::OnBegin )
  EVT_BUTTON( wxID_CANCEL, ServerDialog::OnCancel )
  EVT_CLOSE( ServerDialog::OnClose )
  EVT_CHAT_PANEL_MESSAGE( ServerDialog::OnChatMessage )
END_EVENT_TABLE();

ServerDialog::ServerDialog( wxWindow* parent ):
  wxDialog( parent, wxID_ANY, wxString( "Host network game" ) ),
  positions( 3 )
{
  Sueca& app = wxGetApp();
  app.servdlg = this;

  wxBoxSizer* top_sizer = new wxBoxSizer( wxVERTICAL );

  // IP entry
  wxBoxSizer* ip_sizer = new wxBoxSizer( wxHORIZONTAL );
  wxCheckBox* ip_checkbox = new wxCheckBox( this, ID_IP_CHECKBOX, "Bind to specific &IP address(es)" );
  ip_enabled = app.use_bound_ip_address;
  ip_checkbox->SetValue( ip_enabled );
  ip_sizer->Add( ip_checkbox, 0, wxRIGHT | wxALIGN_CENTER, 5 );
  disconnectedlist.Append( ip_checkbox );
  ip_entry = new wxTextCtrl( this, wxID_ANY, app.bound_ip_address, wxDefaultPosition, wxSize( 120, wxID_ANY ) );
  ip_entry->Enable( ip_enabled );
  ip_sizer->Add( ip_entry, 0, wxALIGN_CENTER );
  disconnectedlist.Append( ip_entry );
  top_sizer->Add( ip_sizer );
  wxStaticText* ip_hint_text = new wxStaticText( this, wxID_ANY, "(separate with ; if more than one)" );
  top_sizer->Add( ip_hint_text, 0, wxBOTTOM | wxALIGN_CENTER, 10 );
  disconnectedlist.Append( ip_hint_text );

  // Port entry
  wxBoxSizer* port_sizer = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText* port_text = new wxStaticText( this, wxID_ANY, "Port");
  port_sizer->Add( port_text, 0, wxRIGHT | wxALIGN_CENTER, 5 );
  disconnectedlist.Append( port_text );
  port_entry = new wxTextCtrl( this, wxID_ANY, wxString::Format( "%u", app.ip_port ), wxDefaultPosition, wxSize( 60, wxID_ANY ) );
  port_sizer->Add( port_entry, 0, wxALIGN_CENTER );
  disconnectedlist.Append( port_entry );
  top_sizer->Add( port_sizer, 0, wxBOTTOM, 10 );

  // Positions
  wxStaticBox* pos_box = new wxStaticBox( this, wxID_ANY, "Player positions" );
  wxStaticBoxSizer* pos_sizer =
    new wxStaticBoxSizer( pos_box, wxVERTICAL );
  wxStaticText* p1text = new wxStaticText( this, wxID_ANY, app.GetLocalPlayerName(), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
  wxStaticText* p2text = new wxStaticText( this, wxID_ANY, freestr, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
  p2text->Enable( false );
  wxStaticText* p3text = new wxStaticText( this, wxID_ANY, freestr, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE );
  p3text->Enable( false );
  wxStaticText* p4text = new wxStaticText( this, wxID_ANY, freestr, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT );
  p4text->Enable( false );
  wxBoxSizer* posmidsizer = new wxBoxSizer( wxVERTICAL );
  posmidsizer->Add( p4text, 0, wxALIGN_LEFT );
  posmidsizer->Add( 10, wxID_ANY, 1 );
  posmidsizer->Add( p2text, 0, wxALIGN_RIGHT );
  pos_sizer->Add( p3text, 0, wxALIGN_CENTER | wxBOTTOM, 20 );
  pos_sizer->Add( posmidsizer, 0, wxEXPAND | wxBOTTOM, 20 );
  pos_sizer->Add( p1text, 0, wxALIGN_CENTER );
  top_sizer->Add( pos_sizer, 0, wxEXPAND | wxBOTTOM, 10 );

  // Chat stuff
  wxStaticBox* chat_box = new wxStaticBox( this, wxID_ANY, "Chat" );
  chat_box->Enable( false );
  connectedlist.Append( chat_box );
  chat = new ChatPanel( this );
  chat->Enable( false );
  connectedlist.Append( chat );
  wxStaticBoxSizer* chat_sizer =
    new wxStaticBoxSizer( chat_box, wxVERTICAL );
  chat_sizer->Add( chat, 0, wxEXPAND );
  top_sizer->Add( chat_sizer, 0, wxEXPAND | wxBOTTOM, 10 );

  // Buttons
  wxBoxSizer* button_sizer = new wxBoxSizer( wxHORIZONTAL );
  listen_button = new wxButton( this, ID_LISTEN_BUTTON, listen_caption );
  button_sizer->Add( listen_button, 0, wxRIGHT, 10 );
  ok_button = new wxButton( this, wxID_OK, "&Begin" );
  ok_button->Enable( false );
  button_sizer->Add( ok_button, 0, wxRIGHT, 10 );
  button_sizer->Add( new wxButton( this, wxID_CANCEL, "&Cancel" ) );
  top_sizer->Add( button_sizer, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5 );

  // Invisible border
  main_sizer = new wxBoxSizer( wxVERTICAL );
  main_sizer->Add( top_sizer, 0, wxALL, 15 );
  SetSizer( main_sizer );
  main_sizer->SetSizeHints( this );

  listen_button->SetDefault();
  CentreOnParent();

  // Game slots
  positions["right"] = new Position( p2text, new GamePosP2() );
  positions["top"] = new Position( p3text, new GamePosP3() );
  positions["left"] = new Position( p4text, new GamePosP4() );
}

void ServerDialog::ReLayout()
{
  for( PosMap::iterator i = positions.begin(); i != positions.end(); i++ ) {
    wxControl* control = i->second->label;
    wxSize s = control->GetBestSize();
    control->SetSizeHints( s, s );
  }
  main_sizer->Layout();
  main_sizer->SetSizeHints( this );
  // TODO: Check why we must call this twice for the window to resize properly
  // Maybe a bug in wxGTK 2.5.2?
  main_sizer->SetSizeHints( this );
}

void ServerDialog::OnIpEnabled( wxCommandEvent& event )
{
  ip_entry->Enable( ip_enabled = event.IsChecked() );
  if( ip_enabled )
    ip_entry->SetFocus();
}

void ServerDialog::EndListen()
{
  // Re-enable connection related controls
  for( wxWindowList::Node* node = disconnectedlist.GetFirst(); node; node = node->GetNext() )
    node->GetData()->Enable( true );
  // Correct state of ip_entry
  ip_entry->Enable( ip_enabled );
  listen_button->SetLabel( listen_caption );
  // Disable connected controls if no client is connected
  if( ! wxGetApp().servhandler->client_sockets.GetCount() )
    for( wxWindowList::Node* node = connectedlist.GetFirst(); node; node = node->GetNext() )
      node->GetData()->Enable( false );
}

void ServerDialog::OnListen( wxCommandEvent& event )
{
  ServerHandler* servhandler = wxGetApp().servhandler;
  if( servhandler->serv_sockets.GetCount() ) {
    servhandler->StopServer();
    EndListen();
    chat->Write( "*** Stopped listening." );
  }
  else {
    // Keep settings as we apply them
    Sueca& app = wxGetApp();
    // Check port
    unsigned long newport;
    if( !port_entry->GetValue().ToULong( &newport ) || !newport || newport > PORT_MAX ) {
      wxMessageBox( "Invalid port.", "Error", wxOK | wxICON_ERROR );
      return;
    }
    app.ip_port = newport;

    // Bind specified addresses
    wxIPV4address addr;
    addr.Service( newport );
    app.use_bound_ip_address = ip_enabled;
    bool failed = false;
    wxString failmsg;
    if( ip_enabled ) {
      wxString newname = ip_entry->GetValue();
      newname.Trim( true );
      newname.Trim( false );
      app.bound_ip_address = newname;
      wxStringTokenizer tkz ( newname, ";", wxTOKEN_STRTOK );
      while( tkz.HasMoreTokens() ) {
	wxString token = tkz.GetNextToken();
	addr.Hostname( token );
	if( ! servhandler->StartServer( addr ) ) {
	  failed = true;
	  failmsg += wxString::Format("%s%s:%hu", failmsg.Len() ? ", " : "", token.c_str(), addr.Service() );
	}
      }
      if( ! servhandler->serv_sockets.GetCount() ) {
	failed = true;
	failmsg = "if you want to bind to a specific IP address please specify it";
      }
    }
    else
      if( ( failed = ! servhandler->StartServer( addr ) ) )
	failmsg = wxString::Format( "port %lu", newport );

    if( failed ) {
      servhandler->StopServer();
      wxMessageBox( wxString::Format( "Could not listen for connections%s.", failmsg.Len() ? wxString::Format( " (%s)", failmsg.c_str() ).c_str() : "" ), "Error", wxOK | wxICON_ERROR );
      return;
    }

    // We're now listening, so disable connection related controls
    for( wxWindowList::Node* node = disconnectedlist.GetFirst(); node; node = node->GetNext() )
      node->GetData()->Enable( false );
    // And enable connected related ones
    for( wxWindowList::Node* node = connectedlist.GetFirst(); node; node = node->GetNext() )
      node->GetData()->Enable( true );

    listen_button->SetLabel( "&Stop" );
    chat->Write( "*** Started listening." );
  }
}

Player* ServerDialog::GetPlayerForPos( wxString posname )
{
  Position* pos = positions[posname];
  Player* player = pos->player;
  if( player )
    return player;
  return wxGetApp().GetBotPlayer( pos->gpos->Clone() );
}

void ServerDialog::OnBegin( wxCommandEvent& event )
{
  // TODO: net players joining on a running game
  // For now we need to stop the server until the server dialog supports that
  // also
  wxGetApp().servhandler->StopServer();
  LocalPlayer* p1 = new LocalPlayer( wxGetApp().GetLocalPlayerName(), new GamePosP1() );
  Player* p2 = GetPlayerForPos( "right" );
  Player* p3 = GetPlayerForPos( "top" );
  Player* p4 = GetPlayerForPos( "left" );
  Sueca& app = wxGetApp();
  app.NewGame( new Game( p1, p2, p3, p4, app.GetFrame()->canvas ), p1 );
  Done( false );
}

void ServerDialog::Done( bool cancel )
{
  if( cancel ) {
    wxGetApp().servhandler->StopServer();
    wxGetApp().servhandler->CloseClients();
  }
  wxGetApp().servdlg = NULL;
  for( PosMap::iterator i = positions.begin(); i != positions.end(); i++ )
    delete i->second;
  if( IsModal() )
    EndModal(0);
  else
    Destroy();
}

void ServerDialog::OnChatMessage( ChatPanelMsgEvt& event )
{
  wxString& text = event.message;
  if( text.Len() ) {
    chat->SayInChat( wxGetApp().GetLocalPlayerName(), text );
    // Send to other clients
    wxGetApp().servhandler->ToAll( wxString::Format( "say:%s:%s", wxGetApp().GetLocalPlayerName().c_str(), text.c_str() ) );
  }
}

Position* ServerDialog::GetPlayerPosition( const Player* player )
{
  for( PosMap::iterator i = positions.begin(); i != positions.end(); i++ ) {
    Position* pos = i->second;
    if( pos->player == player )
      return pos;
  }
  return NULL;
}

// About this function: Returns the first position not matching the given
// player and places its (the position) name followed by other player's names
// and positions in the given string, all separated by colons; if the given
// player does not exist returns NULL. Note that a NULL player means a free
// slot so this is used, amongst other things, to get a free position.
Position* ServerDialog::GetNotMatchingPos( wxString& posstr, Player* player )
{
  posstr = "";
  Position* retpos = NULL;
  // Inform about our player
  wxString otherpositions = wxString::Format( ":%s:%s", wxGetApp().GetLocalPlayerName().c_str(), "bottom" );
  for( PosMap::iterator i = positions.begin(); i != positions.end(); i++ ) {
    Position* pos = i->second;
    Player* this_player = pos->player;
    if( this_player == player && !retpos ) {
      retpos = pos;
      posstr = i->first;
    }
    else
      otherpositions +=
	wxString::Format( ":%s:%s",
			  this_player ? this_player->GetName().c_str() : "",
			  i->first.c_str() );
  }
  posstr += otherpositions;
  return retpos;
}

void ServerDialog::SetPosition( Position* pos, NetServerPlayer* pl )
{
  if( pl ) {
    pl->SetGamePos( pos->gpos->Clone() );
    pos->label->SetLabel( pl->GetName() );
    pos->label->Enable( true );
  }
  else {
    pos->label->SetLabel( freestr );
    pos->label->Enable( false );
  }
  pos->player = pl;
}
