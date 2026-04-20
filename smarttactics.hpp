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

#ifndef _SMARTTACTICS_HPP_
#define _SMARTTACTICS_HPP_ 1

#include "cards.hpp"

// Free-standing tactical helpers shared between SmartPlayer (rule-based
// decisions) and the simulator inside ExpertPlayer (Smart-style policy
// under PIMC). Intentionally kept out of the ObservingBot inheritance
// tree so MethodicPlayer - which is meant to stay a naive, textbook
// PIMC baseline - doesn't accidentally pick them up. Anything that
// belongs to any thinking bot (including Methodic) lives on
// ObservingBot; anything that defines "playing smart" lives here.
//
// Pure functions on card values: no state of their own, no coupling
// to bot class hierarchies. Callers pass the relevant pieces in.
namespace smarttactics {

// When partner is currently winning the trick, should we dump a high-
// value card on them? True when the win is safe against whoever else
// is still to play:
//   - we're the last to play, so no one else can capture it, or
//   - partner's current winning card is already a 7, an ace, or a
//     trump - hard for a remaining opponent to beat.
// False otherwise - then play conservatively to avoid handing points
// to an opponent who could still take the trick.
bool ShouldDumpHigh( Card* best, bool we_are_last, cardsuit_t trumphsuit );

} // namespace smarttactics

#endif // _SMARTTACTICS_HPP_
