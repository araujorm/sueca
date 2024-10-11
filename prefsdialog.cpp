#include "prefsdialog.hpp"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include "main.hpp"
#include "netcommon.hpp"

// Preferences dialog implementation
BEGIN_EVENT_TABLE( PrefsDialog, wxDialog )
  EVT_BUTTON( wxID_OK, PrefsDialog::OnOk )
  EVT_BUTTON( wxID_CANCEL, PrefsDialog::Done )
END_EVENT_TABLE();

PrefsDialog::PrefsDialog( wxWindow* parent ):
  wxDialog( parent, wxID_ANY, wxString( "Game Preferences" ) )
{
  // Name entry
  wxBoxSizer* name_sizer = new wxBoxSizer( wxHORIZONTAL );  
  name_sizer->Add( new wxStaticText( this, wxID_ANY, "Player name" ), 0, wxRIGHT | wxALIGN_CENTER, 5 );
  name_entry = new wxTextCtrl( this, wxID_ANY, wxGetApp().GetLocalPlayerName() );
  name_entry->SetMaxLength( PLAYER_NAME_MAX );
  name_sizer->Add( name_entry, 0, wxALIGN_CENTER );

  // Card speed option
  wxBoxSizer* delay_sizer = new wxBoxSizer( wxHORIZONTAL );
  delay_sizer->Add( new wxStaticText( this, wxID_ANY, "Card movement speed" ), 0, wxRIGHT | wxALIGN_CENTER, 5 );
  delay_entry = new wxSlider( this, wxID_ANY, wxGetApp().GetUpdateDelay(), 1, 10, wxDefaultPosition, wxSize(100, wxID_ANY), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_AUTOTICKS );
  delay_sizer->Add( delay_entry, 0, wxALIGN_CENTER );

  // Buttons
  wxBoxSizer* button_sizer = new wxBoxSizer( wxHORIZONTAL );
  wxButton* ok_button = new wxButton( this, wxID_OK, "OK" );
  button_sizer->Add( ok_button );
  button_sizer->Add( 10, 1 );
  button_sizer->Add( new wxButton( this, wxID_CANCEL, "Cancel" ) );

  // Put all this on top sizer
  wxBoxSizer* top_sizer = new wxBoxSizer( wxVERTICAL );
  top_sizer->Add( name_sizer, 0, wxBOTTOM, 10 );
  top_sizer->Add( delay_sizer, 0, wxBOTTOM, 10 );
  top_sizer->Add( button_sizer, 0, wxTOP | wxALIGN_CENTER_HORIZONTAL, 5 );

  // Invisible border
  wxBoxSizer* border_sizer = new wxBoxSizer( wxVERTICAL );
  border_sizer->Add( top_sizer, 0, wxALL, 15 );
  SetSizer( border_sizer );
  border_sizer->Fit(this);

  ok_button->SetDefault();
  CentreOnParent();
}

void PrefsDialog::OnOk( wxCommandEvent& event )
{
  Sueca& app = wxGetApp();

  wxString newname = name_entry->GetValue();
  if( ! ValidName( newname ) ) {
    wxMessageBox( "Invalid name.", "Error", wxOK | wxICON_ERROR );
    return;
  }

  app.SetLocalPlayerName( newname );
  app.SetUpdateDelay( delay_entry->GetValue() );
  Done( event );
}

void PrefsDialog::Done( wxCommandEvent& event )
{
  if( IsModal() )
    EndModal(0);
  else
    Destroy();
}
