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

#include "catalogloader.hpp"
#include "embedded_mo.hpp"
#include <wx/arrstr.h>
#include <wx/buffer.h>

wxMsgCatalog* SuecaCatalogLoader::LoadCatalog( const wxString& domain,
                                               const wxString& lang )
{
#ifndef SUECA_EMBED_ONLY
	// Dynamic builds let the filesystem win so distro packagers can
	// drop .mo files in /usr/share/locale/... and have them picked up.
	if( wxMsgCatalog* c =
	      wxFileTranslationsLoader::LoadCatalog( domain, lang ) )
		return c;
#endif
	for( int i = 0; kEmbeddedMos[i].locale; i++ ) {
		if( lang == wxString::FromUTF8( kEmbeddedMos[i].locale ) )
			return wxMsgCatalog::CreateFromData(
			  wxScopedCharBuffer::CreateNonOwned(
			    (const char*)kEmbeddedMos[i].data,
			    (size_t)kEmbeddedMos[i].len ),
			  domain );
	}
	return NULL;
}

wxArrayString SuecaCatalogLoader::GetAvailableTranslations(
  const wxString& domain ) const
{
#ifdef SUECA_EMBED_ONLY
	(void)domain;
	wxArrayString langs;
#else
	wxArrayString langs =
	  wxFileTranslationsLoader::GetAvailableTranslations( domain );
#endif
	for( int i = 0; kEmbeddedMos[i].locale; i++ ) {
		wxString l = wxString::FromUTF8( kEmbeddedMos[i].locale );
		if( langs.Index( l ) == wxNOT_FOUND )
			langs.Add( l );
	}
	return langs;
}
