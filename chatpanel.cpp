#include "chatpanel.hpp"
#include "main.hpp"

DEFINE_EVENT_TYPE( CHAT_PANEL_MESSAGE_TYPE )

ChatPanelMsgEvt::ChatPanelMsgEvt( wxString msg ):
  wxEvent( 0, CHAT_PANEL_MESSAGE_TYPE ), message( msg )
{
  m_propagationLevel = 1;
}

// Server creation dialog implementation
enum { ID_SENDTEXT, ID_SEND };

BEGIN_EVENT_TABLE( ChatPanel, wxPanel )
  EVT_TEXT_ENTER( ID_SENDTEXT, ChatPanel::OnSend )
  EVT_BUTTON( ID_SEND, ChatPanel::OnSend )
END_EVENT_TABLE();

ChatPanel::ChatPanel( wxWindow* parent ):
  wxPanel( parent ), has_text( false )
{
  // Chat window
  chat_text = new ChatControl( this, wxID_ANY, "", wxDefaultPosition, wxSize( -1, 100 ), wxTE_READONLY | wxTE_MULTILINE );
  send_text = new wxTextCtrl( this, ID_SENDTEXT, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
  wxButton* send_button = new wxButton( this, ID_SEND, "S&end" );
  wxBoxSizer* send_sizer = new wxBoxSizer( wxHORIZONTAL );
  send_sizer->Add( send_text, 1 );
  send_sizer->Add( send_button );
  wxBoxSizer* main_sizer = new wxBoxSizer( wxVERTICAL );
  main_sizer->Add( chat_text, 0, wxEXPAND );
  main_sizer->Add( send_sizer, 0, wxEXPAND );
  SetSizerAndFit( main_sizer );
}

void ChatPanel::Write( wxString message )
{
  if( has_text )
    chat_text->AppendText( "\n" );
  else
    has_text = true;
  chat_text->AppendText( message );
}

void ChatPanel::SayInChat( wxString& who, wxString& what )
{
  Write( wxString::Format( "%s: %s", who.c_str(), what.c_str() ) );
}

void ChatPanel::OnSend( wxCommandEvent& event )
{
  wxString text = send_text->GetValue();
  send_text->Clear();
  text.Trim( true );
  text.Trim( false );
  ChatPanelMsgEvt chatevent( text );
  ProcessEvent( chatevent );
}
