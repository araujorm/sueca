#include "scoredialog.hpp"
#include <wx/button.h>
#include "main.hpp"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( StaticTextList );

// Score dialog implementation
BEGIN_EVENT_TABLE( ScoreDialog, wxDialog )
  EVT_BUTTON( wxID_ANY, ScoreDialog::OnButton )
  EVT_CLOSE( ScoreDialog::OnClose )
END_EVENT_TABLE();

ScoreDialog::ScoreDialog( wxWindow* parent, Team* nteam1, Team* nteam2, wxPoint& pos ):
  wxDialog( parent, -1, wxString( "Game scores" ), pos ), round( 1 ), displayed( 0 ),
  team1points( NULL ), team2points( NULL ), roundlabel( NULL ),
  new_round( true ), team1( nteam1 ), team2( nteam2 )
{
  gridsz = new wxFlexGridSizer( 3 );
  AddLine( "", team1->GetP1()->GetName() + "\n" + team1->GetP2()->GetName(),
           team2->GetP1()->GetName() + "\n" + team2->GetP2()->GetName(),
           wxFONTWEIGHT_BOLD, *wxBLACK, wxALIGN_CENTER, &team1text, &team2text );
  AddLine( "Victories", wxString::Format( "%hu", team1->GetWon() ),
           wxString::Format( "%hu", team2->GetWon() ),
           wxFONTWEIGHT_BOLD, *wxRED, wxALIGN_RIGHT, &team1won, &team2won );
  top_sizer = new wxBoxSizer( wxVERTICAL );
  top_sizer->Add( 1, 5 );
  top_sizer->Add( gridsz, 0, wxLEFT | wxRIGHT, 20 );
  top_sizer->Add( 1, 10 );
  top_sizer->Add( new wxButton( this, -1, "Close" ), 0, wxALIGN_CENTER );
  top_sizer->Add( 1, 5 );
  SetSizer( top_sizer );  // Fit is called in UpdateRoundResults()
}

void ScoreDialog::UpdateRoundResults( const wxString& s1, const wxString& s2,
				      bool force )
{
  if( ! force && ! IsShown() )
    return;
  wxString t1points = wxString::Format( "%s %hu", s1.c_str(), team1->GetRoundPoints() );
  wxString t2points = wxString::Format( "%s %hu", s2.c_str(), team2->GetRoundPoints() );

  if( new_round ) {
    new_round = false;
    wxFont fnt( *wxNORMAL_FONT );
    fnt.SetWeight( wxFONTWEIGHT_NORMAL );
    if( team1points ) {  // Testing one of them should be enough
      team1points->SetFont( fnt );
      team1points->SetForegroundColour( *wxBLACK );
      team2points->SetFont( fnt );
      team2points->SetForegroundColour( *wxBLACK );
      roundlabel->SetFont( fnt );
      roundlabel->SetForegroundColour( *wxBLACK );
    }
    if( displayed == MAX_DISPLAYED_ROUNDS ) {
      while( oldobjs.GetCount() != 3 ) {  // Leave one row of rounds
        StaticTextList::Node *node = oldobjs.GetFirst();
        wxStaticText* child = node->GetData();
        gridsz->Detach( child );
        child->Destroy();
        oldobjs.DeleteNode( node );
      }
      gridsz->Layout();
      displayed = 1;
    }
    AddLine( wxString::Format( "Round %u", round ),
             t1points, t2points, wxFONTWEIGHT_BOLD, *wxBLUE,
             wxALIGN_RIGHT, &team1points, &team2points, &roundlabel );
    oldobjs.Append( roundlabel );
    oldobjs.Append( team1points );
    oldobjs.Append( team2points );
    displayed++;
  }
  else {
    team1points->SetLabel( t1points );
    team2points->SetLabel( t2points );
  }
  top_sizer->SetSizeHints( this );
}

bool ScoreDialog::Show( bool show )
{
  if( show )
    UpdateRoundResults( "", "", true );
  return wxDialog::Show( show );
}

void ScoreDialog::SetEndRoundResults()
{
  team1won->SetLabel( wxString::Format( "%hu", team1->GetWon() ) );
  team2won->SetLabel( wxString::Format( "%hu", team2->GetWon() ) );
  UpdateRoundResults( team1->GetWonStr().c_str(), team2->GetWonStr().c_str(), true );
  new_round = true;
  round++;
}

void ScoreDialog::AddLine( wxString header, wxString s1, wxString s2,
                           int fontw, wxColour color, int flags,
                           wxStaticText** stxt1, wxStaticText** stxt2,
                           wxStaticText** stxth )
{
  wxFont fnt( *wxNORMAL_FONT );
  fnt.SetWeight( fontw );
  wxStaticText *txth = new wxStaticText( this, -1, header, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER );
  txth->SetFont( fnt );
  txth->SetForegroundColour( color );
  wxStaticText *txt1 = new wxStaticText( this, -1, s1, wxDefaultPosition, wxDefaultSize, flags );
  txt1->SetFont( fnt );
  txt1->SetForegroundColour( color );
  wxStaticText *txt2 = new wxStaticText( this, -1, s2, wxDefaultPosition, wxDefaultSize, flags );
  txt2->SetFont( fnt );
  txt2->SetForegroundColour( color );
  gridsz->Add( txth, 0, wxEXPAND | wxRIGHT, 20 );
  gridsz->Add( txt1, 0, wxEXPAND | wxRIGHT, 20 );
  gridsz->Add( txt2, 0, wxEXPAND );
  if( stxt1 )
    *stxt1 = txt1;
  if( stxt2 )
    *stxt2 = txt2;
  if( stxth )
    *stxth = txth;
}

void ScoreDialog::OnClose( wxCloseEvent& event )
{
  if( event.CanVeto() ) {
    MyFrame* frame = wxGetApp().GetFrame();
    frame->viewscores = false;
    frame->viewMenu->Check( ID_VIEW_SCORES, false );
    Show( false );
  }
  else
    Destroy();
}

void ScoreDialog::RefreshNames()
{
  team1text->SetLabel( team1->GetP1()->GetName() + "\n" + team1->GetP2()->GetName() );
  team2text->SetLabel( team2->GetP1()->GetName() + "\n" + team2->GetP2()->GetName() );
  top_sizer->SetSizeHints( this );
}
