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

#ifndef _CATALOGLOADER_HPP_
#define _CATALOGLOADER_HPP_ 1

#include <wx/translation.h>

// Loader that serves message catalogues (.mo) baked into the binary
// via xxd in the Makerules. When built without SUECA_EMBED_ONLY, the
// filesystem is consulted first so packaged installs (rpm/deb) can
// drop .mo files under the usual locations and have them override
// the embedded ones. When built with SUECA_EMBED_ONLY (static
// releases), only the embedded catalogues are visible.
class SuecaCatalogLoader: public wxFileTranslationsLoader
{
public:
	wxMsgCatalog* LoadCatalog( const wxString& domain,
	                           const wxString& lang ) override;
	wxArrayString GetAvailableTranslations( const wxString& domain )
	  const override;
};

#endif // _CATALOGLOADER_HPP_
