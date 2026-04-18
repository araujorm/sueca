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

#ifndef _PREFSDIALOG_HPP_
#define _PREFSDIALOG_HPP_ 1

// Forward declarations
class PrefsDialog;

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include "definitions.hpp"

// Dialog with game options
class PrefsDialog: public wxDialog
{
public:
	PrefsDialog( wxWindow* parent );
private:
	wxTextCtrl* name_entry;
	wxSlider* delay_entry;
	wxCheckBox* bot_level_checks[BOT_LEVEL_COUNT];
	wxChoice* card_back_entry;
	void OnOk( wxCommandEvent& event );
	void OnBotLevelCheck( wxCommandEvent& event );
	void UpdateBotLevelEnableState();
	void Done( wxCommandEvent& event );
	DECLARE_EVENT_TABLE();
};

#endif // _PREFSDIALOG_HPP_

