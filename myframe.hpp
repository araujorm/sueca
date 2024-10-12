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

#ifndef _MYFRAME_HPP_
#define _MYFRAME_HPP_ 1

// Forward declarations
class MyFrame;

#include <wx/frame.h>
#include <wx/statusbr.h>
#include <wx/menu.h>
#include "mycanvas.hpp"

enum { ID_GAME_PREFERENCES, ID_HOST_GAME, ID_CONNECT, ID_VIEW_SCORES, ID_VIEW_TRUMPH };

// Main window
class MyFrame: public wxFrame
{
public:
  // Frame properties
  bool viewscores;
  wxPoint score_pos;
  bool viewtrumph;
  wxPoint trumph_pos;
  wxMenu* gameMenu;
  wxMenu* viewMenu;
  MyCanvas* canvas;
  wxBoxSizer* main_sizer;
  MyFrame();
  ~MyFrame();
  void OnNew( wxCommandEvent& );
  void OnHostGame( wxCommandEvent& );
  void OnConnectToGame( wxCommandEvent& );
  void OnEndGame( wxCommandEvent& );
  void OnGamePrefs( wxCommandEvent& );
  void OnExit( wxCommandEvent& ) { Close(); }
  void OnViewScores( wxCommandEvent& );
  void OnViewTrumph( wxCommandEvent& );
  void Help( wxCommandEvent& );
  void About( wxCommandEvent& );
  void OnClose( wxCloseEvent& );
private:
  bool ProceedWithNewGame();
  DECLARE_EVENT_TABLE();
};

#endif // _MYFRAME_HPP_
