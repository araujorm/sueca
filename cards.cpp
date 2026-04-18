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

#include <cstdlib>  // for rand() and srand()
#include <ctime>  // time()
#include "cards.hpp"
#include <wx/dcmemory.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include "main.hpp"

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( CardList );

// Helper: load bitmap from embedded PNG data
wxBitmap BitmapFromPNG( const unsigned char* data, unsigned int len )
{
	wxMemoryInputStream stream( data, len );
	wxImage img( stream, wxBITMAP_TYPE_PNG );
	return wxBitmap( img );
}

// Card type implementation
CardType::CardType( cardtype_t id, char* name, char *shortname, short value ):
	m_id( id ), m_name ( name ), m_shortname( shortname ), m_value ( value ) {}

// Card suit implementation
CardSuit::CardSuit( cardsuit_t id ):
	m_id( id ), m_name ( "" ), m_bitmap( wxNullBitmap ) {}
CardSuit::CardSuit( cardsuit_t id, char* name, const unsigned char* pngdata, unsigned int pnglen ):
	m_id( id ), m_name ( name ), m_bitmap( BitmapFromPNG( pngdata, pnglen ) ) {}

// Card implementation
Card::Card( Deck *deck, CardType& type, CardSuit& suit,
            const unsigned char* pngdata, unsigned int pnglen ):
	m_deck( deck ), m_type( type ), m_suit( suit ), m_turned( false ),
	m_playable( false ), m_bitmap( BitmapFromPNG( pngdata, pnglen ) ),
	m_inverted( false ) {}

wxString Card::NameStr()
{
	return ( wxString( m_type.GetName() ) + wxString( " of " ) +
	         wxString( m_suit.GetName() ) );
}

wxString Card::ShortStr()
{
	return wxString::Format( "%s%c", m_type.GetShortName(),
	                         m_suit.GetName()[0] );
}

bool Card::HitTest( const wxPoint& pt ) const
{
	wxRect rect( GetRect() );
	return rect.Contains( pt.x, pt.y );
}

bool Card::Draw( wxDC& dc )
{
	// Pick the bitmap to draw: the deck's back if the card is turned
	// down, otherwise the face - possibly the pre-computed color-negated
	// face while a flash is active.
	wxBitmap bitmap = m_turned ? m_deck->GetFace() :
	                  ( m_inverted ? m_bitmap_inverted : m_bitmap );
	if( bitmap.Ok() ) {
		wxMemoryDC memDC;
		memDC.SelectObject( bitmap );
		dc.Blit( m_pos.x, m_pos.y, bitmap.GetWidth(), bitmap.GetHeight(),
		         & memDC, 0, 0, wxCOPY, TRUE );
		return TRUE;
	}
	else
		return FALSE;
}

void Card::ColorInvert( bool inverted )
{
	m_inverted = inverted;
	// Build the inverted bitmap on first use. Historically this effect
	// was done by blitting with wxSRC_INVERT, but on modern backends
	// (GTK3/Cairo in particular) raster ops other than wxCOPY are not
	// reliably supported, so we pre-compute the negated face and swap
	// it in during the flash.
	if( inverted && ! m_bitmap_inverted.IsOk() && m_bitmap.IsOk() ) {
		wxImage img = m_bitmap.ConvertToImage();
		unsigned char* rgb = img.GetData();
		int n = img.GetWidth() * img.GetHeight() * 3;
		for( int i = 0; i < n; i++ )
			rgb[i] = 255 - rgb[i];
		// Alpha channel (if any) is left untouched - we only invert
		// colour, not transparency.
		m_bitmap_inverted = wxBitmap( img );
	}
}

// Permanent card types
static CardType typeNul( UNKNOWN_CARD_TYPE, "", "", 0 );
static CardType typeTwo( TWO, "Two", "2", 0 );
static CardType typeThree( THREE, "Three", "3", 0 );
static CardType typeFour( FOUR, "Four", "4", 0 );
static CardType typeFive( FIVE, "Five", "5", 0 );
static CardType typeSix( SIX, "Six", "6", 0 );
static CardType typeQueen( QUEEN, "Queen", "Q", 2 );
static CardType typeJack( JACK, "Jack", "J", 3 );
static CardType typeKing( KING, "King", "K", 4 );
static CardType typeSeven( SEVEN, "Seven", "7", 10 );
static CardType typeAce( ACE, "Ace", "A", 11 );

// Embedded PNG data
#include "png/clubs.h"
#include "png/diamonds.h"
#include "png/hearts.h"
#include "png/spades.h"
#include "png/b1fv.h"
#include "png/b2fv.h"
#include "png/c1.h"
#include "png/c2.h"
#include "png/c3.h"
#include "png/c4.h"
#include "png/c5.h"
#include "png/c6.h"
#include "png/c7.h"
#include "png/cj.h"
#include "png/ck.h"
#include "png/cq.h"
#include "png/d1.h"
#include "png/d2.h"
#include "png/d3.h"
#include "png/d4.h"
#include "png/d5.h"
#include "png/d6.h"
#include "png/d7.h"
#include "png/dj.h"
#include "png/dk.h"
#include "png/dq.h"
#include "png/h1.h"
#include "png/h2.h"
#include "png/h3.h"
#include "png/h4.h"
#include "png/h5.h"
#include "png/h6.h"
#include "png/h7.h"
#include "png/hj.h"
#include "png/hk.h"
#include "png/hq.h"
#include "png/s1.h"
#include "png/s2.h"
#include "png/s3.h"
#include "png/s4.h"
#include "png/s5.h"
#include "png/s6.h"
#include "png/s7.h"
#include "png/sj.h"
#include "png/sk.h"
#include "png/sq.h"

// Get card back PNG data by type
void GetCardBackData( int back, const unsigned char** data, unsigned int* len )
{
	if( back == CARDBACK_RED ) {
		*data = b2fv;
		*len = b2fv_len;
	}
	else {
		*data = b1fv;
		*len = b1fv_len;
	}
}

// Permanent card suits
class SuitNul: public CardSuit
{
public: SuitNul(): CardSuit( UNKNOWN_CARD_SUIT ) {}
};
class SuitClubs: public CardSuit
{
public: SuitClubs(): CardSuit( CLUBS, "Clubs", clubs, clubs_len ) {}
};
class SuitDiamonds: public CardSuit
{
public: SuitDiamonds(): CardSuit( DIAMONDS, "Diamonds", diamonds, diamonds_len ) {}
};
class SuitHearts: public CardSuit
{
public: SuitHearts(): CardSuit( HEARTS, "Hearts", hearts, hearts_len ) {}
};
class SuitSpades: public CardSuit
{
public: SuitSpades(): CardSuit ( SPADES, "Spades", spades, spades_len ) {}
};

// Deck implementation
Deck::Deck()
{
	const unsigned char* face_data;
	unsigned int face_len;
	GetCardBackData( wxGetApp().GetCardBack(), &face_data, &face_len );
	m_face = BitmapFromPNG( face_data, face_len );
	SuitNul nulsuit;
	nulcard = new Card( this, typeNul, nulsuit, face_data, face_len );
	CardSuit suits[] = {
		SuitHearts(),
		SuitClubs(),
		SuitDiamonds(),
		SuitSpades()
	};
	CardType types[] = {
		typeTwo,
		typeThree,
		typeFour,
		typeFive,
		typeSix,
		typeQueen,
		typeJack,
		typeKing,
		typeSeven,
		typeAce
	};
	struct { const unsigned char* data; unsigned int len; } pngs[4][10] = {
		{ {h2, h2_len}, {h3, h3_len},
		  {h4, h4_len}, {h5, h5_len},
		  {h6, h6_len}, {hq, hq_len},
		  {hj, hj_len}, {hk, hk_len},
		  {h7, h7_len}, {h1, h1_len} },
		{ {c2, c2_len}, {c3, c3_len},
		  {c4, c4_len}, {c5, c5_len},
		  {c6, c6_len}, {cq, cq_len},
		  {cj, cj_len}, {ck, ck_len},
		  {c7, c7_len}, {c1, c1_len} },
		{ {d2, d2_len}, {d3, d3_len},
		  {d4, d4_len}, {d5, d5_len},
		  {d6, d6_len}, {dq, dq_len},
		  {dj, dj_len}, {dk, dk_len},
		  {d7, d7_len}, {d1, d1_len} },
		{ {s2, s2_len}, {s3, s3_len},
		  {s4, s4_len}, {s5, s5_len},
		  {s6, s6_len}, {sq, sq_len},
		  {sj, sj_len}, {sk, sk_len},
		  {s7, s7_len}, {s1, s1_len} }
	};
	int n = 0;
	for( int s = 0; s < 4; s++ )
		for( int t = 0; t < 10; t++ ) {
			Card* card = new Card( this, types[t], suits[s],
			                       pngs[s][t].data, pngs[s][t].len );
			cards[n++] = card;
			cardmap[card->ShortStr()] = card;
		}
}

Deck::~Deck()
{
	delete nulcard;
	for( int i = 0; i < 40; i++ )
		delete cards[i];
}

void Deck::Shuffle()
{
	for( int i = 0; i < 40; i++ ) {
		int r1 = (int)( 40.0 * rand() / ( RAND_MAX + 1.0 ) );
		int r2 = (int)( 40.0 * rand() / ( RAND_MAX + 1.0 ) );
		Card* t = cards[r1];
		cards[r1] = cards[r2];
		cards[r2] = t;
	}
}
