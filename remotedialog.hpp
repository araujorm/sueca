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

#ifndef _REMOTEDIALOG_HPP_
#define _REMOTEDIALOG_HPP_ 1

// Forward declarations
class RemoteDialog;

#include <wx/list.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/hashmap.h>
#include <wx/radiobut.h>
#include "chatpanel.hpp"
#include "remotehandler.hpp"

enum { ID_RADIOBUTTON_P1, ID_RADIOBUTTON_P2, ID_RADIOBUTTON_P3, ID_RADIOBUTTON_P4 };

class RemotePosition
{
public:
  wxString name;
  wxRadioButton* button;
  RemotePosition( wxString initname, wxRadioButton* initbutton ):
    name( initname ), button( initbutton ) {}
};

WX_DECLARE_STRING_HASH_MAP( RemotePosition*, RemPosMap );

// Remote connection dialog
class RemoteDialog: public wxDialog
{
public:
  RemoteDialog( wxWindow* parent );
  void ReLayout();
protected:
  friend class RemoteHandler;
  wxRadioButton* invisible;
  RemotePosition* posarray[ID_RADIOBUTTON_P4+1];
  RemPosMap positions;
  ChatPanel* chat;
  wxWindowList disconnectedlist;
  wxWindowList connectedlist;
  void ConnectionEndWarn( wxString message );
  void NoConnectionState();
  RemotePosition* SetPosition( wxString& name, const wxString& key );
private:
  RemoteHandler* handler;
  wxBoxSizer* main_sizer;
  wxTextCtrl* ip_entry;
  wxTextCtrl* port_entry;
  wxButton* connect_button;
  void OnSelectPosition( wxCommandEvent& event );
  void OnConnect( wxCommandEvent& event );
  void OnCancel( wxCommandEvent& event ) { Done(); }
  void OnClose( wxCloseEvent& event ) { Done(); }
  void Done( bool cancel = true );
  void OnChatMessage( ChatPanelMsgEvt& event );
  DECLARE_EVENT_TABLE();
};

#endif // _REMOTEDIALOG_HPP_
