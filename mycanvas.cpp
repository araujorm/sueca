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

#include <cmath>
#include "mycanvas.hpp"
#include <wx/gdicmn.h>
#include <wx/intl.h>
#include "myevents.hpp"
#include "main.hpp"

DEFINE_EVENT_TYPE( CARD_MOVE_EVT_TYPE );

#define EVT_CARD_MOVE(func) DECLARE_EVENT_TABLE_ENTRY( CARD_MOVE_EVT_TYPE, wxID_ANY, wxID_ANY, ( wxObjectEventFunction ) & func, (wxObject *) NULL ),

unsigned int CardMoveEvent::skipped = 0;
unsigned int CardMoveEvent::inst = 0;
unsigned int CardMoveEvent::moved = 0;

CardMoveEvent::CardMoveEvent( Card *crd, wxPoint& i_destpos, Game* ownr ):
	wxEvent( 0, CARD_MOVE_EVT_TYPE ), card( crd ), owner( ownr )
{
	inst++;
	destpos.x = i_destpos.x;
	destpos.y = i_destpos.y;
	pos.x = card->GetPosition().x;
	pos.y = card->GetPosition().y;
	double xdist = destpos.x - pos.x;
	double ydist = destpos.y - pos.y;
	double hyp = sqrt( xdist*xdist + ydist*ydist );
	sine = xdist / hyp;
	cossine = ydist / hyp;
}

wxEvent *CardMoveEvent::Clone(void) const
{
	inst++;
	return new CardMoveEvent( *this );
}

NameLabel::NameLabel( Player* player, wxDC& dc ):
	m_text( player->GetName() ), m_visible( true )
{
	wxCoord w; wxCoord h;
	dc.SetFont( *wxSWISS_FONT );
	dc.GetTextExtent( m_text, &w, &h );
	m_area.SetWidth( w );
	m_area.SetHeight( h );
	m_pos = player->GetNamePos( wxSize( w, h ) );
	m_area.SetX( m_pos.x );
	m_area.SetY( m_pos.y );
}

void NameLabel::Draw( wxDC& dc )
{
	dc.SetFont( *wxSWISS_FONT );
	dc.DrawText( GetText(), GetPos().x, GetPos().y );
}


TrumphLabel::TrumphLabel( Player* player, Card* card, wxDC& dc ):
	m_card( card ), m_visible( true )
{
	m_text = card->GetType().GetShortName();
	m_bmp = card->GetSuit().GetBitmap();
	wxCoord w; wxCoord h;
	dc.GetTextExtent( m_text, &w, &h );
	m_bmppos = wxPoint( 0, 0 );
	int xwidth = m_bmp.GetWidth() + 3;
	m_textpos = wxPoint( xwidth, 0 );
	w += xwidth;
	h = h > m_bmp.GetHeight() ? h : m_bmp.GetHeight();
	m_pos = player->GetTrumphPos( wxSize( w, h ) );
	m_area.SetWidth( w );
	m_area.SetHeight( h );
	m_area.SetX( m_pos.x );
	m_area.SetY( m_pos.y );
}

void TrumphLabel::Draw( wxDC& dc )
{
	dc.DrawText( m_text, m_pos.x + m_textpos.x, m_pos.y + m_textpos.y );
	dc.DrawBitmap( m_bmp, m_pos.x + m_bmppos.x, m_pos.y + m_bmppos.y, true);
}

// CardFlashTimer implementation
CardFlashTimer::CardFlashTimer( MyCanvas* canvas ):
	wxTimer(), m_canvas( canvas ) {}

void CardFlashTimer::Notify()
{
	if( m_canvas->flashing ) {
		Card* card = m_canvas->flashing;
		m_canvas->flashing = NULL;
		card->ColorInvert( false );
		m_canvas->Refresh();
		m_canvas->Update();
	}
}

// MyCanvas
BEGIN_EVENT_TABLE( MyCanvas, wxPanel )
	EVT_PAINT( MyCanvas::OnPaint )
	EVT_ERASE_BACKGROUND( MyCanvas::OnEraseBackground )  // Prevent flickering
	EVT_SIZE( MyCanvas::OnSize )
	EVT_MOUSE_EVENTS( MyCanvas::OnMouseEvent )
	// Card move events
	EVT_CARD_MOVE( MyCanvas::OnCardMoveEvent )
END_EVENT_TABLE()

#include <wx/mstream.h>
#include "png/arrow.h"

MyCanvas::MyCanvas( wxFrame* parent, wxWindowID id ):
	wxPanel( parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER ),
	flashing( NULL ), statusbar( parent->GetStatusBar() ), m_localplayer( NULL ),
	m_trumph( NULL ), m_buffer( MC_X_SIZE, MC_Y_SIZE ), fltimer( this ),
	lastclicked( NULL ), m_scale( 1.0 ), m_offset_x( 0 ), m_offset_y( 0 ),
	m_arrow_dir( 0 ), m_arrow_visible( false )
{

	for( int i = 0; i < 4; i++ )
		m_names[i] = NULL;
	SetBackgroundColour( wxColour( 0, 128, 0 ) );
	ClearBackground();
	SetCursor( wxCursor( wxCURSOR_ARROW ) );
	SetMinSize( wxSize(MC_X_SIZE, MC_Y_SIZE) );

	// Create arrow bitmaps for all 4 directions from the left-pointing base
	wxMemoryInputStream arrow_stream( arrow, arrow_len );
	wxImage arrow_img( arrow_stream, wxBITMAP_TYPE_PNG );
	wxImage arrow_cw = arrow_img.Rotate90( true );
	m_arrows[0] = wxBitmap( arrow_img );                          // left (P1)
	m_arrows[1] = wxBitmap( arrow_cw );                            // up (P2, CW)
	m_arrows[2] = wxBitmap( arrow_cw.Rotate90( true ) );           // right (P3, CW+CW)
	m_arrows[3] = wxBitmap( arrow_img.Rotate90( false ) );         // down (P4, CCW)
}

void MyCanvas::OnSize( wxSizeEvent& event )
{
	wxSize sz = event.GetSize();
	double sx = (double)sz.GetWidth() / MC_X_SIZE;
	double sy = (double)sz.GetHeight() / MC_Y_SIZE;
	m_scale = ( sx < sy ) ? sx : sy;
	if( m_scale < 1.0 ) m_scale = 1.0;
	int scaled_w = (int)( MC_X_SIZE * m_scale );
	int scaled_h = (int)( MC_Y_SIZE * m_scale );
	m_offset_x = ( sz.GetWidth() - scaled_w ) / 2;
	m_offset_y = ( sz.GetHeight() - scaled_h ) / 2;
	// Recreate buffer at physical size
	m_buffer = wxBitmap( scaled_w, scaled_h );
	Refresh();
	event.Skip();
}

MyCanvas::~MyCanvas()
{
	ClearCards();
	ClearNames();
	ClearTrumph();
}

void MyCanvas::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
	wxPaintDC destdc( this );

	// Draw everything to the physical-size buffer with scaling
	wxMemoryDC bufdc;
	bufdc.SelectObject( m_buffer );
	bufdc.SetUserScale( m_scale, m_scale );
	bufdc.SetBackground( wxBrush( GetBackgroundColour(), wxBRUSHSTYLE_SOLID ) );
	wxRect fullrect( 0, 0, MC_X_SIZE, MC_Y_SIZE );
	DrawShapes( bufdc, fullrect );
	bufdc.SetUserScale( 1.0, 1.0 );
	bufdc.SelectObject( wxNullBitmap );

	// Clear the canvas background (for areas outside the scaled content)
	destdc.SetBackground( wxBrush( GetBackgroundColour(), wxBRUSHSTYLE_SOLID ) );
	destdc.Clear();

	// Blit the buffer 1:1 to the canvas
	int dest_w = (int)( MC_X_SIZE * m_scale );
	int dest_h = (int)( MC_Y_SIZE * m_scale );
	wxMemoryDC srcdc;
	srcdc.SelectObject( m_buffer );
	destdc.Blit( m_offset_x, m_offset_y, dest_w, dest_h, &srcdc, 0, 0 );
	srcdc.SelectObject( wxNullBitmap );
}

void MyCanvas::OnMouseEvent( wxMouseEvent& event )
{
	static int pos;
	static Card* raised = NULL;

	// Convert physical mouse coordinates to logical
	wxPoint lpt( (int)( ( event.GetX() - m_offset_x ) / m_scale ),
	       (int)( ( event.GetY() - m_offset_y ) / m_scale ) );

	if( event.LeftDClick() && m_trumph &&
	    m_trumph->GetRect().Contains( lpt ) ) {
		wxGetApp().GetFrame()->viewMenu->Check( ID_VIEW_TRUMPH, true );
		wxGetApp().GetGame()->trumphdlg->Show( true );
	}
	else if( event.LeftDown() && m_localplayer ) {
		Card *playcard = FindCard( lpt, &pos );
		if( playcard && playcard->IsPlayable() )
			switch( wxGetApp().GetGame()->PlayMove( m_localplayer, playcard ) ) {
			case MOVE_TURN:
				NotTurnWarning();
				break;
			case MOVE_INVALID:
				InvalidLocalMove( playcard );
				break;
			case MOVE_DELAYED:
				// Network game, the result will be handled by a RemoteHandler
				lastclicked = playcard;
				break;
			case MOVE_OK:
				break;
			}
	}
	if( event.RightDown() ) {
		// Neat efect of raising a playable card
		raised = FindCard( lpt, &pos );
		if( raised ) {
			if ( raised->GetTurned() || !raised->IsPlayable() )
				raised = NULL;
			else {
				CaptureMouse();
				m_displayList.DeleteObject( raised );
				m_displayList.Insert( raised );
				Refresh();
			}
		}
	}
	else if( event.RightUp() && raised ) {
		// Put the card back on its stacking position
		ReleaseMouse();
		m_displayList.DeleteObject( raised );
		m_displayList.Insert( pos, raised );
		Refresh();
		raised = NULL;
	}
	else if( event.Moving() && !event.RightIsDown() ) {
		// Display visible card names on status bar
		Card* under = FindCard( lpt, &pos );
		if( under ) {
			if( !under->GetTurned() )
				statusbar->SetStatusText( under->NameStr() );
		}
		else if( m_trumph && m_trumph->GetRect().Contains( lpt ) )
			statusbar->SetStatusText(
			  wxString::Format( _( "Trumph: %s" ),
			                    m_trumph->GetCardName() ) );
		else
			statusbar->SetStatusText( "" );
	}
}

void MyCanvas::DrawShapes( wxDC& dc, const wxRect& area )
{
	dc.DestroyClippingRegion();
	dc.SetClippingRegion( area );
	dc.Clear();
	// Cards
	CardList::Node* node = m_displayList.GetLast();
	while( node ) {
		Card* card = node->GetData();
		if( card->GetRect().Intersects( area ) )
			card->Draw( dc );
		node = node->GetPrevious();
	}
	// Player Name Labels
	for ( int i = 0; i < 4; i++ ) {
		NameLabel* nl;
		if ( ( nl = m_names[i] ) && nl->GetRect().Intersects( area ) )
			nl->Draw( dc );
	}
	// Trumph Label
	if ( m_trumph && m_trumph->GetRect().Intersects( area ) )
		m_trumph->Draw( dc );
	// Active player arrow
	if( m_arrow_visible ) {
		wxBitmap& arr = m_arrows[m_arrow_dir];
		wxRect arrrect( m_arrow_pos, wxSize( arr.GetWidth(), arr.GetHeight() ) );
		if( arrrect.Intersects( area ) )
			dc.DrawBitmap( arr, m_arrow_pos.x, m_arrow_pos.y, true );
	}
}

void MyCanvas::ClearNames()
{
	for( int i = 0; i < 4; i++ )
		if( m_names[i] ) {
			delete m_names[i];
			m_names[i] = NULL;
		}
}

void MyCanvas::ClearTrumph()
{
	if( m_trumph ) {
		delete m_trumph;
		m_trumph = NULL;
	}
}

Card* MyCanvas::FindCard( const wxPoint& pt, int* pos ) const
{
	int i = 0;
	CardList::Node* node = m_displayList.GetFirst();
	while( node ) {
		Card* card = node->GetData();
		if( card->HitTest( pt ) ) {
			if( pos )
				*pos = i;
			return card;
		}
		node = node->GetNext();
		i++;
	}
	return (Card*)NULL;
}

void MyCanvas::Add( Card* crd, int x, int y, bool turned ) {
	crd->SetTurned( turned );
	crd->SetPosition( wxPoint( x, y ) );
	GetDisplayList().Insert( crd );
	Refresh();
}

void MyCanvas::Remove( Card* crd, bool update )
{
	m_displayList.DeleteObject( crd );
	if( update )
		Refresh();
}

void MyCanvas::SetNameLabel( int playerno, Player* player )
{
	if( m_names[playerno] ) {
		delete m_names[playerno];
		m_names[playerno] = NULL;
		Refresh();
	}
	wxClientDC dc( this );
	m_names[playerno] = new NameLabel( player, dc );
	Refresh();
}

void MyCanvas::SetTrumphLabel( Player* player, Card* card )
{
	if( m_trumph ) {
		delete m_trumph;
		m_trumph = NULL;
		Refresh();
	}
	else
		wxGetApp().GetFrame()->viewMenu->Enable( ID_VIEW_TRUMPH, true );
	wxClientDC dc( this );
	m_trumph = new TrumphLabel( player, card, dc );
	Refresh();
}

void MyCanvas::SetActivePlayer( Player* player )
{
	// Clear old arrow
	if( m_arrow_visible ) {
		wxSize sz( m_arrows[m_arrow_dir].GetWidth(),
		           m_arrows[m_arrow_dir].GetHeight() );
		Refresh();
	}
	if( !player ) {
		m_arrow_visible = false;
		return;
	}
	// Set new arrow
	m_arrow_dir = player->GetArrowDir();
	wxSize arrsz( m_arrows[m_arrow_dir].GetWidth(),
	              m_arrows[m_arrow_dir].GetHeight() );
	m_arrow_pos = player->GetArrowPos( arrsz );
	m_arrow_visible = true;
	Refresh();
}

void MyCanvas::ClearActivePlayer()
{
	SetActivePlayer( NULL );
}

#define STOP_PREC 5e-1
void MyCanvas::OnCardMoveEvent( CardMoveEvent& event )
{
	// Ignore events concerning no longer existing games
	if( wxGetApp().GetGame() != event.owner )
		return;
	double diffx = event.destpos.x - event.pos.x;
	double diffy = event.destpos.y - event.pos.y;
	if( diffx*diffx + diffy*diffy <= STOP_PREC ) {
		wxGetApp().GetGame()->CardMoved( event.card );
		return;
	}
	wxRegion update_reg;
	update_reg.Union( event.card->GetRect() );
	wxPoint oldpos = event.card->GetPosition();
	event.pos.x += event.sine;
	event.pos.y += event.cossine;
	wxPoint newpos( (wxCoord)event.pos.x, (wxCoord)event.pos.y );
	// Update as necessary
	if( oldpos != newpos ) {
		event.card->SetPosition( newpos );
		update_reg.Union( event.card->GetRect() );
		Refresh();
		// Update all moving cards at once
		if( ++event.moved >= event.inst ) {
			unsigned int speed = wxGetApp().GetUpdateDelay();
			if( speed >= 10 ) {
				event.skipped = 0;
			}
			else if( ++event.skipped >= speed ) {
				wxMilliSleep( 5 );
				Update();
				event.skipped = 0;
			}
			event.moved = 0;
		}
	}
	AddPendingEvent( event );
}

void MyCanvas::MoveCardTo( Card* card, wxPoint destpos, bool raise )
{
	if( raise ) {
		if( !m_displayList.DeleteObject( card ) )
			return;
		m_displayList.Insert( card );
	}
	CardMoveEvent event( card, destpos, wxGetApp().GetGame() );
	AddPendingEvent( event );
}

void MyCanvas::FlashCard( Card* card )
{
	// Make sure only one card flashes at a time
	if( !fltimer.IsRunning() ) {
		flashing = card;
		card->ColorInvert( true );
		if( fltimer.Start( 250, wxTIMER_ONE_SHOT ) ) {
			Refresh();
			Update();
		}
		else {
			// can happen if system resources are low,
			// so we have to cancel this effect
			flashing = NULL;
			card->ColorInvert( false );
		}
	}
}

void MyCanvas::NotTurnWarning()
{
	statusbar->SetStatusText( _( "It's not your turn!" ) );
}

void MyCanvas::InvalidLocalMove( Card* card )
{
	if( card || ( card = lastclicked ) ) {
		statusbar->SetStatusText( _( "Invalid move!" ) );
		FlashCard( card );
	}
}
