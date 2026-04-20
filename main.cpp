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

#include "main.hpp"
#include "player.hpp"
#include "smartplayer.hpp"
#include "methodicplayer.hpp"
#include "expertplayer.hpp"
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

IMPLEMENT_APP( Sueca );

BEGIN_EVENT_TABLE( Sueca, wxApp )
	EVT_END_SESSION( Sueca::OnEndSession )
	EVT_FINISH_REMOTE_HANDLER( Sueca::OnFinishRemoteHandler )
END_EVENT_TABLE();

bool Sueca::OnInit()
{
	// Random seed initialization
	srand( time( NULL ) );

	// Initialize image handlers for PNG support
	wxInitAllImageHandlers();

	// UI language setup. Read the user's choice (wxLANGUAGE_DEFAULT
	// means "same as system locale") and bring up wxLocale before the
	// main frame is built so menu labels, dialog texts and everything
	// else passes through the 'sueca' message catalogue.
	{
		wxConfig cfg( SUECA_NAME );
		cfg.Read( "Language", &language, (int)wxLANGUAGE_DEFAULT );
	}
	locale = new wxLocale();
	// wx emits a log warning when asked for a language it considers
	// unavailable on the system; swallow that so a missing locale data
	// file doesn't pop a dialog on every launch.
	{
		wxLogNull suppress_locale_warnings;
		// wxLocale::Init can't be called twice on the same object (it
		// asserts on m_initialized), so on failure we throw the partial
		// locale away and create a fresh one for the default fallback.
		if( ! locale->Init( language, wxLOCALE_DONT_LOAD_DEFAULT ) ) {
			delete locale;
			locale = new wxLocale();
			locale->Init( wxLANGUAGE_DEFAULT, wxLOCALE_DONT_LOAD_DEFAULT );
		}
	}
	// Look up 'sueca.mo' under ./mo/<lang>/LC_MESSAGES/ relative to the
	// binary first (portable install), falling back to wx's standard
	// paths (system-wide /usr/share/locale/...).
	{
		wxFileName exe( wxStandardPaths::Get().GetExecutablePath() );
		wxLocale::AddCatalogLookupPathPrefix(
		  exe.GetPath() + wxFileName::GetPathSeparator() + "mo" );
	}
	locale->AddCatalog( SUECA_NAME );

	// Get stored preferences
	wxConfig* config = new wxConfig( SUECA_NAME );
	// Default preferences go here
	config->Read( "Use bound IP address", &use_bound_ip_address, false );
	config->Read( "Bound IP address", &bound_ip_address, wxEmptyString );
	config->Read( "IP port", (int*)&ip_port, 45678 );
	config->Read( "Connect IP address", &connect_ip_address, wxEmptyString );
	config->Read( "Player name", &playername, wxGetUserId() );
	config->Read( "Update delay", (int*)&update_delay, 8 );
	config->Read( "Bot levels", &bot_levels, BOT_FLAGS_ALL );
	// Defensive: if the stored mask has no valid bits (corrupted config or
	// manual edit), fall back to all levels allowed.
	if( ! ( bot_levels & BOT_FLAGS_ALL ) )
		bot_levels = BOT_FLAGS_ALL;
	int stored_card_back;
	config->Read( "Card back", &stored_card_back, (int)CARDBACK_BLUE );
	card_back = (cardback_t)stored_card_back;

	// Window positions and sizes
	int frame_x, frame_y, frame_w, frame_h;
	config->Read( "Frame x", &frame_x, -1 );
	config->Read( "Frame y", &frame_y, -1 );
	config->Read( "Frame w", &frame_w, -1 );
	config->Read( "Frame h", &frame_h, -1 );
	int score_x, score_y, trumph_x, trumph_y, lasttrick_x, lasttrick_y;
	config->Read( "Score x", &score_x, -1 );
	config->Read( "Score y", &score_y, -1 );
	config->Read( "Trumph x", &trumph_x, -1 );
	config->Read( "Trumph y", &trumph_y, -1 );
	config->Read( "Last trick x", &lasttrick_x, -1 );
	config->Read( "Last trick y", &lasttrick_y, -1 );
	bool view_scores, view_trumph, view_lasttrick;
	config->Read( "View scores", &view_scores, false );
	config->Read( "View trumph", &view_trumph, false );
	config->Read( "View last trick", &view_lasttrick, false );
	delete config;

	servdlg = NULL;
	rmtdlg = NULL;
	servhandler = new ServerHandler();

	m_game = NULL;
	m_frame = new MyFrame();
	if( frame_x >= 0 && frame_y >= 0 )
		m_frame->SetPosition( wxPoint( frame_x, frame_y ) );
	if( frame_w > 0 && frame_h > 0 )
		m_frame->SetSize( frame_w, frame_h );
	if( score_x >= 0 ) m_frame->score_pos = wxPoint( score_x, score_y );
	if( trumph_x >= 0 ) m_frame->trumph_pos = wxPoint( trumph_x, trumph_y );
	if( lasttrick_x >= 0 ) m_frame->lasttrick_pos = wxPoint( lasttrick_x, lasttrick_y );
	m_frame->viewscores = view_scores;
	m_frame->viewtrumph = view_trumph;
	m_frame->viewlasttrick = view_lasttrick;
	m_frame->viewMenu->Check( ID_VIEW_SCORES, view_scores );
	m_frame->viewMenu->Check( ID_VIEW_TRUMPH, view_trumph );
	m_frame->viewMenu->Check( ID_VIEW_LASTTRICK, view_lasttrick );
	m_frame->Show( TRUE );
	SetTopWindow( m_frame );
	return TRUE;
}

int Sueca::OnExit()
{
	PrepareExit();
	return wxApp::OnExit();
}

void Sueca::PrepareExit()
{
	delete servhandler;
	// Store preferences
	wxConfig* config = new wxConfig( SUECA_NAME );
	config->Write( "Use bound IP address", use_bound_ip_address );
	config->Write( "Bound IP address", bound_ip_address );
	config->Write( "IP port", (int)ip_port );
	config->Write( "Connect IP address", connect_ip_address );
	config->Write( "Player name", playername );
	config->Write( "Update delay", (int)update_delay );
	config->Write( "Bot levels", bot_levels );
	config->Write( "Card back", (int)card_back );
	config->Write( "Language", language );
	delete config;
	delete locale;
	locale = NULL;
}

void Sueca::NewGame( Game* the_game, LocalPlayer* lp )
{
	m_game = the_game;
	m_frame->canvas->SetLocalPlayer( lp );
	m_frame->viewMenu->Enable( ID_VIEW_TRUMPH, true );
	m_frame->viewMenu->Enable( ID_VIEW_SCORES, true );
	m_frame->viewMenu->Enable( ID_VIEW_LASTTRICK, true );
	m_frame->gameMenu->Enable( wxID_CLOSE, true );
	m_game->NewRound();
}

void Sueca::EndGame()
{
	if( m_game ) {
		delete m_game;
		m_frame->viewMenu->Enable( ID_VIEW_TRUMPH, false );
		m_frame->viewMenu->Enable( ID_VIEW_SCORES, false );
		m_frame->viewMenu->Enable( ID_VIEW_LASTTRICK, false );
		m_frame->gameMenu->Enable( wxID_CLOSE, false );
		m_frame->canvas->ClearActivePlayer();
		m_frame->canvas->SetLocalPlayer( NULL );
		m_game = NULL;
		m_frame->SetStatusText("", 1);
		m_frame->UpdateBotLevelStatus();
		m_frame->Refresh();
	}
}

LocalPlayer* Sueca::GetLocalPlayer()
{
	return m_frame->canvas->GetLocalPlayer();
}

void Sueca::SetLocalPlayerName( const wxString& newname )
{
	playername = newname;
	// Update on current game
	LocalPlayer* lp;
	if( m_game && ( lp = GetLocalPlayer() ) ) {
		m_game->SetPlayerName( lp, newname );
	}
}

Player* Sueca::GetBotPlayer( GamePos* gamepos )
{
	// Collect currently enabled levels from the bitmask and pick one uniformly
	// at random. When a single level is enabled, that's the only option.
	botlevel_t enabled[BOT_LEVEL_COUNT];
	int n_enabled = 0;
	for( int l = 0; l < BOT_LEVEL_COUNT; l++ )
		if( bot_levels & BOT_FLAG( l ) )
			enabled[n_enabled++] = (botlevel_t)l;
	// Failsafe if no level is enabled (should not happen via the UI)
	if( n_enabled == 0 )
		enabled[n_enabled++] = BOT_SMART;
	botlevel_t level =
	  enabled[(int)( (double)n_enabled * rand() / ( RAND_MAX + 1.0 ) )];
	switch( level ) {
	case BOT_DUMB:
		return new DumbPlayer( gamepos );
	case BOT_METHODIC:
		return new MethodicPlayer( gamepos );
	case BOT_EXPERT:
		return new ExpertPlayer( gamepos );
	default:
		return new SmartPlayer( gamepos );
	}
}

void Sueca::SetCardBack( cardback_t back )
{
	card_back = back;
	if( m_game ) {
		const unsigned char* data;
		unsigned int len;
		GetCardBackData( back, &data, &len );
		m_game->GetDeck().SetFace( data, len );
		m_frame->canvas->Refresh();
	}
}

void Sueca::OnFinishRemoteHandler( FinishRemoteHandlerEvt& event )
{
	delete event.handler;
}
