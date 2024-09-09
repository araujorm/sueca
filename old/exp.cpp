#include <cmath>
#include "mycanvas.hpp"
#include <wx/gdicmn.h>
#include "main.hpp"

BEGIN_DECLARE_EVENT_TYPES()
  DEFINE_EVENT_TYPE( CARD_MOVE_EVT )
END_DECLARE_EVENT_TYPES()

unsigned int CardMoveEvent::inst = 0;
unsigned int CardMoveEvent::moved = 0;

CardMoveEvent::CardMoveEvent( Card *crd, wxPoint& i_destpos, Game* ownr ):
  wxEvent( 0, CARD_MOVE_EVT ), card( crd ), owner( ownr )
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
  return new CardMoveEvent(*this);
}

NameLabel::NameLabel( Player* player, wxDC& dc ):
  m_text( player->GetName() ), m_visible( true )
{
  wxCoord w; wxCoord h;
  dc.GetTextExtent( m_text, &w, &h );
  m_area.SetWidth( w );
  m_area.SetHeight( h );
  m_pos = player->GetNamePos( wxSize( w, h ) );
  m_area.SetX( m_pos.x );
  m_area.SetY( m_pos.y );
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
    m_canvas->RefreshRect( card->GetRect() );
    m_canvas->Update();
  }
}

// MyCanvas
BEGIN_EVENT_TABLE( MyCanvas, wxPanel )
  EVT_PAINT( MyCanvas::OnPaint )
  EVT_ERASE_BACKGROUND( MyCanvas::OnEraseBackground )  // Prevent flickering
  EVT_MOUSE_EVENTS( MyCanvas::OnMouseEvent )
  // Card move events
  EVT_CUSTOM( CARD_MOVE_EVT, 0, MyCanvas::OnCardMoveEvent )
END_EVENT_TABLE()

MyCanvas::MyCanvas( wxFrame* parent, wxWindowID id ):
  wxPanel( parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER ),
  flashing( NULL ), statusbar( parent->GetStatusBar() ), m_localplayer( NULL ),
  m_trumph( NULL ), m_buffer( MC_X_SIZE, MC_Y_SIZE ), fltimer( this )
{
  for( int i = 0; i < 4; i++ )
    m_names[i] = NULL;
  SetBackgroundColour( wxColour( 0, 128, 0 ) );
  Clear();
  SetCursor( wxCursor( wxCURSOR_ARROW ) );
  SetClientSize( MC_X_SIZE, MC_Y_SIZE );
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
  PrepareDC( destdc );
  wxMemoryDC m_dc;
  m_dc.SelectObject( m_buffer );
  m_dc.SetBackground( wxBrush( GetBackgroundColour(), wxSOLID ) );

  // Update all regions at once
  wxRegion upd;
  wxRegionIterator updi( GetUpdateRegion() );
  while( updi ) {
    upd.Union( updi.GetRect() );
    updi++;
  }
  wxRect updrect = upd.GetBox();
  m_dc.BeginDrawing();
  DrawShapes( m_dc, updrect );
  destdc.Blit( updrect.GetX(), updrect.GetY(),
	       updrect.GetWidth(), updrect.GetHeight(),
	       & m_dc, updrect.GetX(), updrect.GetY() );
  m_dc.EndDrawing();
}

void MyCanvas::OnMouseEvent( wxMouseEvent& event )
{
  static int pos;
  static Card* raised = NULL;

  if( event.LeftDown() && m_localplayer ) {
    Card *playcard = FindCard( event.GetPosition(), &pos );
    if( playcard && playcard->IsPlayable() )
      switch( wxGetApp().GetGame()->PlayMove( m_localplayer, playcard ) ) {
      case MOVE_TURN:
        statusbar->SetStatusText( "It's not your turn!" );
        break;
      case MOVE_INVALID:
        statusbar->SetStatusText( "Invalid move!" );
        FlashCard( playcard );
        break;
      case MOVE_OK:
        m_localplayer->Remove( playcard );
        break;
      }
  }
  if( event.RightDown() ) {
    // Neat efect of raising a playable card
    raised = FindCard( event.GetPosition(), &pos );
    if( raised ) {
      if ( raised->GetTurned() || !raised->IsPlayable() )
        raised = NULL;
      else {
        CaptureMouse();
        m_displayList.DeleteObject( raised );
        m_displayList.Insert( raised );
        RefreshRect( raised->GetRect() );
      }
    }
  }
  else if( event.RightUp() && raised ) {
    // Put the card back on its stacking position
    ReleaseMouse();
    m_displayList.DeleteObject( raised );
    m_displayList.Insert( pos, raised );
    RefreshRect( raised->GetRect() );
    raised = NULL;
  }
  else if( event.Moving() && !event.RightIsDown() ) {
/*    statusbar->SetStatusText( wxString::Format( "(%d,%d)", event.GetPosition().x, event.GetPosition().y ) );
    return;*/
    // Display visible card names on status bar
    Card* under = FindCard( event.GetPosition(), &pos );
    if( under ) {
      if( !under->GetTurned() )
        statusbar->SetStatusText( under->NameStr() );
    }
    else if( m_trumph && m_trumph->GetRect().Inside( event.GetPosition() ) )
      statusbar->SetStatusText( "Trumph: " + m_trumph->GetCardName() );
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
    if( card->GetRect().Intersects( area ) ) {
      wxRegion visible( card->GetRect() );
      CardList::Node* topnode = node->GetPrevious();
      while( topnode ) {
	visible.Subtract( card->GetRect() );
	topnode = topnode->GetPrevious();
      }
      dc.SetClippingRegion( visible );
      card->Draw( dc );
    }
    node = node->GetPrevious();
  }
  dc.DestroyClippingRegion();
  dc.SetClippingRegion( area );
  // Player Name Labels
  for ( int i = 0; i < 4; i++ ) {
    NameLabel* nl;
    if ( ( nl = m_names[i] ) && nl->GetRect().Intersects( area ) )
      nl->Draw( dc );
  }
  // Trumph Label
  if ( m_trumph && m_trumph->GetRect().Intersects( area ) )
    m_trumph->Draw( dc );
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
    wxGetApp().GetFrame()->viewMenu->Enable( ID_VIEW_TRUMPH, false );
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
  RefreshRect( crd->GetRect() );
}

void MyCanvas::Remove( Card* crd )
{
  wxRect updrect = crd->GetRect();
  m_displayList.DeleteObject( crd );
  RefreshRect( updrect );
}

void MyCanvas::SetNameLabel( int playerno, Player* player )
{
  if( m_names[playerno] ) {
    wxRect updrect = m_names[playerno]->GetRect();
    delete m_names[playerno];
    m_names[playerno] = NULL;
    RefreshRect( updrect );
  }
  wxClientDC dc( this );
  m_names[playerno] = new NameLabel( player, dc );
  RefreshRect( m_names[playerno]->GetRect() );
}

void MyCanvas::SetTrumphLabel( Player* player, Card* card )
{
  if( m_trumph ) {
    wxRect updrect = m_trumph->GetRect();
    delete m_trumph;
    m_trumph = NULL;
    RefreshRect( updrect );
  }
  else
    wxGetApp().GetFrame()->viewMenu->Enable( ID_VIEW_TRUMPH, true );
  wxClientDC dc( this );
  m_trumph = new TrumphLabel( player, card, dc );
  RefreshRect( m_trumph->GetRect() );
}

#define STOP_PREC 5e-1
void MyCanvas::OnCardMoveEvent( CardMoveEvent* event )
{
  // Ignore events concerning no longer existing games
  if( wxGetApp().GetGame() != event->owner )
    return;
  double diffx = event->destpos.x - event->pos.x;
  double diffy = event->destpos.y - event->pos.y;
  if( diffx*diffx + diffy*diffy <= STOP_PREC ) {
    wxGetApp().GetGame()->CardMoved( event->card );
    return;
  }
  wxRegion update_reg;
  update_reg.Union( event->card->GetRect() );
  wxPoint oldpos = event->card->GetPosition();
  event->pos.x += event->sine;
  event->pos.y += event->cossine;
  wxPoint newpos( (wxCoord)event->pos.x, (wxCoord)event->pos.y );
  // Update as necessary
  if( oldpos != newpos ) {
    event->card->SetPosition( newpos );
    update_reg.Union( event->card->GetRect() );
    RefreshRect( update_reg.GetBox() );
    // Update all moving cards at once
    if( ++event->moved >= event->inst ) {
      Update();
      event->moved = 0;
      for( unsigned int i = 0; i < 50000; i++ );
    }
  }
  AddPendingEvent( *event );
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
      RefreshRect( card->GetRect() );
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

