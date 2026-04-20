/*
sueca - An implementation of the Portuguese game "Sueca" in C++ and wxWidgets
Copyright (C) 2003-2026 Rodrigo Araujo

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

#include "prefsdialog.hpp"
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <wx/intl.h>
#include "main.hpp"
#include "netcommon.hpp"
#include "lang.hpp"

// Preferences dialog implementation
BEGIN_EVENT_TABLE( PrefsDialog, wxDialog )
	EVT_BUTTON( wxID_OK, PrefsDialog::OnOk )
	EVT_BUTTON( wxID_CANCEL, PrefsDialog::Done )
END_EVENT_TABLE();

PrefsDialog::PrefsDialog( wxWindow* parent ):
	wxDialog( parent, wxID_ANY, _( "Game Preferences" ) )
{
	// Name entry
	wxBoxSizer* name_sizer = new wxBoxSizer( wxHORIZONTAL );
	name_sizer->Add( new wxStaticText( this, wxID_ANY, _( "Player name" ) ), 0, wxRIGHT | wxALIGN_CENTER, 5 );
	name_entry = new wxTextCtrl( this, wxID_ANY, wxGetApp().GetLocalPlayerName() );
	name_entry->SetMaxLength( PLAYER_NAME_MAX );
	name_sizer->Add( name_entry, 0, wxALIGN_CENTER );

	// Card speed option
	wxBoxSizer* delay_sizer = new wxBoxSizer( wxHORIZONTAL );
	delay_sizer->Add( new wxStaticText( this, wxID_ANY, _( "Card movement speed" ) ), 0, wxRIGHT | wxALIGN_CENTER, 5 );
	delay_entry = new wxSlider( this, wxID_ANY, wxGetApp().GetUpdateDelay(), 1, 10, wxDefaultPosition, wxSize(100, wxID_ANY), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_AUTOTICKS );
	delay_sizer->Add( delay_entry, 0, wxALIGN_CENTER );

	// Bot difficulty options: one or more levels can be enabled. When more
	// than one is checked, each bot picks a level at random among them.
	int current_mask = wxGetApp().GetBotLevels();
	wxBoxSizer* bot_sizer = new wxBoxSizer( wxHORIZONTAL );
	bot_sizer->Add( new wxStaticText( this, wxID_ANY, _( "Bot difficulty" ) ), 0, wxRIGHT | wxALIGN_CENTER, 5 );
	// Tagged with wxTRANSLATE so xgettext picks them up; wxGetTranslation
	// converts them at runtime. Must match the level names shown on the
	// status bar.
	static const char* bot_labels[BOT_LEVEL_COUNT] = {
	  wxTRANSLATE( "Dumb" ),
	  wxTRANSLATE( "Methodic" ),
	  wxTRANSLATE( "Smart" ),
	  wxTRANSLATE( "Expert" )
	};
	static const char* bot_tooltips[BOT_LEVEL_COUNT] = {
	  wxTRANSLATE( "Plays any valid card, no strategy.\n"
	               "At least one level must stay selected." ),
	  wxTRANSLATE( "Uses Monte Carlo simulation with a simple rule-based\n"
	               "heuristic for each simulated play.\n"
	               "At least one level must stay selected." ),
	  wxTRANSLATE( "Uses rule-based strategy with card tracking and\n"
	               "observational memory of other players.\n"
	               "At least one level must stay selected." ),
	  wxTRANSLATE( "Blends Monte Carlo simulation with the rule-based\n"
	               "heuristics of the Smart level for the strongest play.\n"
	               "At least one level must stay selected." )
	};
	for( int l = 0; l < BOT_LEVEL_COUNT; l++ ) {
		bot_level_checks[l] = new wxCheckBox( this, wxID_ANY,
		  wxGetTranslation( bot_labels[l] ) );
		bot_level_checks[l]->SetValue( ( current_mask & BOT_FLAG( l ) ) != 0 );
		bot_level_checks[l]->SetToolTip( wxGetTranslation( bot_tooltips[l] ) );
		bot_level_checks[l]->Bind( wxEVT_CHECKBOX,
		                           &PrefsDialog::OnBotLevelCheck, this );
		bot_sizer->Add( bot_level_checks[l], 0, wxALIGN_CENTER | wxRIGHT, 5 );
	}
	UpdateBotLevelEnableState();

	// Card back option
	wxBoxSizer* back_sizer = new wxBoxSizer( wxHORIZONTAL );
	back_sizer->Add( new wxStaticText( this, wxID_ANY, _( "Card back" ) ), 0, wxRIGHT | wxALIGN_CENTER, 5 );
	wxString back_choices[] = { _( "Blue" ), _( "Red" ) };
	card_back_entry = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, back_choices );
	card_back_entry->SetSelection( (int)wxGetApp().GetCardBack() );
	back_sizer->Add( card_back_entry, 0, wxALIGN_CENTER );

	// Language option: "Same as system" first (translated), then every
	// supported language in its own native name. Changing this requires
	// restarting the application for catalogue reload to take effect.
	wxBoxSizer* lang_sizer = new wxBoxSizer( wxHORIZONTAL );
	lang_sizer->Add( new wxStaticText( this, wxID_ANY, _( "Language" ) ),
	                 0, wxRIGHT | wxALIGN_CENTER, 5 );
	language_entry = new wxChoice( this, wxID_ANY );
	int current_lang = wxGetApp().GetLanguage();
	int selected_lang_idx = 0;
	for( int i = 0; kSuecaLanguages[i].native_name; i++ ) {
		wxString label;
		if( kSuecaLanguages[i].code == wxLANGUAGE_DEFAULT )
			label = _( "Same as system" );
		else
			label = wxString::FromUTF8( kSuecaLanguages[i].native_name );
		language_entry->Append( label );
		if( kSuecaLanguages[i].code == current_lang )
			selected_lang_idx = i;
	}
	language_entry->SetSelection( selected_lang_idx );
	language_entry->SetToolTip( _( "Takes effect on next application start." ) );
	lang_sizer->Add( language_entry, 0, wxALIGN_CENTER );

	// Buttons
	wxBoxSizer* button_sizer = new wxBoxSizer( wxHORIZONTAL );
	wxButton* ok_button = new wxButton( this, wxID_OK, _( "OK" ) );
	button_sizer->Add( ok_button );
	button_sizer->Add( 10, 1 );
	button_sizer->Add( new wxButton( this, wxID_CANCEL, _( "Cancel" ) ) );

	// Put all this on top sizer
	wxBoxSizer* top_sizer = new wxBoxSizer( wxVERTICAL );
	top_sizer->Add( name_sizer, 0, wxBOTTOM, 10 );
	top_sizer->Add( delay_sizer, 0, wxBOTTOM, 10 );
	top_sizer->Add( bot_sizer, 0, wxBOTTOM, 10 );
	top_sizer->Add( back_sizer, 0, wxBOTTOM, 10 );
	top_sizer->Add( lang_sizer, 0, wxBOTTOM, 10 );
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
		wxMessageBox( _( "Invalid name." ), _( "Error" ), wxOK | wxICON_ERROR );
		return;
	}

	app.SetLocalPlayerName( newname );
	app.SetUpdateDelay( delay_entry->GetValue() );
	int newmask = 0;
	for( int l = 0; l < BOT_LEVEL_COUNT; l++ )
		if( bot_level_checks[l]->GetValue() )
			newmask |= BOT_FLAG( l );
	// Failsafe: guarantee at least one level stays enabled, even if the UI
	// constraint was bypassed.
	if( newmask == 0 )
		newmask = BOT_FLAG( BOT_SMART );
	if( newmask != app.GetBotLevels() ) {
		app.SetBotLevels( newmask );
		if( app.GetGame() )
			wxMessageBox( _( "Bot difficulty change will take effect\n"
			                 "when you start a new game." ),
			  _( "Note" ), wxOK | wxICON_INFORMATION );
	}
	app.SetCardBack( (cardback_t)card_back_entry->GetSelection() );
	int lang_idx = language_entry->GetSelection();
	if( lang_idx >= 0 ) {
		int new_lang = kSuecaLanguages[lang_idx].code;
		if( new_lang != app.GetLanguage() ) {
			app.SetLanguage( new_lang );
			wxMessageBox( _( "The language change will take effect the "
			                 "next time Sueca is started." ),
			              _( "Language" ), wxOK | wxICON_INFORMATION );
		}
	}
	Done( event );
}

void PrefsDialog::OnBotLevelCheck( wxCommandEvent& WXUNUSED( event ) )
{
	UpdateBotLevelEnableState();
}

// If exactly one level is checked, disable that checkbox so the user can't
// leave the dialog with zero levels enabled. Re-enable all others once more
// than one is checked.
void PrefsDialog::UpdateBotLevelEnableState()
{
	int checked_count = 0, last_checked = -1;
	for( int l = 0; l < BOT_LEVEL_COUNT; l++ )
		if( bot_level_checks[l]->GetValue() ) {
			checked_count++;
			last_checked = l;
		}
	for( int l = 0; l < BOT_LEVEL_COUNT; l++ )
		bot_level_checks[l]->Enable(
		  ! ( checked_count == 1 && l == last_checked ) );
}

void PrefsDialog::Done( wxCommandEvent& event )
{
	if( IsModal() )
		EndModal(0);
	else
		Destroy();
}
