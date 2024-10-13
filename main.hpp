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

#ifndef _MAIN_HPP_
#define _MAIN_HPP_ 1

// Forward declarations
class Sueca;

/*
// For compilers that support precompilation, includes <wx/wx.h>.
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif
*/

#include <wx/app.h>
#include "definitions.hpp"
#include "myframe.hpp"
#include "serverhandler.hpp"
#include "serverdialog.hpp"
#include "remotedialog.hpp"
#include "game.hpp"

class Sueca: public wxApp
{
public:
  // Some preferences go here
  bool use_bound_ip_address;
  wxString bound_ip_address;
  wxString connect_ip_address;
  unsigned int ip_port;

  ServerDialog* servdlg;
  RemoteDialog* rmtdlg;
  ServerHandler* servhandler;

  virtual bool OnInit();
  virtual int OnExit();
  void OnEndSession( wxCloseEvent& event ) { PrepareExit(); }
  void PrepareExit();
  MyFrame* GetFrame() const { return m_frame; }
  void NewGame( Game* the_game, LocalPlayer* lp );
  void EndGame();
  wxString& GetLocalPlayerName() const { return (wxString&)playername; }
  LocalPlayer* GetLocalPlayer();
  void SetLocalPlayerName( const wxString& newname );
  unsigned int GetUpdateDelay() const { return update_delay; }
  void SetUpdateDelay( int new_delay ) { update_delay = new_delay; }
  Game* GetGame() const { return m_game; }
  Player* GetBotPlayer( GamePos* gamepos );
  void OnFinishRemoteHandler( FinishRemoteHandlerEvt& event );
private:
  // Preferences not directly accessible
  wxString playername;
  unsigned int update_delay;

  Game* m_game;
  MyFrame* m_frame;
  DECLARE_EVENT_TABLE();
};

DECLARE_APP(Sueca);

#endif // _MAIN_HPP_
