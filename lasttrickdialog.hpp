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

#ifndef _LASTTRICKDIALOG_HPP_
#define _LASTTRICKDIALOG_HPP_ 1

// Forward declarations
class TrickPanel;
class LastTrickDialog;

#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include "game.hpp"

// Panel that draws the 4 cards in a cross pattern
class TrickPanel: public wxPanel
{
public:
	TrickPanel( wxWindow* parent );
	void SetCard( int pos, Card* card, const wxString& name, bool winner );
	void Clear();
	void RecalcSize();
	void OnPaint( wxPaintEvent& event );
	int GetCX() const { return GetClientSize().GetWidth() / 2; }
	int GetCY() const { return GetClientSize().GetHeight() / 2; }
private:
	Card* m_cards[4];
	wxString m_names[4];
	bool m_winner[4];
	DECLARE_EVENT_TABLE();
};

// Dialog that displays the last completed trick
class LastTrickDialog: public wxDialog
{
public:
	LastTrickDialog( wxWindow* parent, wxPoint& pos );
	void UpdateTrick( PlayerIterator* players, const CardList& played,
	                  Player* winner );
	void RefitDialog();
	void OnButton( wxCommandEvent& event ) { Close(); }
	void OnClose( wxCloseEvent& event );
private:
	TrickPanel* m_panel;
	wxStaticText* m_winner_text;
	wxBoxSizer* m_top_sizer;
	DECLARE_EVENT_TABLE();
};

#endif // _LASTTRICKDIALOG_HPP_
