#ifndef _PREFSDIALOG_HPP_
#define _PREFSDIALOG_HPP_ 1

// Forward declarations
class PrefsDialog;

#include <wx/dialog.h>
#include <wx/textctrl.h>
#include <wx/slider.h>

// Dialog with game options
class PrefsDialog: public wxDialog
{
public:
  PrefsDialog( wxWindow* parent );
private:
  wxTextCtrl* name_entry;
  wxSlider* delay_entry;
  void OnOk( wxCommandEvent& event );
  void Done( wxCommandEvent& event );
  DECLARE_EVENT_TABLE();
};

#endif // _PREFSDIALOG_HPP_

