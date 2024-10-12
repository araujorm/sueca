/*
sueca - An implementation of the Portuguese game "Sueca" in C++ and wxWidgets
Copyright (C) 2003-2024 Rodrigo Araujo

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

#ifndef _MYEVENTS_HPP_
#define _MYEVENTS_HPP_ 1

#include <wx/event.h>

BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_LOCAL_EVENT_TYPE( CARD_MOVE_EVT_TYPE, 0 )
  DECLARE_LOCAL_EVENT_TYPE( CHAT_PANEL_MESSAGE_TYPE, 1 )
  DECLARE_LOCAL_EVENT_TYPE( FINISH_REMOTE_HANDLER_TYPE, 2 )
END_DECLARE_EVENT_TYPES()

#endif // _MYEVENTS_HPP_
