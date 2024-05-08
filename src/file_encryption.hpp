#ifndef ___CRYPT_
#define ___CRYPT_

#include "headers.hpp"

class frameEncryption : public wxFrame {
	public:
		frameEncryption(wxWindow* pParent, std::string strPath);

		void OnGenerarPass(wxCommandEvent& event);
		void OnExecCrypt(wxCommandEvent& event);

		
	private:
		std::string p_strPath = "";
		wxTextCtrl* txt_Pass = nullptr;
		wxRadioBox* rdio_Options = nullptr;
		wxCheckBox* chk_del = nullptr;

		wxDECLARE_EVENT_TABLE();
};

#endif // !___CRYPT
