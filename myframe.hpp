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
