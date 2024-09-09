#include <cstdlib>  // for rand() and srand()
#include <ctime>  // time()
#include "cards.hpp"
#include <wx/dcmemory.h>

#include <wx/listimpl.cpp>
WX_DEFINE_LIST( CardList );

// Card type implementation
CardType::CardType( cardtype_t id, char* name, char *shortname, short value ):
  m_id( id ), m_name ( name ), m_shortname( shortname ), m_value ( value ) {}

// Card suit implementation
CardSuit::CardSuit( cardsuit_t id ):
  m_id( id ), m_name ( "" ), m_bitmap( wxNullBitmap ) {}
CardSuit::CardSuit( cardsuit_t id, char* name, char* xpmdata[] ):
  m_id( id ), m_name ( name ), m_bitmap( xpmdata ) {}

// Card implementation
Card::Card( Deck *deck, CardType& type, CardSuit& suit, char** xpmdata):
  m_deck( deck ), m_type( type ), m_suit( suit ), m_turned( false ),
  m_playable( false ), m_bitmap( xpmdata ), blitop( wxCOPY ) {}

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
  return rect.Inside( pt.x, pt.y );
}

bool Card::Draw( wxDC& dc )
{
  wxBitmap bitmap = m_turned ? m_deck->GetFace() : GetBitmap();
  if( bitmap.Ok() ) {
    wxMemoryDC memDC;
    memDC.SelectObject( bitmap );

    dc.Blit( m_pos.x, m_pos.y, bitmap.GetWidth(), bitmap.GetHeight(),
             & memDC, 0, 0, blitop, TRUE );

    return TRUE;
  }
  else
    return FALSE;
}

void Card::ColorInvert( bool inverted )
{
  blitop = inverted ? wxSRC_INVERT : wxCOPY;
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

// Permanent card suits
class SuitNul: public CardSuit
{
public: SuitNul(): CardSuit( UNKNOWN_CARD_SUIT ) {}
};
#include "xpm_cards/clubs.xpm"
class SuitClubs: public CardSuit
{
public: SuitClubs(): CardSuit( CLUBS, "Clubs", clubs_xpm ) {}
};
#include "xpm_cards/diamonds.xpm"
class SuitDiamonds: public CardSuit
{
public: SuitDiamonds(): CardSuit( DIAMONDS, "Diamonds", diamonds_xpm ) {}
};
#include "xpm_cards/hearts.xpm"
class SuitHearts: public CardSuit
{
public: SuitHearts(): CardSuit( HEARTS, "Hearts", hearts_xpm ) {}
};
#include "xpm_cards/spades.xpm"
class SuitSpades: public CardSuit
{
public: SuitSpades(): CardSuit ( SPADES, "Spades", spades_xpm ) {}
};

// Deck implementation
//#include "xpm_cards/b1fh.xpm"
// TODO: Add more faces
#include "xpm_cards/b1fv.xpm"
#define DECK_FACE b1fv
/*
#include "xpm_cards/b1pb.xpm"
#include "xpm_cards/b1pl.xpm"
#include "xpm_cards/b1pr.xpm"
#include "xpm_cards/b1pt.xpm"
#include "xpm_cards/b2fh.xpm"
#include "xpm_cards/b2fv.xpm"
#include "xpm_cards/b2pb.xpm"
#include "xpm_cards/b2pl.xpm"
#include "xpm_cards/b2pr.xpm"
#include "xpm_cards/b2pt.xpm"
#include "xpm_cards/c10.xpm"
*/
#include "xpm_cards/c1.xpm"
#include "xpm_cards/c2.xpm"
#include "xpm_cards/c3.xpm"
#include "xpm_cards/c4.xpm"
#include "xpm_cards/c5.xpm"
#include "xpm_cards/c6.xpm"
#include "xpm_cards/c7.xpm"
//#include "xpm_cards/c8.xpm"
//#include "xpm_cards/c9.xpm"
#include "xpm_cards/cj.xpm"
#include "xpm_cards/ck.xpm"
#include "xpm_cards/cq.xpm"
//#include "xpm_cards/d10.xpm"
#include "xpm_cards/d1.xpm"
#include "xpm_cards/d2.xpm"
#include "xpm_cards/d3.xpm"
#include "xpm_cards/d4.xpm"
#include "xpm_cards/d5.xpm"
#include "xpm_cards/d6.xpm"
#include "xpm_cards/d7.xpm"
//#include "xpm_cards/d8.xpm"
//#include "xpm_cards/d9.xpm"
#include "xpm_cards/dj.xpm"
#include "xpm_cards/dk.xpm"
#include "xpm_cards/dq.xpm"
//#include "xpm_cards/ec.xpm"
//#include "xpm_cards/h10.xpm"
#include "xpm_cards/h1.xpm"
#include "xpm_cards/h2.xpm"
#include "xpm_cards/h3.xpm"
#include "xpm_cards/h4.xpm"
#include "xpm_cards/h5.xpm"
#include "xpm_cards/h6.xpm"
#include "xpm_cards/h7.xpm"
//#include "xpm_cards/h8.xpm"
//#include "xpm_cards/h9.xpm"
#include "xpm_cards/hj.xpm"
#include "xpm_cards/hk.xpm"
#include "xpm_cards/hq.xpm"
//#include "xpm_cards/jb.xpm"
//#include "xpm_cards/jr.xpm"
//#include "xpm_cards/s10.xpm"
#include "xpm_cards/s1.xpm"
#include "xpm_cards/s2.xpm"
#include "xpm_cards/s3.xpm"
#include "xpm_cards/s4.xpm"
#include "xpm_cards/s5.xpm"
#include "xpm_cards/s6.xpm"
#include "xpm_cards/s7.xpm"
//#include "xpm_cards/s8.xpm"
//#include "xpm_cards/s9.xpm"
#include "xpm_cards/sj.xpm"
#include "xpm_cards/sk.xpm"
#include "xpm_cards/sq.xpm"
Deck::Deck():
  m_face( DECK_FACE )
{
  SuitNul nulsuit;
  nulcard = new Card( this, typeNul, nulsuit, DECK_FACE );
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
  char** xpms[4][10] = {
    { h2, h3, h4, h5, h6, hq, hj, hk, h7, h1 },
    { c2, c3, c4, c5, c6, cq, cj, ck, c7, c1 },
    { d2, d3, d4, d5, d6, dq, dj, dk, d7, d1 },
    { s2, s3, s4, s5, s6, sq, sj, sk, s7, s1 }
  };
  int n = 0;
  for( int s = 0; s < 4; s++ )
    for( int t = 0; t < 10; t++ ) {
      Card* card = new Card( this, types[t], suits[s], xpms[s][t] );
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
