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

#ifndef _CHATPANEL_HPP_
#define _CHATPANEL_HPP_ 1

// Forward declarations
class ChatPanel;

#include <wx/string.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include "myevents.hpp"

#define EVT_CHAT_PANEL_MESSAGE(func) DECLARE_EVENT_TABLE_ENTRY( CHAT_PANEL_MESSAGE_TYPE, wxID_ANY, wxID_ANY, ( wxObjectEventFunction ) & func, (wxObject *) NULL ),

class ChatPanelMsgEvt: public wxEvent
{
public:
  wxString message;
  ChatPanelMsgEvt( wxString msg );
  wxEvent *Clone(void) const { return new ChatPanelMsgEvt( *this ); }
};

class ChatControl: public wxTextCtrl
{
public:
  ChatControl(wxWindow* parent, wxWindowID id, const wxString& value = "", const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxTextCtrlNameStr):
    wxTextCtrl( parent, id, value, pos, size, style, validator, name) {}
  virtual void SetFocusFromKbd() {}
};

// Panel with chat controls
class ChatPanel: public wxPanel
{
public:
  ChatPanel( wxWindow* parent );
  void Write( wxString message );
  void SayInChat( wxString& who, wxString& what );
  void OnSend( wxCommandEvent& event );
private:
  bool has_text;
  ChatControl* chat_text;
  wxTextCtrl* send_text;
  DECLARE_EVENT_TABLE();
};

#endif // _CHATPANEL_HPP_
