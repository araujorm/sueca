#ifndef _CARDS_HPP_
#define _CARDS_HPP_ 1

// Forward declarations
class CardType;
class CardSuit;
class Card;
class CardList;
class Deck;

#include <wx/dc.h>
#include <wx/bitmap.h>
#include <wx/string.h>
#include <wx/list.h>
#include <wx/hashmap.h>
#include <wx/timer.h>

// Card types
enum cardtype_t { TWO=2, THREE, FOUR, FIVE, SIX, QUEEN, JACK, KING, SEVEN, ACE, UNKNOWN_CARD_TYPE };
class CardType
{
public:
  CardType( cardtype_t id, char* name, char* shortname, short value );
  cardtype_t GetId() const { return m_id; }
  unsigned short GetValue() const { return m_value; }
  char* GetName() const { return m_name; }
  char* GetShortName() const { return m_shortname; }
  bool operator !=( const CardType& other ) { return m_id != other.GetId(); }
  bool operator ==( const CardType& other ) { return m_id == other.GetId(); }
  bool operator <( const CardType& other ) { return m_id < other.GetId(); }
  bool operator >( const CardType& other ) { return m_id > other.GetId(); }
private:
  cardtype_t m_id;
  char* m_name;
  char* m_shortname;
  unsigned short m_value;
};

// Card suits
enum cardsuit_t { CLUBS=0, DIAMONDS, SPADES, HEARTS, UNKNOWN_CARD_SUIT };
#define SUITMIN CLUBS
#define SUITMAX HEARTS
class CardSuit
{
public:
  CardSuit( cardsuit_t id );  // For nul type
  CardSuit( cardsuit_t id, char* name, char* xpmdata[] );
  cardsuit_t GetId() const { return m_id; }
  char* GetName() const { return m_name; }
  wxBitmap& GetBitmap() const { return (wxBitmap&)m_bitmap; }
  bool operator !=( const CardSuit& other ) { return m_id != other.GetId(); }
  bool operator ==( const CardSuit& other ) { return m_id == other.GetId(); }
  bool operator <( const CardSuit& other ) { return m_id < other.GetId(); }
  bool operator >( const CardSuit& other ) { return m_id > other.GetId(); }
private:
  cardsuit_t m_id;
  char* m_name;
  wxBitmap m_bitmap;
};

// Cards
class Card
{
public:
  Card( Deck *deck, CardType& type, CardSuit& suit, char* xpmdata[] );
  wxString NameStr();
  wxString ShortStr();
  bool GetTurned() { return m_turned; }
  void SetTurned( bool turned=true ) { m_turned = turned; }
  bool IsPlayable() const { return m_playable; }
  void SetPlayable( bool playable=true ) { m_playable = playable; }
  CardSuit& GetSuit() const { return (CardSuit&)m_suit; }
  CardType& GetType() const { return (CardType&)m_type; }
  bool HitTest( const wxPoint& pt ) const;
  bool Draw( wxDC& dc );
  wxPoint GetPosition() const { return m_pos; }
  void SetPosition( const wxPoint& pos ) { m_pos = pos; } 
  wxRect GetRect() const { return wxRect( m_pos.x, m_pos.y, m_bitmap.GetWidth(), m_bitmap.GetHeight() ); }
  wxBitmap& GetBitmap() const { return (wxBitmap&)m_bitmap; }
  void ColorInvert( bool inverted = true );
private:
  Deck *m_deck;
  CardType m_type;
  CardSuit m_suit;
  bool m_turned;
  bool m_playable;
  wxBitmap m_bitmap;
  wxPoint m_pos;
  wxRasterOperationMode blitop;
};

// List of Cards
WX_DECLARE_LIST( Card, CardList );
WX_DECLARE_STRING_HASH_MAP( Card*, CardMap );

// Decks
class Deck
{
public:
  Card* nulcard;
  Card* cards[40];
  CardMap cardmap;
  Deck();
  ~Deck();
  void Shuffle();
  wxBitmap& GetFace() const { return (wxBitmap&)m_face; }
private:
  wxBitmap m_face;
};

#endif  // _CARDS_HPP_

