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

#include "lasttrickdialog.hpp"
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/intl.h>
#include "main.hpp"

// Card cross layout constants
#define LT_CARD_W 71
#define LT_CARD_H 96
#define LT_GAP 12
#define LT_NAME_GAP 8
#define LT_MIN_MARGIN 15

// Fixed height: cards cross + space for top/bottom names
#define LT_PANEL_H (LT_CARD_H * 2 + LT_GAP * 2 + 40)

// TrickPanel implementation
BEGIN_EVENT_TABLE( TrickPanel, wxPanel )
	EVT_PAINT( TrickPanel::OnPaint )
END_EVENT_TABLE();

TrickPanel::TrickPanel( wxWindow* parent ):
	wxPanel( parent, wxID_ANY, wxDefaultPosition,
	         wxSize( LT_CARD_W * 2 + LT_GAP * 2 + LT_MIN_MARGIN * 2,
	                 LT_PANEL_H ) )
{
	Clear();
}

void TrickPanel::Clear()
{
	for( int i = 0; i < 4; i++ ) {
		m_cards[i] = NULL;
		m_names[i] = "";
		m_winner[i] = false;
	}
}

void TrickPanel::SetCard( int pos, Card* card, const wxString& name,
                          bool winner )
{
	m_cards[pos] = card;
	m_names[pos] = name;
	m_winner[pos] = winner;
}

void TrickPanel::RecalcSize()
{
	wxClientDC dc( this );
	dc.SetFont( *wxNORMAL_FONT );

	// Measure text height and lateral name widths
	wxCoord left_w = 0, right_w = 0, text_h = 0, tw, th;
	for( int i = 0; i < 4; i++ ) {
		if( m_names[i].Length() > 0 ) {
			dc.GetTextExtent( m_names[i], &tw, &th );
			if( th > text_h ) text_h = th;
			if( i == 3 ) left_w = tw;
			if( i == 1 ) right_w = tw;
		}
	}

	// Panel width: left name + gap + card cross + gap + right name + margins
	int cross_w = LT_CARD_W * 2 + LT_GAP * 2;
	int panel_w = LT_MIN_MARGIN + left_w + LT_NAME_GAP
	              + cross_w + LT_NAME_GAP + right_w + LT_MIN_MARGIN;

	// Ensure minimum width for top/bottom names too
	for( int i = 0; i <= 2; i += 2 ) {
		if( m_names[i].Length() > 0 ) {
			dc.GetTextExtent( m_names[i], &tw, &th );
			if( tw + LT_MIN_MARGIN * 2 > panel_w )
				panel_w = tw + LT_MIN_MARGIN * 2;
		}
	}

	// Panel height: top name + gap + card cross + gap + bottom name + margins
	int cross_h = LT_CARD_H * 2 + LT_GAP * 2;
	int panel_h = LT_MIN_MARGIN + text_h + LT_NAME_GAP
	              + cross_h + LT_NAME_GAP + text_h + LT_MIN_MARGIN;

	wxSize sz( panel_w, panel_h );
	SetMinSize( sz );
	SetMaxSize( sz );
	SetSize( sz );
}

void TrickPanel::OnPaint( wxPaintEvent& event )
{
	wxPaintDC dc( this );
	int cx = GetCX();
	int cy = GetCY();

	// Card positions relative to panel center
	wxPoint cardpos[4] = {
		wxPoint( cx - LT_CARD_W / 2, cy + LT_GAP ),                // bottom
		wxPoint( cx + LT_GAP, cy - LT_CARD_H / 2 ),                // right
		wxPoint( cx - LT_CARD_W / 2, cy - LT_GAP - LT_CARD_H ),   // top
		wxPoint( cx - LT_GAP - LT_CARD_W, cy - LT_CARD_H / 2 )    // left
	};

	dc.SetFont( *wxNORMAL_FONT );

	for( int i = 0; i < 4; i++ ) {
		if( !m_cards[i] )
			continue;

		// Draw card
		wxBitmap& bmp = m_cards[i]->GetBitmap();
		wxMemoryDC memDC;
		memDC.SelectObject( bmp );
		dc.Blit( cardpos[i].x, cardpos[i].y,
		         bmp.GetWidth(), bmp.GetHeight(),
		         &memDC, 0, 0, wxCOPY, true );

		// Measure name
		wxCoord tw, th;
		dc.GetTextExtent( m_names[i], &tw, &th );

		wxPoint namepos;
		switch( i ) {
		case 0: // bottom - name below card
			namepos = wxPoint( cardpos[i].x + ( LT_CARD_W - tw ) / 2,
			                   cardpos[i].y + LT_CARD_H + LT_NAME_GAP );
			break;
		case 1: // right - name right of card
			namepos = wxPoint( cardpos[i].x + LT_CARD_W + LT_NAME_GAP,
			                   cardpos[i].y + ( LT_CARD_H - th ) / 2 );
			break;
		case 2: // top - name above card
			namepos = wxPoint( cardpos[i].x + ( LT_CARD_W - tw ) / 2,
			                   cardpos[i].y - th - LT_NAME_GAP );
			break;
		case 3: // left - name left of card
			namepos = wxPoint( cardpos[i].x - tw - LT_NAME_GAP,
			                   cardpos[i].y + ( LT_CARD_H - th ) / 2 );
			break;
		}

		if( m_winner[i] ) {
			dc.SetTextForeground( wxColour( 0, 0, 128 ) );
			dc.SetFont( dc.GetFont().Bold() );
		}
		else {
			dc.SetTextForeground( *wxBLACK );
			dc.SetFont( *wxNORMAL_FONT );
		}
		dc.DrawText( m_names[i], namepos.x, namepos.y );
	}
}

// LastTrickDialog implementation
BEGIN_EVENT_TABLE( LastTrickDialog, wxDialog )
	EVT_BUTTON( wxID_ANY, LastTrickDialog::OnButton )
	EVT_CLOSE( LastTrickDialog::OnClose )
END_EVENT_TABLE();

LastTrickDialog::LastTrickDialog( wxWindow* parent, wxPoint& pos ):
	wxDialog( parent, wxID_ANY, _( "Last Trick" ), pos,
	          wxDefaultSize, wxDEFAULT_DIALOG_STYLE & ~wxRESIZE_BORDER ),
	m_no_focus( false )
{
	m_panel = new TrickPanel( this );

	m_winner_text = new wxStaticText( this, wxID_ANY, _( "No trick yet" ),
	  wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER );

	m_top_sizer = new wxBoxSizer( wxVERTICAL );
	m_top_sizer->Add( m_panel, 0, wxALIGN_CENTER | wxALL, 10 );
	m_top_sizer->Add( m_winner_text, 0, wxALIGN_CENTER | wxBOTTOM, 5 );
	m_top_sizer->Add( new wxButton( this, wxID_ANY, _( "Close" ) ), 0,
	                wxALIGN_CENTER | wxBOTTOM, 10 );
	SetSizer( m_top_sizer );
	m_top_sizer->Fit( this );
	m_top_sizer->SetSizeHints( this );
}

void LastTrickDialog::UpdateTrick( PlayerIterator* players,
  const CardList& played, Player* winner )
{
	m_panel->Clear();
	CardList::Node* node = played.GetFirst();

	for( int i = 0; i < 4 && node; i++ ) {
		Card* card = node->GetData();
		Player* player = players->GetCurrent();
		wxString posname = player->GetNamePosStr();
		int slot;
		if( posname == "bottom" ) slot = 0;
		else if( posname == "right" ) slot = 1;
		else if( posname == "top" ) slot = 2;
		else slot = 3;
		m_panel->SetCard( slot, card, player->GetName(), player == winner );
		node = node->GetNext();
		players->GetNext();
	}

	m_winner_text->SetLabel(
	  wxString::Format( _( "Won by %s" ), winner->GetName() ) );
	RefitDialog();
	m_panel->Refresh();
}

void LastTrickDialog::RefitDialog()
{
	m_panel->RecalcSize();
	m_top_sizer->SetSizeHints( this );
	m_top_sizer->Fit( this );
}

bool LastTrickDialog::Show( bool show )
{
	// ShowWithoutActivating() internally calls Show() via virtual
	// dispatch; without the guard we'd re-enter this override and
	// recurse until the stack overflows.
	if( m_no_focus )
		return wxDialog::Show( show );
	if( show ) {
		m_no_focus = true;
		ShowWithoutActivating();
		m_no_focus = false;
		return true;
	}
	return wxDialog::Show( false );
}

void LastTrickDialog::OnClose( wxCloseEvent& event )
{
	if( event.CanVeto() ) {
		MyFrame* frame = wxGetApp().GetFrame();
		frame->viewlasttrick = false;
		frame->viewMenu->Check( ID_VIEW_LASTTRICK, false );
		Show( false );
	}
	else
		Destroy();
}
