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

#include "smarttactics.hpp"

namespace smarttactics {

bool ShouldDumpHigh( Card* best, bool we_are_last, cardsuit_t trumphsuit )
{
	return we_are_last ||
	       best->GetType().GetId() == ACE ||
	       best->GetType().GetId() == SEVEN ||
	       best->GetSuit().GetId() == trumphsuit;
}

// Same "beats" rule as the main bot code: trump beats non-trump, or
// higher same-suit. Duplicated locally so the array-shaped helpers
// don't need to reach into ObservingBot just for this check.
static bool beats( Card* card, Card* best, cardsuit_t trumphsuit )
{
	if( card->GetSuit().GetId() == trumphsuit &&
	    best->GetSuit().GetId() != trumphsuit )
		return true;
	if( card->GetSuit() == best->GetSuit() &&
	    card->GetType() > best->GetType() )
		return true;
	return false;
}

int LowestBeaterInSuit( Card** hand, int handsize, cardsuit_t suit,
                        Card* best, cardsuit_t trumphsuit )
{
	int idx = -1;
	for( int i = 0; i < handsize; i++ ) {
		if( hand[i]->GetSuit().GetId() != suit )
			continue;
		if( !beats( hand[i], best, trumphsuit ) )
			continue;
		if( idx < 0 || hand[i]->GetType() < hand[idx]->GetType() )
			idx = i;
	}
	return idx;
}

int LowestValueInSuit( Card** hand, int handsize, cardsuit_t suit )
{
	int idx = -1;
	for( int i = 0; i < handsize; i++ ) {
		if( hand[i]->GetSuit().GetId() != suit )
			continue;
		if( idx < 0 ||
		    hand[i]->GetType().GetValue() <
		      hand[idx]->GetType().GetValue() ||
		    ( hand[i]->GetType().GetValue() ==
		        hand[idx]->GetType().GetValue() &&
		      hand[i]->GetType() < hand[idx]->GetType() ) )
			idx = i;
	}
	return idx;
}

int HighestValueInSuit( Card** hand, int handsize, cardsuit_t suit )
{
	int idx = -1;
	for( int i = 0; i < handsize; i++ ) {
		if( hand[i]->GetSuit().GetId() != suit )
			continue;
		if( idx < 0 ||
		    hand[i]->GetType().GetValue() >
		      hand[idx]->GetType().GetValue() ||
		    ( hand[i]->GetType().GetValue() ==
		        hand[idx]->GetType().GetValue() &&
		      hand[i]->GetType() > hand[idx]->GetType() ) )
			idx = i;
	}
	return idx;
}

int LowestValueNonTrump( Card** hand, int handsize, cardsuit_t trumphsuit )
{
	int idx = -1;
	for( int i = 0; i < handsize; i++ ) {
		if( hand[i]->GetSuit().GetId() == trumphsuit )
			continue;
		if( idx < 0 ||
		    hand[i]->GetType().GetValue() <
		      hand[idx]->GetType().GetValue() ||
		    ( hand[i]->GetType().GetValue() ==
		        hand[idx]->GetType().GetValue() &&
		      hand[i]->GetType() < hand[idx]->GetType() ) )
			idx = i;
	}
	return idx;
}

int HighestValueNonTrump( Card** hand, int handsize, cardsuit_t trumphsuit )
{
	int idx = -1;
	for( int i = 0; i < handsize; i++ ) {
		if( hand[i]->GetSuit().GetId() == trumphsuit )
			continue;
		if( idx < 0 ||
		    hand[i]->GetType().GetValue() >
		      hand[idx]->GetType().GetValue() ||
		    ( hand[i]->GetType().GetValue() ==
		        hand[idx]->GetType().GetValue() &&
		      hand[i]->GetType() > hand[idx]->GetType() ) )
			idx = i;
	}
	return idx;
}

int LowestTypeInSuit( Card** hand, int handsize, cardsuit_t suit )
{
	int idx = -1;
	for( int i = 0; i < handsize; i++ ) {
		if( hand[i]->GetSuit().GetId() != suit )
			continue;
		if( idx < 0 || hand[i]->GetType() < hand[idx]->GetType() )
			idx = i;
	}
	return idx;
}

int LowestType( Card** hand, int handsize )
{
	int idx = -1;
	for( int i = 0; i < handsize; i++ ) {
		if( idx < 0 || hand[i]->GetType() < hand[idx]->GetType() )
			idx = i;
	}
	return idx;
}

} // namespace smarttactics
