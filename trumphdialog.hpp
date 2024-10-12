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

#ifndef _TRUMPHDIALOG_HPP_
#define _TRUMPHDIALOG_HPP_ 1

// Forward declarations
class TrumphDialog;

#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include "game.hpp"

// Dialog that displays the current trumph
class TrumphDialog: public wxDialog
{
public:
  wxStaticBitmap* bmp;
  wxStaticText* text;
  TrumphDialog( wxWindow* parent, wxPoint& pos );
  void UpdateTrumph( Card* trumph, Player* owner );
  void OnButton( wxCommandEvent& event ) { Close(); }
  void OnClose( wxCloseEvent& event );
private:
  wxBitmap empty_bmp;
  wxBoxSizer* top_sizer;
  DECLARE_EVENT_TABLE();
};

#endif // _TRUMPHDIALOG_HPP_

