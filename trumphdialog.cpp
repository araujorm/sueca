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

#include "trumphdialog.hpp"
#include <wx/button.h>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include "main.hpp"

// Trumph dialog implementation
BEGIN_EVENT_TABLE( TrumphDialog, wxDialog )
  EVT_BUTTON( wxID_ANY, TrumphDialog::OnButton )
  EVT_CLOSE( TrumphDialog::OnClose )
END_EVENT_TABLE();

static char* no_trumph_str = "No trumph set";

TrumphDialog::TrumphDialog( wxWindow* parent, wxPoint& pos ):
  wxDialog( parent, wxID_ANY, wxString( "Trumph" ), pos ), empty_bmp( 1, 1 )
{
  bmp = new wxStaticBitmap( this, wxID_ANY, empty_bmp );
  text = new wxStaticText( this, wxID_ANY, no_trumph_str, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER );
  top_sizer = new wxBoxSizer( wxVERTICAL );
  top_sizer->Add( 1, 10 );
  top_sizer->Add( bmp, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 10 );
  top_sizer->Add( text, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER, 10 );
  top_sizer->Add( 1, 5 );
  top_sizer->Add( new wxButton( this, wxID_ANY, "Close" ), 0, wxALIGN_CENTER );
  top_sizer->Add( 1, 10 );
  SetSizer( top_sizer );
  top_sizer->SetSizeHints( this );
}

void TrumphDialog::UpdateTrumph( Card* trumph, Player* owner )
{
  wxBitmap dispbmp;
  wxString txtstr;
  if( !trumph || !owner ) {
    dispbmp = empty_bmp;
    txtstr = no_trumph_str;
  }
  else {
    dispbmp = trumph->GetBitmap();
    txtstr = trumph->NameStr() + "\nOwned by " + owner->GetName();
  }
  bmp->SetBitmap( dispbmp );
  text->SetLabel( txtstr );
  top_sizer->SetSizeHints( this );
}

void TrumphDialog::OnClose( wxCloseEvent& event )
{
  if( event.CanVeto() ) {
    MyFrame* frame = wxGetApp().GetFrame();
    frame->viewtrumph = false;
    frame->viewMenu->Check( ID_VIEW_TRUMPH, false );
    Show( false );
  }
  else
    Destroy();
}
