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
#include "expertplayer.hpp"
#include <wx/config.h>
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

	// Get stored preferences
	wxConfig* config = new wxConfig( SUECA_NAME );
	// Default preferences go here
	config->Read( "Use bound IP address", &use_bound_ip_address, false );
	config->Read( "Bound IP address", &bound_ip_address, wxEmptyString );
	config->Read( "IP port", (int*)&ip_port, 45678 );
	config->Read( "Connect IP address", &connect_ip_address, wxEmptyString );
	config->Read( "Player name", &playername, wxGetUserId() );
	config->Read( "Update delay", (int*)&update_delay, 8 );
	int stored_bot_level;
	config->Read( "Bot level", &stored_bot_level, (int)BOT_SMART );
	bot_level = (botlevel_t)stored_bot_level;
	int stored_card_back;
	config->Read( "Card back", &stored_card_back, (int)CARDBACK_BLUE );
	card_back = (cardback_t)stored_card_back;
	delete config;

	servdlg = NULL;
	rmtdlg = NULL;
	servhandler = new ServerHandler();

	m_game = NULL;
	m_frame = new MyFrame();
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
	config->Write( "Bot level", (int)bot_level );
	config->Write( "Card back", (int)card_back );
	delete config;
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
	botlevel_t level = bot_level;
	if( level == BOT_RANDOM )
		level = (botlevel_t)( (int)( 3.0 * rand() / ( RAND_MAX + 1.0 ) ) );
	switch( level ) {
	case BOT_DUMB:
		return new DumbPlayer( gamepos );
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
