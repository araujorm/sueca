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

#include "lang.hpp"
#include <wx/intl.h>

// "Same as system" is shown first as a meta-option; English is the
// source language and is always available without a catalogue. Every
// other entry must have a matching po/<locale>.po committed - add the
// entry here in the same commit that introduces the translation, in
// alphabetical order by native name. The native-name labels are
// intentionally NOT marked for translation: each language labels
// itself in its own script, so the user always sees the list in
// whatever language is native to each entry. Only the "Same as
// system" label is translated, via _() at display time in the
// preferences dialog.
const LangEntry kSuecaLanguages[] = {
	{ wxLANGUAGE_DEFAULT,             "Same as system" },
	{ wxLANGUAGE_GERMAN,              "Deutsch" },
	{ wxLANGUAGE_ENGLISH,             "English" },
	{ wxLANGUAGE_SPANISH,             "Español (España)" },
	{ wxLANGUAGE_SPANISH_LATIN_AMERICA, "Español (Latinoamérica)" },
	{ wxLANGUAGE_FRENCH,              "Français" },
	{ wxLANGUAGE_ITALIAN,             "Italiano" },
	{ wxLANGUAGE_DUTCH,               "Nederlands" },
	{ wxLANGUAGE_POLISH,              "Polski" },
	{ wxLANGUAGE_PORTUGUESE_PORTUGAL, "Português (Portugal)" },
	{ -2, 0 }  // sentinel
};
