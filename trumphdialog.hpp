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

