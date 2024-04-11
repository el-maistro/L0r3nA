#ifndef __FILE_EDITOR
#define __FILE_EDITOR

#include "headers.hpp"

class wxEditForm : public wxFrame {
	public:
		wxEditForm(wxWindow* pParent, wxString strNombre);
	private:
		wxTextCtrl* p_txtEditor = nullptr;
};

#endif