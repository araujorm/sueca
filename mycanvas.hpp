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

#ifndef _CANVAS_HPP_
#define _CANVAS_HPP_ 1

// Forward declarations
class CardMoveEvent;
class NameLabel;
class TrumphLabel;
class CardFlashTimer;
class MyCanvas;

#ifndef MC_X_SIZE
#define MC_X_SIZE 450
#endif
#ifndef MC_Y_SIZE
#define MC_Y_SIZE 450
#endif

#include <wx/dcclient.h>
#include <wx/panel.h>
#include <wx/frame.h>
#include <wx/dcmemory.h>
#include "cards.hpp"
#include "player.hpp"
#include "game.hpp"

class CardMoveEvent: public wxEvent
{
public:
  static unsigned int skipped;
  static unsigned int inst;
  static unsigned int moved;
  Card* card;
  Game* owner;
  wxRealPoint destpos;
  wxRealPoint pos;
  double sine;
  double cossine;

  CardMoveEvent( Card *crd, wxPoint& i_destpos, Game* ownr );
  ~CardMoveEvent() { inst--; }
  wxEvent *Clone(void) const;
};

enum { P1_NAME = 0, P2_NAME, P3_NAME, P4_NAME };

class NameLabel
{
public:
  NameLabel( Player* player, wxDC& dc );
  wxPoint GetPos() const { return m_pos; }
  void SetPos( wxPoint& pos ) { m_pos = pos; }
  wxString& GetText() const { return (wxString&)m_text; }
  wxRect GetRect() const { return m_area; }
  bool GetVisible() const { return m_visible; }
  void Show() { m_visible = true; }
  void Hide() { m_visible = false; }
  void Draw( wxDC& dc );
private:
  wxString m_text;
  wxPoint m_pos;
  wxRect m_area;
  bool m_visible;
};

class TrumphLabel
{
public:
  TrumphLabel( Player* player, Card* card, wxDC& dc );
  wxPoint GetPos() const { return m_pos; }
  void SetPos( wxPoint& pos ) { m_pos = pos; }
  wxRect GetRect() const { return m_area; }
  bool GetVisible() const { return m_visible; }
  void Show() { m_visible = true; }
  void Hide() { m_visible = false; }
  void Draw( wxDC& dc );
  wxString GetCardName() const { return m_card->NameStr(); }
private:
  Card* m_card;
  wxString m_text;
  wxBitmap m_bmp;
  wxPoint m_pos;
  wxPoint m_textpos;
  wxPoint m_bmppos;
  wxRect m_area;
  bool m_visible;
};

// Timer to wait before restoring a "flashing" (color inverted) card
class CardFlashTimer: public wxTimer
{
public:
  CardFlashTimer( MyCanvas* canvas );
  void Notify();
private:
  MyCanvas* m_canvas;
};

class MyCanvas: public wxPanel
{
public:
  Card* flashing;

  MyCanvas( wxFrame* parent, wxWindowID );
  ~MyCanvas();
  void OnPaint( wxPaintEvent& event );
  void OnEraseBackground( wxEraseEvent& event ) {}
  void OnMouseEvent( wxMouseEvent& event );
  void DrawShapes( wxDC& dc, const wxRect& region );
  // Don't free the cards as they belong to a Deck
  void ClearCards() { m_displayList.Clear(); }
  void ClearNames();
  void ClearTrumph();
  Card* FindCard( const wxPoint& pt, int* pos = NULL ) const;
  CardList& GetDisplayList() { return m_displayList; }
  void Add( Card* crd, int x, int y, bool turned = false );
  void Remove( Card* crd, bool update = true );
  void SetLocalPlayer( LocalPlayer *player ) { m_localplayer = player; }
  LocalPlayer* GetLocalPlayer() { return m_localplayer; }
  void SetNameLabel( int playerno, Player* player );
  void SetTrumphLabel( Player* player, Card* card );
  void OnCardMoveEvent( CardMoveEvent& event );
  void MoveCardTo( Card* card, wxPoint destpos, bool raise = true );
  void FlashCard( Card* card );
  void NotTurnWarning();
  void InvalidLocalMove( Card* card = NULL );
private:
  wxStatusBar* statusbar;
  CardList m_displayList;
  LocalPlayer* m_localplayer;
  NameLabel *m_names[4];
  TrumphLabel *m_trumph;
  wxBitmap m_buffer;
  CardFlashTimer fltimer;
  Card* lastclicked;
  DECLARE_EVENT_TABLE()
};

#endif  // _CANVAS_HPP_

