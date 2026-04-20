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

#ifndef _LANG_HPP_
#define _LANG_HPP_ 1

// UI language configuration for sueca. The language catalogue (.mo
// files) is looked up under "mo/<canonical>/LC_MESSAGES/sueca.mo"
// relative to the binary as well as in the platform's standard
// translation directories.

// One entry per language offered in the preferences dropdown.
// 'code' is a wxLANGUAGE_* value (wxLANGUAGE_DEFAULT for "same as
// system"); 'native_name' is how the language refers to itself and is
// what the user sees in the dropdown.
struct LangEntry {
	int code;
	const char* native_name;
};

// Null-terminated (code=-2 sentinel) catalogue of supported
// languages. The single "same as system" meta-entry appears first;
// the remaining entries are listed alphabetically by native name.
extern const LangEntry kSuecaLanguages[];

#endif // _LANG_HPP_
