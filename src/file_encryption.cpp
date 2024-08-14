#include "file_encryption.hpp"
#include "panel_file_manager.hpp"
#include "misc.hpp"

wxBEGIN_EVENT_TABLE(frameEncryption, wxFrame)
	EVT_BUTTON(EnumIDS::ID_FM_BTN_Random, frameEncryption::OnGenerarPass)
	EVT_BUTTON(EnumIDS::ID_FM_BTN_Crypt_Exec, frameEncryption::OnExecCrypt)
wxEND_EVENT_TABLE()

frameEncryption::frameEncryption(wxWindow* pParent, std::string strPath) :
	wxFrame(pParent, wxID_ANY, "Encriptar/Desencriptar Archivo", wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE) {
	this->p_strPath = strPath;
	
	wxArrayString selection;
	selection.Insert("Encriptar", 0);
	selection.Insert("Desencriptar", 1);
	//selection->Add("Encriptar");
	//selection->Add("Desencriptar");

	this->rdio_Options = new wxRadioBox(this, EnumIDS::ID_FM_Radio_Encriptar, wxEmptyString, wxDefaultPosition,
		wxDefaultSize, selection, 0, wxRA_SPECIFY_COLS);

	wxStaticText* lbl_File = new wxStaticText(this, wxID_ANY, "Archivo: " + this->p_strPath);
	wxStaticText* lbl_Pass = new wxStaticText(this, wxID_ANY, "Contraseña");
	wxButton* btn_Random_Pass = new wxButton(this, EnumIDS::ID_FM_BTN_Random, "Generar contraseña");
	wxButton* btn_Exec = new wxButton(this, EnumIDS::ID_FM_BTN_Crypt_Exec, ">>>");
	this->chk_del = new wxCheckBox(this, EnumIDS::ID_FM_Del_Check_Crypt, "Eliminar una vez sea cifrado/descifrado");

	this->txt_Pass = new wxTextCtrl(this, EnumIDS::ID_FM_Text_Password);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	sizer->Add(lbl_File, 0, wxEXPAND, 1);
	sizer->Add(this->rdio_Options, 0, wxEXPAND, 1);
	sizer->AddSpacer(10);
	sizer->Add(this->chk_del, 0, wxEXPAND, 1);
	sizer->AddSpacer(10);
	sizer->Add(lbl_Pass, 0, wxEXPAND, 1);
	sizer->Add(this->txt_Pass, 0, wxEXPAND, 1);
	sizer->AddSpacer(10);
	sizer->Add(btn_Random_Pass, 0, wxEXPAND, 1);
	sizer->AddSpacer(20);
	sizer->Add(btn_Exec, 0, wxEXPAND, 1);

	this->SetSizerAndFit(sizer);

	this->SetClientSize(400, 150);

}

void frameEncryption::Exec_SQL(const char* cCMD) {
	sqlite3* db;
	char* zErrMsg = 0;

	if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
		std::cout << "[DBCRYPT] No se pudo abrir la bd :" << sqlite3_errmsg(db) << std::endl;
		wxMessageBox("No se pudo abrir la base de datos");
		goto dbRelease;
	}

	if (sqlite3_exec(db, cCMD, NULL, 0, &zErrMsg) != SQLITE_OK) {
		wxMessageBox("Error: " + std::string(zErrMsg));
		goto dbRelease;
	}

dbRelease:

	sqlite3_free(zErrMsg);
	sqlite3_close(db);
}

void frameEncryption::OnGenerarPass(wxCommandEvent& event) {
	this->txt_Pass->SetValue(RandomPass(40));
}

void frameEncryption::OnExecCrypt(wxCommandEvent& event) {
	std::string strComando = (this->rdio_Options->GetSelection() == 0) ? "0" : "1";
	strComando += this->chk_del->IsChecked() ? "1" : "0";
	strComando.append(1, CMD_DEL);
	strComando += this->p_strPath;
	strComando.append(1, CMD_DEL);
	strComando += this->txt_Pass->GetValue();

	ListCtrlManager* list_parent = (ListCtrlManager*)this->GetParent();

	if (list_parent) {
		list_parent->itemp->EnviarComando(strComando, EnumComandos::FM_Crypt_Archivo);
		
		//Agregar a BD si es para cifrar
		if (this->rdio_Options->GetSelection() == 0) {
			time_t temp = time(0);
			struct tm* timeptr = localtime(&temp);

			std::string strFecha = std::to_string(timeptr->tm_mday);
			strFecha += "/" + std::to_string(timeptr->tm_mon);
			strFecha += "/" + std::to_string(timeptr->tm_year);
			std::string strCMD = "INSERT INTO keys (id, ip, nombre, fecha, pass) VALUES('";
			strCMD += list_parent->itemp->strID + "', '";
			strCMD += list_parent->itemp->strIP + "', '";
			strCMD += this->p_strPath + "', '";
			strCMD += strFecha + "', '";
			strCMD += this->txt_Pass->GetValue() + "');";

			this->Exec_SQL(strCMD.c_str());
		}
	}
}