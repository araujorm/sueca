#ifndef _SERVERDIALOG_HPP_
#define _SERVERDIALOG_HPP_ 1

// Forward declarations
class ServerDialog;

#include <wx/list.h>
#include <wx/sizer.h>
#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/hashmap.h>
#include <wx/stattext.h>
#include "chatpanel.hpp"
#include "serverhandler.hpp"
#include "netserverplayer.hpp"

class Position
{
public:
  NetServerPlayer* player;
  wxStaticText* label;
  GamePos* gpos;
  Position( wxStaticText* initlabel, GamePos* initgpos ):
    player( NULL ), label( initlabel ), gpos( initgpos ) {}
};

WX_DECLARE_STRING_HASH_MAP( Position*, PosMap );

// Server creation dialog
class ServerDialog: public wxDialog
{
public:
  ServerDialog( wxWindow* parent );
  void ReLayout();
protected:
  friend class ServerHandler;
  PosMap positions;
  ChatPanel* chat;
  wxWindowList disconnectedlist;
  wxWindowList connectedlist;
  Position* GetPlayerPosition( const Player* player );
  void SetPosition( Position* pos, NetServerPlayer* pl );
private:
  bool ip_enabled;
  wxBoxSizer* main_sizer;
  wxTextCtrl* ip_entry;
  wxTextCtrl* port_entry;
  wxButton* ok_button;
  wxButton* listen_button;
  void OnIpEnabled( wxCommandEvent& event );
  void EndListen();
  void OnListen( wxCommandEvent& event );
  Player* GetPlayerForPos( wxString posname );
  void OnBegin( wxCommandEvent& event );
  void OnCancel( wxCommandEvent& event ) { Done(); }
  void OnClose( wxCloseEvent& event ) { Done(); }
  void Done( bool cancel = true );
  void OnChatMessage( ChatPanelMsgEvt& event );
  Position* GetNotMatchingPos( wxString& posstr, Player* player );
  DECLARE_EVENT_TABLE();
};

#endif // _SERVERDIALOG_HPP_
