#include "frame_crypt_db.hpp"

wxBEGIN_EVENT_TABLE(ListCtrlManager3, wxListCtrl)
	EVT_MENU(EnumIDS::ID_CryptDB_Refrescar, ListCtrlManager3::OnRefrescar)
	EVT_MENU(EnumIDS::ID_CryptDB_Copiar, ListCtrlManager3::OnCopiarPass)
	EVT_MENU(EnumIDS::ID_CryptDB_Eliminar, ListCtrlManager3::OnEliminar)
	EVT_CONTEXT_MENU(ListCtrlManager3::OnContextMenu)
wxEND_EVENT_TABLE()

void ListCtrlManager3::OnRefrescar(wxCommandEvent& event) {
	frameCryptDB* frame = (frameCryptDB*)this->GetParent();
	frame->ObtenerData();
}

void ListCtrlManager3::OnCopiarPass(wxCommandEvent& event) {
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string strPass = this->GetItemText(item, 4);
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
	strcpy(pszBuffer, strPass.c_str());
	GlobalUnlock(hClipboardData);
	SetClipboardData(CF_TEXT, hClipboardData);

	CloseClipboard();
}

void ListCtrlManager3::OnEliminar(wxCommandEvent& event) {
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string strCMD = "DELETE FROM keys WHERE id = '";
	strCMD += this->GetItemText(item, 0);
	strCMD += "' AND nombre = '";
	strCMD += this->GetItemText(item, 2);
	strCMD += "' AND pass = '";
	strCMD += this->GetItemText(item, 4);
	strCMD += "';";

	char* zErrMsg = 0;
	frameCryptDB* frame = (frameCryptDB*)this->GetParent();

	frame->Exec_SQL(strCMD.c_str());

	frame->ObtenerData();
}

void ListCtrlManager3::ShowContextMenu(const wxPoint& pos, bool isEmpty) {
	wxMenu menu;

	if (isEmpty) {
		menu.Append(EnumIDS::ID_CryptDB_Refrescar, "Refrescar");
	}else {
		menu.Append(EnumIDS::ID_CryptDB_Refrescar, "Refrescar");
		menu.AppendSeparator();
		menu.Append(EnumIDS::ID_CryptDB_Copiar, "Copiar PASS");
		menu.Append(EnumIDS::ID_CryptDB_Eliminar, "Eliminar record");
	}

	PopupMenu(&menu, pos.x, pos.y);
}

void ListCtrlManager3::OnContextMenu(wxContextMenuEvent& event) {
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

		ShowContextMenu(point, iItem == -1 ? true : false);

	}
	else {
		event.Skip();
	}
}

frameCryptDB::frameCryptDB():
	wxFrame(nullptr, wxID_ANY, "Crypt DB"){

	
	const char* cQuery = "CREATE TABLE IF NOT EXISTS \"keys\" (\
			\"id\"	TEXT,\
			\"ip\"	TEXT,\
			\"nombre\"	TEXT,\
			\"fecha\"	TEXT,\
			\"pass\"	TEXT\
			);";
	
	this->Exec_SQL(cQuery);

	this->p_listctrl = new ListCtrlManager3(this, wxID_ANY, wxDefaultPosition, wxSize(600, 400), wxBORDER_THEME | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);
	
	wxListItem itemCol;
	itemCol.SetText("Id");
	itemCol.SetWidth(70);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->p_listctrl->InsertColumn(0, itemCol);

	itemCol.SetText("Ip");
	itemCol.SetAlign(wxLIST_FORMAT_LEFT);
	itemCol.SetWidth(100);
	this->p_listctrl->InsertColumn(1, itemCol);

	itemCol.SetText("Nombre");
	itemCol.SetWidth(120);
	this->p_listctrl->InsertColumn(2, itemCol);

	itemCol.SetText("Fecha");
	itemCol.SetWidth(100);
	this->p_listctrl->InsertColumn(3, itemCol);

	itemCol.SetText("Pass");
	itemCol.SetWidth(250);
	this->p_listctrl->InsertColumn(4, itemCol);

	this->ObtenerData();

	wxBoxSizer* nsizer = new wxBoxSizer(wxHORIZONTAL);
	nsizer->Add(this->p_listctrl, 1, wxEXPAND, 1);

	this->SetSizerAndFit(nsizer);
}

void frameCryptDB::Exec_SQL(const char* cCMD) {
	sqlite3* db;
	char* zErrMsg = 0;

	if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
		DEBUG_MSG("[DBCRYPT] No se pudo abrir la bd");
		DEBUG_MSG(sqlite3_errmsg(db));
		wxMessageBox("No se pudo abrir la base de datos");
		sqlite3_close(db);
		return;
	}

	if (sqlite3_exec(db, cCMD, NULL, 0, &zErrMsg) != SQLITE_OK) {
		wxMessageBox("Error: " + std::string(zErrMsg));
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return;
	}

	sqlite3_free(zErrMsg);
	sqlite3_close(db);
}
