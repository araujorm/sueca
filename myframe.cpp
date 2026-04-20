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

#include "myframe.hpp"
#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/intl.h>
#include "prefsdialog.hpp"
#include "cards.hpp"
#include "main.hpp"

// Derive a status bar for handling any mouse events on it
class MyStatusBar: public wxStatusBar
{
public:
	MyStatusBar( wxWindow* parent, wxWindowID id );
	void DClick( wxMouseEvent& event );
private:
	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE( MyStatusBar, wxStatusBar )
	EVT_LEFT_DCLICK( MyStatusBar::DClick )
END_EVENT_TABLE();

MyStatusBar::MyStatusBar( wxWindow* parent, wxWindowID id ):
	wxStatusBar( parent, id )
{
	SetFieldsCount( 2 );
}

void MyStatusBar::DClick( wxMouseEvent& event )
{
	wxRect field_area;
	GetFieldRect( 1, field_area );
	Game* game = wxGetApp().GetGame();
	if( game && field_area.Contains( event.GetPosition() ) ) {
		wxGetApp().GetFrame()->viewMenu->Check( ID_VIEW_SCORES, true );
		game->DisplayResults();
	}
}

// Main window implementation
BEGIN_EVENT_TABLE( MyFrame, wxFrame )
	EVT_MENU( wxID_NEW, MyFrame::OnNew )
	EVT_MENU( ID_HOST_GAME, MyFrame::OnHostGame )
	EVT_MENU( ID_CONNECT, MyFrame::OnConnectToGame )
	EVT_MENU( wxID_CLOSE, MyFrame::OnEndGame )
	EVT_MENU( ID_GAME_PREFERENCES, MyFrame::OnGamePrefs )
	EVT_MENU( wxID_EXIT, MyFrame::OnExit )
	EVT_MENU( ID_VIEW_SCORES, MyFrame::OnViewScores )
	EVT_MENU( ID_VIEW_TRUMPH, MyFrame::OnViewTrumph )
	EVT_MENU( ID_VIEW_LASTTRICK, MyFrame::OnViewLastTrick )
	EVT_MENU( wxID_HELP_CONTENTS, MyFrame::Help )
	EVT_MENU( wxID_ABOUT, MyFrame::About )
	EVT_CLOSE( MyFrame::OnClose )
END_EVENT_TABLE();

// In windows the icon is included in a windows resource file
// On other platforms we load it from embedded PNG
#ifndef __WXMSW__
#include <wx/mstream.h>
#include "icons/icon32.h"
#endif

MyFrame::MyFrame():
	wxFrame( NULL, wxID_ANY, "Sueca", wxDefaultPosition, wxDefaultSize,
	         wxDEFAULT_FRAME_STYLE ),
	viewscores( false ), score_pos( wxDefaultPosition ),
	viewtrumph( false ), trumph_pos( wxDefaultPosition ),
	viewlasttrick( false ), lasttrick_pos( wxDefaultPosition )
{
#ifdef __WXMSW__
	SetIcon( wxICON( icon32 ) );
#else
	wxIcon appicon;
	appicon.CopyFromBitmap( BitmapFromPNG( icon32, icon32_len ) );
	SetIcon( appicon );
#endif

	gameMenu = new wxMenu();
	gameMenu->Append( wxID_NEW, _( "&New single player\tF2" ), _( "Start a new game against computer players" ) );
	gameMenu->Append( ID_HOST_GAME, _( "&Host network game" ), _( "Start a network game" ) );
	gameMenu->Append( ID_CONNECT, _( "&Connect to network game" ), _( "Connect to a remote game" ) );
	gameMenu->Append( wxID_CLOSE, _( "&End Game" ), _( "End current game" ) );
	gameMenu->Enable( wxID_CLOSE, false );
	gameMenu->AppendSeparator();
	gameMenu->Append( ID_GAME_PREFERENCES, _( "&Preferences" ), _( "Set game preferences" ) );
	gameMenu->AppendSeparator();
	gameMenu->Append( wxID_EXIT, _( "&Exit\tCtrl-Q" ), _( "Exit program" ) );

	viewMenu = new wxMenu();
	viewMenu->AppendCheckItem( ID_VIEW_SCORES, _( "&Scores\tCtrl-S" ), _( "Show current game scores" ) );
	viewMenu->Enable( ID_VIEW_SCORES, false );
	viewMenu->AppendCheckItem( ID_VIEW_TRUMPH, _( "&Trumph\tCtrl-T" ), _( "Show current trumph card" ) );
	viewMenu->Enable( ID_VIEW_TRUMPH, false );
	viewMenu->AppendCheckItem( ID_VIEW_LASTTRICK, _( "&Last Trick\tCtrl-L" ), _( "Show last completed trick" ) );
	viewMenu->Enable( ID_VIEW_LASTTRICK, false );

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append( wxID_HELP_CONTENTS, _( "&Contents\tF1" ), _( "Help contents" ) );
	helpMenu->Append( wxID_ABOUT, _( "&About" ), _( "About this game" ) );

	wxMenuBar* mymenubar = new wxMenuBar();
	mymenubar->Append( gameMenu, _( "&Game" ) );
	mymenubar->Append( viewMenu, _( "&View" ) );
	mymenubar->Append( helpMenu, _( "&Help" ) );
	SetMenuBar( mymenubar );

	MyStatusBar* status = new MyStatusBar( this, wxID_ANY );
	SetStatusBar( status );
	status->SetStatusText( _( "Welcome!" ) );
	UpdateBotLevelStatus();

	main_sizer = new wxBoxSizer( wxVERTICAL );
	canvas = new MyCanvas( this, wxID_ANY );
	main_sizer->Add( canvas, 1, wxEXPAND );
	SetSizer( main_sizer );
	main_sizer->SetSizeHints( this );

	CentreOnScreen();
}

MyFrame::~MyFrame()
{
	if( wxGetApp().GetGame() )
		wxGetApp().EndGame();
}

void MyFrame::OnNew( wxCommandEvent& event )
{
	if( ProceedWithNewGame() ) {
		LocalPlayer* p1 = new LocalPlayer( wxGetApp().GetLocalPlayerName(), new GamePosP1() );
		//Player* p1 = wxGetApp().GetBotPlayer( new GamePosP1() );
		Player* p2 = wxGetApp().GetBotPlayer( new GamePosP2() );
		Player* p3 = wxGetApp().GetBotPlayer( new GamePosP3() );
		Player* p4 = wxGetApp().GetBotPlayer( new GamePosP4() );
		wxGetApp().NewGame( new Game( p1, p2, p3, p4, canvas ), p1 );
	}
}

void MyFrame::OnHostGame( wxCommandEvent& event )
{
	if( ProceedWithNewGame() ) {
		wxGetApp().EndGame();
		ServerDialog( this ).ShowModal();
	}
}

void MyFrame::OnConnectToGame( wxCommandEvent& event )
{
	if( ProceedWithNewGame() ) {
		wxGetApp().EndGame();
		RemoteDialog( this ).ShowModal();
	}
}

void MyFrame::OnEndGame( wxCommandEvent& event )
{
	if( wxGetApp().GetGame() ) {
		if(wxMessageBox( _( "Are you sure?" ),
		                _( "End game" ), wxYES_NO, this ) == wxNO )
			return;
		wxGetApp().EndGame();
	}
}

void MyFrame::OnGamePrefs( wxCommandEvent& event )
{
	PrefsDialog( this ).ShowModal();
	UpdateBotLevelStatus();
}

void MyFrame::OnViewScores( wxCommandEvent& event )
{
	Game *game = wxGetApp().GetGame();
	viewscores = event.IsChecked();
	if( game ) {
		if( viewscores )
			game->DisplayResults();
		else
			game->score->Show( false );
	}
}

void MyFrame::OnViewTrumph( wxCommandEvent& event )
{
	Game *game = wxGetApp().GetGame();
	viewtrumph = event.IsChecked();
	if( game )
		game->trumphdlg->Show( viewtrumph );
}

void MyFrame::OnViewLastTrick( wxCommandEvent& event )
{
	Game *game = wxGetApp().GetGame();
	viewlasttrick = event.IsChecked();
	if( game )
		game->lasttrickdlg->Show( viewlasttrick );
}

void MyFrame::Help( wxCommandEvent& event )
{
	wxMessageDialog msg( this, _( "Not implemented yet." ) );
	msg.ShowModal();
}

void MyFrame::About( wxCommandEvent& event )
{
	wxMessageDialog md( this,
	  wxString( VERSION_STRING "\n" ) +
	  _( "(C) 2003-2026 Rodrigo Araujo\n"
	     "Card graphics by jfitz technologies - http://www.jfitz.com/\n"
	     "\n"
	     "This program is free software; you can redistribute it and/or modify\n"
	     "it under the terms of the GNU General Public License as published by\n"
	     "the Free Software Foundation; either version 2 of the License, or\n"
	     "(at your option) any later version.\n"
	     "\n"
	     "This program is distributed in the hope that it will be useful,\n"
	     "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	     "GNU General Public License for more details." ),
	  _( "About this game" ), wxOK );
	md.ShowModal();
}

void MyFrame::OnClose( wxCloseEvent& event )
{
	Game* game = wxGetApp().GetGame();
	// Confirm before exiting mid-game. Skip the prompt when the close
	// cannot be vetoed (system shutdown, logout) so we don't block the
	// session from ending.
	if( game && event.CanVeto() &&
	    wxMessageBox( _( "A game is in progress. Do you really want to exit?" ),
	                  _( "Game in progress" ),
	                  wxYES_NO | wxICON_QUESTION, this ) == wxNO ) {
		event.Veto();
		return;
	}
	// Update dialog positions from live dialogs if game is running
	if( game ) {
		score_pos = game->score->GetPosition();
		trumph_pos = game->trumphdlg->GetPosition();
		lasttrick_pos = game->lasttrickdlg->GetPosition();
	}
	// Save window positions before closing
	wxConfig* config = new wxConfig( SUECA_NAME );
	wxPoint fpos = GetPosition();
	wxSize fsz = GetSize();
	config->Write( "Frame x", fpos.x );
	config->Write( "Frame y", fpos.y );
	config->Write( "Frame w", fsz.GetWidth() );
	config->Write( "Frame h", fsz.GetHeight() );
	config->Write( "Score x", score_pos.x );
	config->Write( "Score y", score_pos.y );
	config->Write( "Trumph x", trumph_pos.x );
	config->Write( "Trumph y", trumph_pos.y );
	config->Write( "Last trick x", lasttrick_pos.x );
	config->Write( "Last trick y", lasttrick_pos.y );
	config->Write( "View scores", viewscores );
	config->Write( "View trumph", viewtrumph );
	config->Write( "View last trick", viewlasttrick );
	delete config;
	Destroy();
}

bool MyFrame::ProceedWithNewGame()
{
	if( wxGetApp().GetGame() &&
	    wxMessageBox( _( "End current game and start a new one?" ),
	                 _( "Game in progress" ), wxYES_NO, this ) == wxNO )
		return false;
	return true;
}

// Tagged for extraction so translators can localise the bot-difficulty
// labels that show up on the status bar (same strings the preferences
// dialog uses for the checkboxes).
static const char* bot_level_names[BOT_LEVEL_COUNT] = {
	wxTRANSLATE( "Dumb" ),
	wxTRANSLATE( "Methodic" ),
	wxTRANSLATE( "Smart" ),
	wxTRANSLATE( "Expert" )
};

void MyFrame::UpdateBotLevelStatus()
{
	if( wxGetApp().GetGame() )
		return;
	int mask = wxGetApp().GetBotLevels();
	int count = 0, only = -1;
	for( int l = 0; l < BOT_LEVEL_COUNT; l++ )
		if( mask & BOT_FLAG( l ) ) {
			count++;
			only = l;
		}
	wxString name = ( count == 1 ) ?
	  wxGetTranslation( bot_level_names[only] ) : _( "Mixed" );
	SetStatusText( wxString::Format( _( "Bot: %s" ), name ), 1 );
}
