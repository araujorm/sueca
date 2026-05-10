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

#ifndef _HELPDIALOG_HPP_
#define _HELPDIALOG_HPP_ 1

#include <wx/dialog.h>

// Modal dialog with the rules of sueca and a short note on how to
// drive the application. Displayed from the Help -> Contents menu
// entry. Text is held in a single _() string so it lands in
// po/sueca.pot; translators who care to localise the rules can pick
// it up from there.
class HelpDialog: public wxDialog
{
public:
	HelpDialog( wxWindow* parent );
};

#endif // _HELPDIALOG_HPP_
