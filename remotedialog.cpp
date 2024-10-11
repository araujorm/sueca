#include "remotedialog.hpp"
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include "main.hpp"

// Remote connection dialog implementation
BEGIN_EVENT_TABLE( RemoteDialog, wxDialog )
  EVT_RADIOBUTTON( wxID_ANY, RemoteDialog::OnSelectPosition )
  EVT_BUTTON( wxID_OK, RemoteDialog::OnConnect )
  EVT_BUTTON( wxID_CANCEL, RemoteDialog::OnCancel )
  EVT_CLOSE( RemoteDialog::OnClose )
  EVT_CHAT_PANEL_MESSAGE( RemoteDialog::OnChatMessage )
END_EVENT_TABLE();

const char* noconnstr = "<not connected>";
const char* connect_caption = "C&onnect";

RemoteDialog::RemoteDialog( wxWindow* parent ):
  wxDialog( parent, wxID_ANY, wxString( "Connect to remote game" ) ), positions( 4 )
{
  Sueca& app = wxGetApp();
  app.rmtdlg = this;
  handler = new RemoteHandler();

  wxBoxSizer* top_sizer = new wxBoxSizer( wxVERTICAL );

  // IP entry
  wxBoxSizer* ip_sizer = new wxBoxSizer( wxHORIZONTAL );
  wxStaticText* ip_text = new wxStaticText( this, wxID_ANY, "Host address");
  ip_sizer->Add( ip_text, 0, wxRIGHT | wxALIGN_CENTER, 5 );
  disconnectedlist.Append( ip_text );
  ip_entry = new wxTextCtrl( this, wxID_ANY, app.connect_ip_address, wxDefaultPosition, wxSize( 120, wxID_ANY ) );
  ip_sizer->Add( ip_entry, 0, wxALIGN_CENTER | wxRIGHT, 10 );
  disconnectedlist.Append( ip_entry );

  // Port entry
  wxStaticText* port_text = new wxStaticText( this, wxID_ANY, "Port");
  ip_sizer->Add( port_text, 0, wxRIGHT | wxALIGN_CENTER, 5 );
  disconnectedlist.Append( port_text );
  port_entry = new wxTextCtrl( this, wxID_ANY, wxString::Format( "%u", app.ip_port ), wxDefaultPosition, wxSize( 60, wxID_ANY ) );
  ip_sizer->Add( port_entry, 0, wxALIGN_CENTER );
  disconnectedlist.Append( port_entry );
  top_sizer->Add( ip_sizer, 0, wxBOTTOM, 10 );

  // Positions
  wxStaticBox* pos_box = new wxStaticBox( this, wxID_ANY, "Player positions" );
  wxStaticBoxSizer* pos_sizer =
    new wxStaticBoxSizer( pos_box, wxVERTICAL );
  invisible = new wxRadioButton( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
  invisible->Show( false );
  invisible->SetValue( true );
  wxRadioButton* p1button = new wxRadioButton( this, ID_RADIOBUTTON_P1, noconnstr );
  p1button->Enable( false );
  connectedlist.Append( p1button );
  wxRadioButton* p2button = new wxRadioButton( this, ID_RADIOBUTTON_P2, noconnstr );
  p2button->Enable( false );
  connectedlist.Append( p2button );
  wxRadioButton* p3button = new wxRadioButton( this, ID_RADIOBUTTON_P3, noconnstr );
  p3button->Enable( false );
  connectedlist.Append( p3button );
  wxRadioButton* p4button = new wxRadioButton( this, ID_RADIOBUTTON_P4, noconnstr );
  p4button->Enable( false );
  connectedlist.Append( p4button );
  wxBoxSizer* posmidsizer = new wxBoxSizer( wxVERTICAL );
  posmidsizer->Add( p4button, 0, wxALIGN_LEFT );
  posmidsizer->Add( 10, wxID_ANY, 1 );
  posmidsizer->Add( p2button, 0, wxALIGN_RIGHT );
  pos_sizer->Add( p3button, 0, wxALIGN_CENTER | wxBOTTOM, 20 );
  pos_sizer->Add( posmidsizer, 0, wxEXPAND | wxBOTTOM, 20 );
  pos_sizer->Add( p1button, 0, wxALIGN_CENTER );
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
  connect_button = new wxButton( this, wxID_OK, connect_caption );
  button_sizer->Add( connect_button, 0, wxRIGHT, 10 );
  button_sizer->Add( new wxButton( this, wxID_CANCEL, "&Cancel" ) );
  top_sizer->Add( button_sizer, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5 );

  // Invisible border
  main_sizer = new wxBoxSizer( wxVERTICAL );
  main_sizer->Add( top_sizer, 0, wxALL, 15 );
  SetSizer( main_sizer );
  main_sizer->SetSizeHints( this );

  connect_button->SetDefault();
  CentreOnParent();

  // Game slots
  positions["bottom"] = posarray[ID_RADIOBUTTON_P1] = new RemotePosition( "bottom", p1button );
  positions["right"] = posarray[ID_RADIOBUTTON_P2] = new RemotePosition( "right", p2button );
  positions["top"] = posarray[ID_RADIOBUTTON_P3] = new RemotePosition( "top", p3button );
  positions["left"] = posarray[ID_RADIOBUTTON_P4] = new RemotePosition( "left", p4button );
}

void RemoteDialog::ReLayout()
{
  for( int i = ID_RADIOBUTTON_P1; i <= ID_RADIOBUTTON_P4; i++ ) {
    wxControl* control = posarray[i]->button;
    wxSize s = control->GetBestSize();
    control->SetSizeHints( s, s );
  }
  main_sizer->Layout();
  main_sizer->SetSizeHints( this );
  // TODO: Check why we must call this twice for the window to resize properly
  // Maybe a bug in wxGTK 2.5.2?
  main_sizer->SetSizeHints( this );
}

void RemoteDialog::OnSelectPosition( wxCommandEvent& event )
{
  RemotePosition* pos = posarray[event.GetId()];
  handler->Send( "position:" + pos->name );
  event.StopPropagation();
}

void RemoteDialog::ConnectionEndWarn( wxString message )
{
  wxMessageBox( message, "Error", wxOK | wxICON_EXCLAMATION );
  chat->Write( wxString::Format( "*** %s", message.c_str() ) );
  NoConnectionState();
}

void RemoteDialog::NoConnectionState()
{
  invisible->SetValue( true );
  // Re-enable connection related controls
  for( wxWindowList::Node* node = disconnectedlist.GetFirst(); node; node = node->GetNext() )
    node->GetData()->Enable( true );
  // Disable connection related controls
  for( wxWindowList::Node* node = connectedlist.GetFirst(); node; node = node->GetNext() )
    node->GetData()->Enable( false );
  for( int i = ID_RADIOBUTTON_P1; i <= ID_RADIOBUTTON_P4; i++ ) {
    wxRadioButton* button = posarray[i]->button;
    button->SetLabel( noconnstr );
  }
  connect_button->SetLabel( connect_caption );
  ReLayout();
}

void RemoteDialog::OnConnect( wxCommandEvent& event )
{
  if( handler->GetSocket() ) {
    if( handler->IsConnected() )
      // Disconnect
      chat->Write( "*** Disconnected." );
    else
      // Stop
      chat->Write( "*** Connection aborted." );
    handler->SetSocket( NULL );
    NoConnectionState();
  }
  else {
    // Connect
    // Keep settings as we apply them
    Sueca& app = wxGetApp();
    // Check IP address
    wxString host = ip_entry->GetValue();
    host.Trim( true );
    host.Trim( false );
    if( ! host.Len() ) {
      wxMessageBox( "Invalid address.", "Error", wxOK | wxICON_ERROR );
      return;
    }
    // Check port
    unsigned long newport;
    if( !port_entry->GetValue().ToULong( &newport ) || !newport || newport > PORT_MAX ) {
      wxMessageBox( "Invalid port.", "Error", wxOK | wxICON_ERROR );
      return;
    }
    app.connect_ip_address = host;
    app.ip_port = newport;

    // Connect to specified addresses
    wxIPV4address addr;
    addr.Service( newport );
    addr.Hostname( host );

    wxSocketClient* socket = new wxSocketClient( wxSOCKET_NOWAIT );
    handler->SetSocket( socket );
    socket->Connect( addr, false );

    // We're now trying to connect, so disable connection related controls
    for( wxWindowList::Node* node = disconnectedlist.GetFirst(); node; node = node->GetNext() )
      node->GetData()->Enable( false );
    chat->Write( wxString::Format( "*** Connecting to %s:%hu...", host.c_str(), short(newport) ) );
    connect_button->SetLabel( "St&op" );
  }
}

void RemoteDialog::Done( bool cancel )
{
  if( cancel )
    delete handler;
  wxGetApp().rmtdlg = NULL;
  if( IsModal() )
    EndModal(0);
  else
    Destroy();
}

void RemoteDialog::OnChatMessage( ChatPanelMsgEvt& event )
{
  handler->Send( "say:" + event.message );
}

RemotePosition* RemoteDialog::SetPosition( wxString& name, const wxString& key )
{
  RemPosMap::iterator it = positions.find( key );
  // Ignore possibly corrupted server
  if( it == positions.end() ||
      ( ! ValidName( name ) && name.Len() > 0 ) )
    return NULL;
  RemotePosition* pos = it->second;
  if( name.Len() ) {
    pos->button->SetLabel( name );
    pos->button->Enable( false );
  }
  else {
    pos->button->SetLabel( freestr );
    pos->button->Enable( true );
  }
  return pos;
}
