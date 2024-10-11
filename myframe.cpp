#include "myframe.hpp"
#include <wx/msgdlg.h>
#include "prefsdialog.hpp"
#include "main.hpp"

// Derive a status bar for handling any mouse events on it
class MyStatusBar: public wxStatusBar
{
public:
  MyStatusBar( wxWindow* parent, wxWindowID id );
  void DClick( wxMouseEvent& event );
private:
  DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE( MyStatusBar, wxStatusBar )
  EVT_LEFT_DCLICK( MyStatusBar::DClick )
END_EVENT_TABLE();

MyStatusBar::MyStatusBar( wxWindow* parent, wxWindowID id ):
  wxStatusBar( parent, id )
{
  SetFieldsCount( 2 );
}

void MyStatusBar::DClick( wxMouseEvent& event )
{
  wxRect field_area;
  GetFieldRect( 1, field_area );
  Game* game = wxGetApp().GetGame();
  if( game && field_area.Contains( event.GetPosition() ) ) {
    wxGetApp().GetFrame()->viewMenu->Check( ID_VIEW_SCORES, true );
    game->DisplayResults();
  }
}

// Main window implementation
BEGIN_EVENT_TABLE( MyFrame, wxFrame )
  EVT_MENU( wxID_NEW, MyFrame::OnNew )
  EVT_MENU( ID_HOST_GAME, MyFrame::OnHostGame )
  EVT_MENU( ID_CONNECT, MyFrame::OnConnectToGame )
  EVT_MENU( wxID_CLOSE, MyFrame::OnEndGame )
  EVT_MENU( ID_GAME_PREFERENCES, MyFrame::OnGamePrefs )
  EVT_MENU( wxID_EXIT, MyFrame::OnExit )
  EVT_MENU( ID_VIEW_SCORES, MyFrame::OnViewScores )
  EVT_MENU( ID_VIEW_TRUMPH, MyFrame::OnViewTrumph )
  EVT_MENU( wxID_HELP_CONTENTS, MyFrame::Help )
  EVT_MENU( wxID_ABOUT, MyFrame::About )
  EVT_CLOSE( MyFrame::OnClose )
END_EVENT_TABLE();

// In windows the icon is included in a windows resource file
// On other platforms we use this xpm
#ifndef __WXMSW__
#include "icons/icon32.xpm"
#endif

MyFrame::MyFrame():
  wxFrame( NULL, -1, "Sueca", wxDefaultPosition, wxDefaultSize,
           wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX) ),
  viewscores( false ), score_pos( wxDefaultPosition ),
  viewtrumph( false ), trumph_pos( wxDefaultPosition )
{
  SetIcon( wxICON( icon32 ) );

  gameMenu = new wxMenu();
  gameMenu->Append( wxID_NEW, "&New single player\tF2", "Start a new game against computer players" );
  gameMenu->Append( ID_HOST_GAME, "&Host network game", "Start a network game" );
  gameMenu->Append( ID_CONNECT, "&Connect to network game", "Connect to a remote game" );
  gameMenu->Append( wxID_CLOSE, "&End Game", "End current game" );
  gameMenu->Enable( wxID_CLOSE, false );
  gameMenu->AppendSeparator();
  gameMenu->Append( ID_GAME_PREFERENCES, "&Preferences", "Set game preferences" );
  gameMenu->AppendSeparator();
  gameMenu->Append( wxID_EXIT, "&Exit\tCtrl-Q", "Exit program" );

  viewMenu = new wxMenu();
  viewMenu->AppendCheckItem( ID_VIEW_SCORES, "&Scores\tCtrl-S", "Show current game scores" );
  viewMenu->Enable( ID_VIEW_SCORES, false );
  viewMenu->AppendCheckItem( ID_VIEW_TRUMPH, "&Trumph\tCtrl-T", "Show current trumph card" );
  viewMenu->Enable( ID_VIEW_TRUMPH, false );

  wxMenu* helpMenu = new wxMenu();
  helpMenu->Append( wxID_HELP_CONTENTS, "&Contents\tF1", "Help contents" );
  helpMenu->Append( wxID_ABOUT, "&About", "About this game" );

  wxMenuBar* mymenubar = new wxMenuBar();
  mymenubar->Append( gameMenu, "&Game" );
  mymenubar->Append( viewMenu, "&View" );
  mymenubar->Append( helpMenu, "&Help" );
  SetMenuBar( mymenubar );

  MyStatusBar* status = new MyStatusBar( this, -1 );
  SetStatusBar( status );  
  status->SetStatusText( "Welcome!" );

  main_sizer = new wxBoxSizer( wxVERTICAL );
  canvas = new MyCanvas( this, -1 );
  main_sizer->Add( canvas );
  SetSizer( main_sizer );
  main_sizer->SetSizeHints( this );

  CentreOnScreen();
}

MyFrame::~MyFrame()
{
  if( wxGetApp().GetGame() )
    wxGetApp().EndGame();
}

void MyFrame::OnNew( wxCommandEvent& event )
{
  if( ProceedWithNewGame() ) {
    LocalPlayer* p1 = new LocalPlayer( wxGetApp().GetLocalPlayerName(), new GamePosP1() );
    //Player* p1 = wxGetApp().GetBotPlayer( new GamePosP1() );
    Player* p2 = wxGetApp().GetBotPlayer( new GamePosP2() );
    Player* p3 = wxGetApp().GetBotPlayer( new GamePosP3() );
    Player* p4 = wxGetApp().GetBotPlayer( new GamePosP4() );
    wxGetApp().NewGame( new Game( p1, p2, p3, p4, canvas ), p1 );
  }
}

void MyFrame::OnHostGame( wxCommandEvent& event )
{
  if( ProceedWithNewGame() ) {
    wxGetApp().EndGame();
    ServerDialog( this ).ShowModal();
  }
}

void MyFrame::OnConnectToGame( wxCommandEvent& event )
{
  if( ProceedWithNewGame() ) {
    wxGetApp().EndGame();
    RemoteDialog( this ).ShowModal();
  }
}

void MyFrame::OnEndGame( wxCommandEvent& event )
{
  if( wxGetApp().GetGame() ) {
    if(wxMessageBox("Are you sure?",
                    "End game", wxYES_NO, this ) == wxNO )
      return;
    wxGetApp().EndGame();
  }
}

void MyFrame::OnGamePrefs( wxCommandEvent& event )
{
  PrefsDialog( this ).ShowModal();
}

void MyFrame::OnViewScores( wxCommandEvent& event )
{
  Game *game = wxGetApp().GetGame();
  viewscores = event.IsChecked();
  if( game ) {
    if( viewscores )
      game->DisplayResults();
    else
      game->score->Show( false );
  }
}

void MyFrame::OnViewTrumph( wxCommandEvent& event )
{
  Game *game = wxGetApp().GetGame();
  viewtrumph = event.IsChecked();
  if( game )
    game->trumphdlg->Show( viewtrumph );
}

void MyFrame::Help( wxCommandEvent& event )
{
  wxMessageDialog msg( this,"Not implemented yet." );
  msg.ShowModal();
}

void MyFrame::About( wxCommandEvent& event )
{
  wxMessageDialog md( this, VERSION_STRING "\n\
(C) 2003, 2004 Rodrigo Araujo <rmaraujo@dcc.online.pt>\n\
Card graphics by jfitz technologies - http://www.jfitz.com/",
                      "About this game", wxOK );
  md.ShowModal();
}

void MyFrame::OnClose( wxCloseEvent& event )
{
//  event.Veto();
  Destroy();
}

bool MyFrame::ProceedWithNewGame()
{
  if( wxGetApp().GetGame() &&
      wxMessageBox("End current game and start a new one?",
                   "Game in progress", wxYES_NO, this ) == wxNO )
    return false;
  return true;
}
