#include "server.hpp"
#include "frame_listener.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(frameListeners, wxFrame)
	EVT_BUTTON(EnumIDSListeners::ID_GenerarPass, frameListeners::OnGenerarPass)
	EVT_BUTTON(EnumIDSListeners::ID_CrearListener, frameListeners::OnCrearListener)
	EVT_TEXT(EnumIDSListeners::ID_TXT_Puerto, frameListeners::OnInputPuerto)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ListCtrlManagerListeners, wxListCtrl)
	EVT_MENU(EnumIDSListeners::ID_CM_Refrescar, ListCtrlManagerListeners::OnRefrescar)
	EVT_MENU(EnumIDSListeners::ID_CM_Copiar, ListCtrlManagerListeners::OnCopiarPass)
	EVT_MENU(EnumIDSListeners::ID_CM_Eliminar, ListCtrlManagerListeners::OnEliminar)
	EVT_MENU(EnumIDSListeners::ID_CM_Habilitar, ListCtrlManagerListeners::OnToggle)
	EVT_MENU(EnumIDSListeners::ID_CM_Deshabilitar, ListCtrlManagerListeners::OnToggle)
	EVT_CONTEXT_MENU(ListCtrlManagerListeners::OnContextMenu)
wxEND_EVENT_TABLE()

frameListeners::frameListeners(wxWindow* pParent)
	:wxFrame(pParent, EnumIDSListeners::ID_Frame, "Listeners", wxDefaultPosition, wxSize(430, 500)) {

	const char* cQuery = "CREATE TABLE IF NOT EXISTS \"listeners\" (\
			\"nombre\"	TEXT,\
			\"clave_acceso\"	TEXT,\
			\"puerto\"	TEXT\
			);";

	this->Exec_SQL(cQuery);

	this->txtNombre = new wxTextCtrl(this, wxID_ANY, "Listener 1");
	this->txtPass = new wxTextCtrl(this, wxID_ANY, RandomPass(AES_KEY_LEN));
	this->txtPuerto = new wxTextCtrl(this, EnumIDSListeners::ID_TXT_Puerto, "65500");
	this->txtPuerto->SetMaxLength(5);

	this->txtPass->SetMaxLength(AES_KEY_LEN);
	this->txtPass->Enable(false);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->AddSpacer(10);
	main_sizer->Add(new wxStaticText(this, wxID_ANY, "Nombre: "), 0);
	main_sizer->Add(this->txtNombre, 0, wxALL | wxEXPAND);
	main_sizer->AddSpacer(5);

	wxBoxSizer* pass_sizer = new wxBoxSizer(wxHORIZONTAL);
	pass_sizer->Add(this->txtPass, 1, wxALL | wxEXPAND);
	pass_sizer->AddSpacer(5);
	pass_sizer->Add(new wxButton(this, EnumIDSListeners::ID_GenerarPass, "Generar"), 0);

	main_sizer->Add(new wxStaticText(this, wxID_ANY, "Clave de acceso: "), 0);
	main_sizer->Add(pass_sizer, 0, wxEXPAND);
	main_sizer->AddSpacer(5);

	main_sizer->Add(new wxStaticText(this, wxID_ANY, "Puerto de escucha "), 0);
	main_sizer->Add(this->txtPuerto, 0, wxALL | wxEXPAND);
	main_sizer->AddSpacer(5);

	main_sizer->Add(new wxButton(this, EnumIDSListeners::ID_CrearListener, "Crear listener"), 0, wxEXPAND);
	main_sizer->AddSpacer(10);

	this->CrearLista();
	this->list_ctrl->MostrarLista();

	main_sizer->Add(this->list_ctrl, 1, wxALL | wxEXPAND);

	this->SetSizer(main_sizer);
	this->SetSizeHints(this->GetSize(), this->GetSize());

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void frameListeners::OnCrearListener(wxCommandEvent& event) {
	std::string strNombre = this->txtNombre->GetValue();
	std::string strPass = this->txtPass->GetValue();
	std::string strPuerto = this->txtPuerto->GetValue();

	for (size_t i = 0; i < strNombre.length(); i++) {
		if (strNombre[i] == '\'' || strNombre[i] == '"') {
			wxMessageBox("Nombre del listener invalido");
			return;
		}
	}

	if (this->list_ctrl->isExiste(strNombre.c_str())) {
		wxMessageBox("El listener ya existe!");
		return;
	}

	if (strNombre != "") {
		if (strPass != "") {
			if (strPuerto != "") {
				//Registrar listener
				std::string strCMD = "INSERT INTO listeners (nombre, clave_acceso, puerto) VALUES ('";
				strCMD.append(strNombre);
				strCMD.append("', '");
				strCMD.append(strPass);
				strCMD.append("', '");
				strCMD.append(strPuerto);
				strCMD.append("');");

				//Agregar listener al vector de servidor
				int iPuerto = atoi(strPuerto.c_str());
				p_Servidor->m_AgregarListener(strNombre, iPuerto, strPass.c_str());

				this->Exec_SQL(strCMD.c_str());
				wxMessageBox("Listener agregado!");
				this->list_ctrl->MostrarLista();
			}else {
				wxMessageBox("El puerto de escucha no puede estar en blanco");
			}
		}else {
			wxMessageBox("La clave de acceso no puede estar en blanco");
		}
	}else {
		wxMessageBox("El nombre no puede estar en blanco");
	}
}

void frameListeners::OnGenerarPass(wxCommandEvent& event) {
	this->txtPass->SetValue(RandomPass(AES_KEY_LEN));
}

void frameListeners::OnInputPuerto(wxCommandEvent& event) {
	wxString strTemp = "";
	wxString strData = event.GetString();

	for (size_t i = 0; i < strData.size(); i++) {
		const char c = strData[i].GetValue();
		if (c >= 48 && c <= 57) {
			if (c == 48 && i == 0) {
				continue;
			}
			strTemp.append(c);
		}
	}
	//this->txtPuerto->SetValue(strTemp); envia EVT_TEXT nuevamente (loop infinito)
	this->txtPuerto->ChangeValue(strTemp);
	this->txtPuerto->SetInsertionPointEnd();

}

void frameListeners::Exec_SQL(const char* cCMD) {
	sqlite3* db;
	char* zErrMsg = 0;

	if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
		DEBUG_MSG("[DB] No se pudo abrir la bd");
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

void frameListeners::CrearLista() {
	this->list_ctrl = new ListCtrlManagerListeners(this, wxID_ANY, wxDefaultPosition , wxDefaultSize, wxBORDER_THEME | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);
	
	wxListItem itemCol;
	itemCol.SetText("Nombre");
	itemCol.SetWidth(80);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->list_ctrl->InsertColumn(0, itemCol);

	itemCol.SetText("Clave acceso");
	itemCol.SetAlign(wxLIST_FORMAT_LEFT);
	itemCol.SetWidth(150);
	this->list_ctrl->InsertColumn(1, itemCol);

	itemCol.SetText("Puerto");
	itemCol.SetWidth(80);
	this->list_ctrl->InsertColumn(2, itemCol);

	itemCol.SetText("Estado");
	itemCol.SetWidth(120);
	this->list_ctrl->InsertColumn(3, itemCol);

}

//Menu contextual
void ListCtrlManagerListeners::ShowContextMenu(const wxPoint& pos, bool isEmpty) {
	wxMenu menu;

	if (isEmpty) {
		menu.Append(EnumIDSListeners::ID_CM_Refrescar, "Refrescar");
	}else {
		menu.Append(EnumIDSListeners::ID_CM_Refrescar, "Refrescar");
		menu.AppendSeparator();
		menu.Append(EnumIDSListeners::ID_CM_Copiar, "Copiar PASS");

		if (this->GetItemText(this->iSelectedIndex, 3) == "Habilitado") {
			menu.Append(EnumIDSListeners::ID_CM_Deshabilitar, "Deshabilitar");
		}else {
			menu.Append(EnumIDSListeners::ID_CM_Habilitar, "Habilitar");
		}

		menu.Append(EnumIDSListeners::ID_CM_Eliminar, "Eliminar");
	}

	PopupMenu(&menu, pos.x, pos.y);
}

void ListCtrlManagerListeners::OnContextMenu(wxContextMenuEvent& event) {
	if (GetEditControl() == NULL)
	{
		wxPoint point = event.GetPosition();

		// If from keyboard
		if ((point.x == -1) && (point.y == -1))
		{
			wxSize size = GetSize();
			point.x = size.x / 2;
			point.y = size.y / 2;
		}
		else
		{
			point = ScreenToClient(point);
		}

		int flags;
		long iItem = HitTest(point, flags);

		this->iSelectedIndex = iItem;

		ShowContextMenu(point, iItem == -1 ? true : false);

	}
	else {
		event.Skip();
	}
}

static int static_callback(void* objClass, int argc, char** argv, char** azColName) {
	ListCtrlManagerListeners* self = static_cast<ListCtrlManagerListeners*>(objClass);
	return self->Callback(argc, argv, azColName);
}

int ListCtrlManagerListeners::Callback(int argc, char** argv, char** azColName) {
	std::string strNombre = "";
	int iPuerto = 0;
	std::string strPass = "";
	for (size_t i = 0; i < argc; i++) {
		if (strncmp(azColName[i], "nombre", 5) == 0) {
			this->InsertItem(this->iCount, wxString(argv[i]));
			strNombre = wxString(argv[i]).ToStdString();
		}else if (strncmp(azColName[i], "clave_acceso", 12) == 0) {
			this->SetItem(this->iCount, 1, wxString(argv[i]));
			strPass = wxString(argv[i]).ToStdString();
		}else if (strncmp(azColName[i], "puerto", 6) == 0) {
			this->SetItem(this->iCount, 2, wxString(argv[i]));
			iPuerto = atoi(wxString(argv[i]).ToStdString().c_str());
		}
	}

	this->SetItem(this->iCount, 3, "Deshabilitado");
	p_Servidor->m_AgregarListener(strNombre, iPuerto, strPass.c_str());
	this->iCount++;
	return 0;
}

void ListCtrlManagerListeners::MostrarLista() {
	this->DeleteAllItems();
	int icount = 0;
	//Obtener lista del vector
	std::vector<Listener_List_Data> vcData = p_Servidor->m_ListenerVectorCopy();
	if (vcData.size() > 0) {
		for (Listener_List_Data& ntemp : vcData) {
			this->InsertItem(icount, ntemp.nombre);
			this->SetItem(icount, 1, ntemp.clave_acceso);
			this->SetItem(icount, 2, ntemp.puerto);
			this->SetItem(icount++, 3, ntemp.estado);
		}
	}else {
		//Obtener lista de sqlite db
		sqlite3* db;
		this->iCount = 0;
		char* zErrMsg = 0;
		if (sqlite3_open(DB_FILE, &db) == SQLITE_OK) {
			sqlite3_exec(db, "SELECT * FROM listeners;", static_callback, this, &zErrMsg);
		}
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
	}
}

void ListCtrlManagerListeners::OnRefrescar(wxCommandEvent& event) {
	this->MostrarLista();
	return;
}

void ListCtrlManagerListeners::OnEliminar(wxCommandEvent& event) {
	if (wxMessageBox("Seguro que quieres eliminar el listener?", "Eliminar", wxYES_NO) == wxNO) {
		return;
	}

	std::string strNombre = this->GetItemText(this->iSelectedIndex, 0);
	std::string strPass = this->GetItemText(this->iSelectedIndex, 1);

	std::string strCMD = "DELETE FROM listeners WHERE nombre = '";
	strCMD.append(strNombre);
	strCMD.append("' AND clave_acceso = '");
	strCMD.append(strPass);
	strCMD.append("';");

	p_Servidor->m_BorrarListener(strNombre);

	frameListeners* frm = static_cast<frameListeners*>(this->GetParent());
	frm->Exec_SQL(strCMD.c_str());

	this->MostrarLista();
}

void ListCtrlManagerListeners::OnCopiarPass(wxCommandEvent& event) {
	std::string strPass = this->GetItemText(this->iSelectedIndex, 1);
	if (!OpenClipboard(nullptr)) {
		std::cerr << "Error al abrir el portapapeles" << std::endl;
		return;
	}

	if (!EmptyClipboard()) {
		std::cerr << "Error al limpiar el portapapeles" << std::endl;
		CloseClipboard();
		return;
	}

	HGLOBAL hClipboardData;

	hClipboardData = GlobalAlloc(GMEM_MOVEABLE, strPass.size() + 1);
	if (hClipboardData == nullptr) {
		std::cerr << "Error al asignar memoria global" << std::endl;
		CloseClipboard();
		return;
	}
	char* pszBuffer = static_cast<char*>(GlobalLock(hClipboardData));
	if (pszBuffer == nullptr) {
		std::cerr << "Error al bloquear la memoria global" << std::endl;
		GlobalFree(hClipboardData);
		CloseClipboard();
		return;
	}
	strncpy(pszBuffer, strPass.c_str(), strPass.size());
	GlobalUnlock(hClipboardData);
	SetClipboardData(CF_TEXT, hClipboardData);

	CloseClipboard();
}

void ListCtrlManagerListeners::OnToggle(wxCommandEvent& event) {
	int id = event.GetId();

	if (id == EnumIDSListeners::ID_CM_Habilitar) {

		//Iniciar listener
		this->SetItem(this->iSelectedIndex, 3, "Habilitado");
	}else if (id == EnumIDSListeners::ID_CM_Deshabilitar) {

		//Detener listener
		this->SetItem(this->iSelectedIndex, 3, "Deshabilitado");
	}

	p_Servidor->m_ToggleListener(this->GetItemText(this->iSelectedIndex, 0).ToStdString());

	return;
}

bool ListCtrlManagerListeners::isExiste(const char* _cnombre) {
	for (int index = 0; index < this->GetItemCount(); index++) {
		if (this->GetItemText(index, 0) == _cnombre) {
			return true;
		}
	}
	return false;
}