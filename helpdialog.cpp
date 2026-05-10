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

#include "helpdialog.hpp"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/html/htmlwin.h>

HelpDialog::HelpDialog( wxWindow* parent )
  : wxDialog( parent, wxID_ANY, _( "Sueca - Rules" ),
              wxDefaultPosition, wxSize( 660, 580 ),
              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
	wxBoxSizer* sizer = new wxBoxSizer( wxVERTICAL );

	wxHtmlWindow* html =
	  new wxHtmlWindow( this, wxID_ANY, wxDefaultPosition,
	                    wxDefaultSize, wxHW_SCROLLBAR_AUTO );
	html->SetPage( _(
"<html><body>"
"<h2>Sueca</h2>"
"<p>Sueca is a traditional Portuguese trick-taking card game for "
"four players in two teams of two. Partners sit opposite each "
"other; the seating order around the table is therefore "
"<i>A - B - A - B</i>.</p>"

"<h3>The deck</h3>"
"<p>40 cards in four suits (Clubs, Diamonds, Hearts, Spades). "
"Each suit has ten ranks; the 8s, 9s and 10s are not used.</p>"

"<h3>Card values</h3>"
"<table border=\"0\" cellpadding=\"3\" cellspacing=\"0\">"
"<tr><td><b>Ace</b></td><td>&nbsp;&nbsp;</td><td>11 points</td></tr>"
"<tr><td><b>Seven</b></td><td></td><td>10 points</td></tr>"
"<tr><td><b>King</b></td><td></td><td>4 points</td></tr>"
"<tr><td><b>Jack</b></td><td></td><td>3 points</td></tr>"
"<tr><td><b>Queen</b></td><td></td><td>2 points</td></tr>"
"<tr><td>6, 5, 4, 3, 2</td><td></td><td>0 points</td></tr>"
"</table>"
"<p><i>Total: 120 points per round.</i></p>"

"<h3>Card ranking within a trick</h3>"
"<p>Ace &gt; Seven &gt; King &gt; Jack &gt; Queen &gt; "
"6 &gt; 5 &gt; 4 &gt; 3 &gt; 2</p>"
"<p>A trump card always beats a non-trump card.</p>"

"<h3>The deal</h3>"
"<p>Each player is dealt 10 cards. The dealer's last card is shown "
"to all players and its suit becomes the trump suit "
"(<i>trunfo</i>) for the round. The dealer keeps that card.</p>"

"<h3>Playing</h3>"
"<p>The player to the dealer's right leads the first trick. "
"Players <b>must</b> follow the led suit if they can; otherwise "
"they may play any card, including a trump.</p>"
"<p>The trick is won by the highest trump played, or, if no trump "
"was played, by the highest card of the led suit. The winner of a "
"trick leads the next one.</p>"

"<h3>Scoring (per round)</h3>"
"<ul>"
"<li><b>61 - 89</b> captured points: round won (1 game point)</li>"
"<li><b>90 - 119</b> captured points: <i>vaza</i> (2 game points)</li>"
"<li><b>120</b> captured points: <i>bandeira</i> (4 game points)</li>"
"<li><b>60 - 60</b>: tie (no points awarded)</li>"
"</ul>"

"<h3>Winning the match</h3>"
"<p>A match is typically won by the first team to reach 4 game "
"points. Successive rounds are played until a team reaches that "
"total.</p>"

"<h3>Using this application</h3>"
"<p>Click a card in your hand to play it. The application warns "
"you if the move is invalid. Use the <b>View</b> menu to show or "
"hide the score, trump-card and last-trick dialogs at any time. "
"Game and player preferences (name, bot difficulty, language, "
"card back, animation speed) are under <b>Game &rarr; Preferences</b>.</p>"
"</body></html>"
	) );

	sizer->Add( html, 1, wxEXPAND | wxALL, 5 );

	wxButton* close = new wxButton( this, wxID_OK, _( "Close" ) );
	sizer->Add( close, 0, wxALIGN_CENTER | wxBOTTOM, 5 );

	SetSizer( sizer );
	close->SetFocus();
}
