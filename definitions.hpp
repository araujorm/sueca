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

#ifndef _DEFINITIONS_HPP_
#define _DEFINITIONS_HPP_ 1

#define SUECA_VER "0.3"
#define SUECA_NAME "Sueca"
#define VERSION_STRING  SUECA_NAME " version " SUECA_VER
#define PLAYER_NAME_MAX 32

// Bot difficulty levels
enum botlevel_t { BOT_DUMB = 0, BOT_SMART, BOT_EXPERT, BOT_LEVEL_COUNT };

// Bitmask flags for enabled bot levels (one or more may be active)
#define BOT_FLAG( lvl )  ( 1 << ( lvl ) )
#define BOT_FLAGS_ALL    ( BOT_FLAG( BOT_DUMB ) | BOT_FLAG( BOT_SMART ) | BOT_FLAG( BOT_EXPERT ) )

// Card back designs
enum cardback_t { CARDBACK_BLUE = 0, CARDBACK_RED };

#endif  // _DEFINITIONS_HPP_
