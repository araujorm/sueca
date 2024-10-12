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

#ifndef _SCOREDIALOG_HPP_
#define _SCOREDIALOG_HPP_ 1

#ifndef MAX_DISPLAYED_ROUNDS
#define MAX_DISPLAYED_ROUNDS 10
#endif

// Forward declarations
class WindowList;
class ScoreDialog;

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/list.h>
#include "player.hpp"

// List of previous rounds display objects
WX_DECLARE_LIST( wxStaticText, StaticTextList );

// Dialog that displays the scores
class ScoreDialog: public wxDialog
{
public:
  ScoreDialog( wxWindow* parent, Team* nteam1, Team* nteam2, wxPoint& pos );
  void UpdateRoundResults( const wxString& s1 = "", const wxString& s2 = "",
			   bool force = false );
  bool Show( bool show );
  void SetEndRoundResults();
  void AddLine( wxString header, wxString s1, wxString s2,
                int fontw = wxNORMAL, wxColour color = *wxBLACK, int flags = 0,
                wxStaticText** stxt1 = NULL, wxStaticText** stxt2 = NULL,
                wxStaticText** stxth = NULL );
  void OnButton( wxCommandEvent& event ) { Close(); }
  void OnClose( wxCloseEvent& event );
  void RefreshNames();
private:
  unsigned int round;
  unsigned short displayed;
  wxBoxSizer* top_sizer;
  wxFlexGridSizer* gridsz;
  wxStaticText* team1text;
  wxStaticText* team2text;
  wxStaticText* team1won;
  wxStaticText* team2won;
  wxStaticText* team1points;
  wxStaticText* team2points;
  wxStaticText* roundlabel;
  bool new_round;
  StaticTextList oldobjs;
  Team* team1;
  Team* team2;
  DECLARE_EVENT_TABLE();
};

#endif // _SCOREDIALOG_HPP_
