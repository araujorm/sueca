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

#ifndef _SERVERDIALOG_HPP_
#define _SERVERDIALOG_HPP_ 1

// Forward declarations
class ServerDialog;

#include <wx/list.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/hashmap.h>
#include <wx/stattext.h>
#include "chatpanel.hpp"
#include "serverhandler.hpp"
#include "netserverplayer.hpp"

class Position
{
public:
  NetServerPlayer* player;
  wxStaticText* label;
  GamePos* gpos;
  Position( wxStaticText* initlabel, GamePos* initgpos ):
    player( NULL ), label( initlabel ), gpos( initgpos ) {}
};

WX_DECLARE_STRING_HASH_MAP( Position*, PosMap );

// Server creation dialog
class ServerDialog: public wxDialog
{
public:
  ServerDialog( wxWindow* parent );
  void ReLayout();
protected:
  friend class ServerHandler;
  PosMap positions;
  ChatPanel* chat;
  wxWindowList disconnectedlist;
  wxWindowList connectedlist;
  Position* GetPlayerPosition( const Player* player );
  void SetPosition( Position* pos, NetServerPlayer* pl );
private:
  bool ip_enabled;
  wxBoxSizer* main_sizer;
  wxTextCtrl* ip_entry;
  wxTextCtrl* port_entry;
  wxButton* ok_button;
  wxButton* listen_button;
  void OnIpEnabled( wxCommandEvent& event );
  void EndListen();
  void OnListen( wxCommandEvent& event );
  Player* GetPlayerForPos( wxString posname );
  void OnBegin( wxCommandEvent& event );
  void OnCancel( wxCommandEvent& event ) { Done(); }
  void OnClose( wxCloseEvent& event ) { Done(); }
  void Done( bool cancel = true );
  void OnChatMessage( ChatPanelMsgEvt& event );
  Position* GetNotMatchingPos( wxString& posstr, Player* player );
  DECLARE_EVENT_TABLE();
};

#endif // _SERVERDIALOG_HPP_
