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
	EVT_MENU(EnumMenuFM::ID_Crypt, ListCtrlManager::OnEncriptarArchivo)
	EVT_CONTEXT_MENU(ListCtrlManager::OnContextMenu)
	EVT_LIST_ITEM_ACTIVATED(EnumIDS::ID_Panel_FM_List, ListCtrlManager::OnActivated)
wxEND_EVENT_TABLE()

panelFileManager::panelFileManager(wxWindow* pParent) :
	wxPanel(pParent, EnumIDS::ID_Panel_FM) {
	this->SetBackgroundColour(wxColor(200, 200, 200));
	
	wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
	if (wxTree) {
		wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
		if (panel_cliente) {
			FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
			if (frame_cliente) {
				this->sckCliente = frame_cliente->sckCliente;
				this->strID = frame_cliente->strClienteID;
				this->strIP = frame_cliente->strIP;
			}
		}
	}

	this->p_ToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_LEFT);
	wxImage::AddHandler(new wxPNGHandler);

	wxBitmap pcBitmap(wxT(".\\imgs\\computer.png"), wxBITMAP_TYPE_PNG);
	wxBitmap desktopBitmap(wxT(".\\imgs\\desktop.png"), wxBITMAP_TYPE_PNG);
	wxBitmap downloadBitmap(wxT(".\\imgs\\download.png"), wxBITMAP_TYPE_PNG);
	wxBitmap refreshBitmap(wxT(".\\imgs\\refresh.png"), wxBITMAP_TYPE_PNG);
	wxBitmap uploadBitmap(wxT(".\\imgs\\upload.png"), wxBITMAP_TYPE_PNG);


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
	sizer->Add(this->p_ToolBar, 0, wxEXPAND);
	
	wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
	sizer2->Add(this->listManager, 0, wxEXPAND, 1);
	sizer2->Add(this->p_RutaActual, 1, wxEXPAND, 1);

	sizer->Add(sizer2);
	SetSizer(sizer);

}

void panelFileManager::OnToolBarClick(wxCommandEvent& event) {
	wxListItem itemCol;
	std::string strComando = "";
	switch (event.GetId()) {
		case EnumIDS::ID_Panel_FM_Equipo:
			this->listManager->ClearAll();
			
			itemCol.SetText("-");
			itemCol.SetWidth(20);
			itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
			this->listManager->InsertColumn(0, itemCol);

			itemCol.SetText("Nombre");
			itemCol.SetAlign(wxLIST_FORMAT_LEFT);
			itemCol.SetWidth(150);
			this->listManager->InsertColumn(1, itemCol);

			itemCol.SetText("Tipo");
			itemCol.SetWidth(100);
			this->listManager->InsertColumn(2, itemCol);

			itemCol.SetText("Libre");
			itemCol.SetWidth(50);
			this->listManager->InsertColumn(3, itemCol);

			itemCol.SetText("Total");
			itemCol.SetWidth(50);
			this->listManager->InsertColumn(4, itemCol);

			//ENVIAR COMANDO OBTENER DRIVES
			this->iMODE = FM_EQUIPO;
			strComando = std::to_string(EnumComandos::FM_Discos);
			strComando.append(1, CMD_DEL);
			
			this->Enable(false);

			Sleep(100); //Darle algo de tiempo al gui para crear la lista
			
			this->EnviarComando(strComando);
			break;
		case EnumIDS::ID_Panel_FM_Descargas:
			this->listManager->ClearAll();
			itemCol.SetText("-");
			itemCol.SetWidth(20);
			itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
			this->listManager->InsertColumn(0, itemCol);

			itemCol.SetText("Nombre");
			itemCol.SetWidth(150);
			itemCol.SetAlign(wxLIST_FORMAT_LEFT);
			this->listManager->InsertColumn(1, itemCol);

			itemCol.SetText("Tamaño");
			itemCol.SetWidth(80);
			this->listManager->InsertColumn(2, itemCol);

			itemCol.SetText("Fecha Mod");
			itemCol.SetWidth(100);
			this->listManager->InsertColumn(3, itemCol);
			//ENVIAR COMANDO OBTENER FOLDER DESCARGAS
			this->iMODE = FM_NORMAL;

			strComando = std::to_string(EnumComandos::FM_Dir_Folder);
			strComando += "~DESCAR-DOWN";

			this->Enable(false);

			Sleep(100); //Darle algo de tiempo al gui para crear la lista
			
			this->EnviarComando(strComando);
			break;
		case EnumIDS::ID_Panel_FM_Escritorio:
			this->listManager->ClearAll();
			itemCol.SetText("-");
			itemCol.SetWidth(20);
			itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
			this->listManager->InsertColumn(0, itemCol);

			itemCol.SetText("Nombre");
			itemCol.SetWidth(150);
			itemCol.SetAlign(wxLIST_FORMAT_LEFT);
			this->listManager->InsertColumn(1, itemCol);

			itemCol.SetText("Tamaño");
			itemCol.SetWidth(80);
			this->listManager->InsertColumn(2, itemCol);

			itemCol.SetText("Fecha Mod");
			itemCol.SetWidth(100);
			this->listManager->InsertColumn(3, itemCol);
			//ENVIAR COMANDO OBTENER FOLDER DE ESCRITORIO

			this->iMODE = FM_NORMAL;

			strComando = std::to_string(EnumComandos::FM_Dir_Folder);
			strComando += "~ESCRI-DESK";

			this->Enable(false);

			Sleep(100); //Darle algo de tiempo al gui para crear la lista

			this->EnviarComando(strComando);
			break;
		case EnumIDS::ID_Panel_FM_Refresh:
			this->listManager->DeleteAllItems();
			Sleep(100);
			strComando = std::to_string(EnumComandos::FM_Dir_Folder);
			strComando.append(1, CMD_DEL);
			strComando += this->p_RutaActual->GetLabelText();
			
			this->Enable(false);
			
			Sleep(100);

			this->EnviarComando(strComando);
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
	this->listManager = new ListCtrlManager(this, EnumIDS::ID_Panel_FM_List, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 400), wxBORDER_THEME | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);

}

void panelFileManager::EnviarComando(std::string pComando) {
	p_Servidor->cSend(this->sckCliente, pComando.c_str(), pComando.size() + 1, 0, false);
}

void panelFileManager::EnviarArchivo(const std::string lPath, const char* rPath, std::string strCliente) {
	std::cout << "Enviando " << lPath << std::endl;

	std::ifstream localFile(lPath, std::ios::binary);
	if (!localFile.is_open()) {
		error();
		std::cout << "No se pudo abrir el archivo " << lPath << std::endl;
		return;
	}

	u_int uiTamBloque = 1024 * 70; //70 KB
	u64 uTamArchivo = GetFileSize(lPath.c_str());
	u64 uBytesEnviados = 0;

	//AGREGAR TRANSFER AL VECTOR DEL SERVIDOR
	struct TransferStatus nuevo_transfer;
	nuevo_transfer.isUpload = true;
	nuevo_transfer.uTamano = uTamArchivo;
	nuevo_transfer.strCliente = strCliente;
	nuevo_transfer.strNombre = lPath;
	nuevo_transfer.uDescargado = 0;
	std::unique_lock<std::mutex> lock(p_Servidor->p_transfers);
	p_Servidor->vcTransferencias.insert({ strCliente, nuevo_transfer });
	lock.unlock();


	std::string strComando = std::to_string(EnumComandos::FM_Descargar_Archivo_Init);
	strComando.append(1, CMD_DEL);
	strComando += rPath;
	strComando.append(1, CMD_DEL);
	strComando += std::to_string(uTamArchivo);
	//Enviar ruta remota y tamaño de archivo
	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, true);
	Sleep(100);

	//Calcular tamaño header
	std::string strHeader = std::to_string(EnumComandos::FM_Descargar_Archivo_Recibir);
	strHeader.append(1, CMD_DEL);

	int iHeaderSize = strHeader.size();
		
	char* cBufferArchivo = new char[uiTamBloque];
	int iBytesLeidos = 0;

	while (1) {
		localFile.read(cBufferArchivo, uiTamBloque);
		iBytesLeidos = localFile.gcount();
		if (iBytesLeidos > 0) {
			int iTotal = iBytesLeidos + iHeaderSize;
			char* nTempBuffer = new char[iTotal];

			memcpy(nTempBuffer, strHeader.c_str(), iHeaderSize);
			memcpy(nTempBuffer + iHeaderSize, cBufferArchivo, iBytesLeidos);

			//Implementar otro metodo para verificar el id antes de enviar
			uBytesEnviados += p_Servidor->cSend(this->sckCliente, nTempBuffer, iTotal, 0, true);
			std::unique_lock<std::mutex> lock(p_Servidor->p_transfers);
			p_Servidor->vcTransferencias[strCliente].uDescargado += iBytesLeidos;
			lock.unlock();
			
			if (nTempBuffer) {
				delete[] nTempBuffer;
				nTempBuffer = nullptr;
			}
		}else {
			break;
		}
	}

	localFile.close();

	//Ya se envio todo, cerrar el archivo
	Sleep(500);
	std::string strComandoCerrar = std::to_string(EnumComandos::FM_Descargar_Archivo_End);
	strComandoCerrar.append(1, CMD_DEL);
	p_Servidor->cSend(this->sckCliente, strComandoCerrar.c_str(), strComandoCerrar.size(), 0, true);

	if (cBufferArchivo) {
		delete[] cBufferArchivo;
		cBufferArchivo = nullptr;
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
		std::string strComando = std::to_string(EnumComandos::FM_Crear_Folder);
		strComando.append(1, CMD_DEL);
		strComando += this->CarpetaActual();
		strComando += dialog.GetValue();
		this->itemp->EnviarComando(strComando);
	}
}

void ListCtrlManager::OnCrearArchivo(wxCommandEvent& event) {
	wxTextEntryDialog dialog(this, "Nombre", "Crear archivo", "LEEME.txt", wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK) {
		std::string strComando = std::to_string(EnumComandos::FM_Crear_Archivo);
		strComando.append(1, CMD_DEL);
		strComando += this->CarpetaActual();
		strComando += dialog.GetValue();
		this->itemp->EnviarComando(strComando);
	}
}

void ListCtrlManager::OnBorrarArchivo(wxCommandEvent& event) {
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	wxString strFile = this->GetItemText(item, 1);
	//wxDialog dialog(this, 10000, "Borrar arhivo", wxDefaultPosition, wxDefaultSize, wxYES_NO, "Seguro que quieres borrar el archivo: " + strFile);
	wxMessageDialog dialog(this, "Seguro que quieres borrar el archivo: " + strFile + "?", "Borrar archivo", wxCENTER | wxNO_DEFAULT | wxYES_NO | wxICON_QUESTION);
	if (dialog.ShowModal() == wxID_YES) {
		//Borrar archivo
		std::string strComando = std::to_string(EnumComandos::FM_Borrar_Archivo);
		strComando.append(1, CMD_DEL);
		strComando += this->CarpetaActual();
		strComando += strFile;
		this->itemp->EnviarComando(strComando);
	}
}

void ListCtrlManager::OnDescargarArchivo(wxCommandEvent& event) {
	std::string strComando = std::to_string(EnumComandos::FM_Descargar_Archivo);
	std::string strID = RandomID(3);
	strComando.append(1, CMD_DEL);
	strComando += this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando += strID;

	//Agregar el archivo al vector del cliente pero solo el id
	//actualizar el tamaño al obtener la info del cliente
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string strNombre = this->itemp->strID;
	strNombre.append(1, '-');
	strNombre += this->GetItemText(item, 1);
	
	wxFileDialog dialog(this, "Guardar archivo", wxEmptyString, strNombre, wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (dialog.ShowModal() != wxID_OK) {
		return;
	}

	struct TransferStatus nuevo_transfer;
	struct Archivo_Descarga nuevo_archivo;
	nuevo_archivo.iFP = fopen(dialog.GetPath().c_str(), "wb");
	nuevo_transfer.uDescargado = nuevo_transfer.uTamano = nuevo_archivo.uTamarchivo = nuevo_archivo.uDescargado = 0;
	nuevo_transfer.strNombre = nuevo_archivo.strNombre = this->GetItemText(item, 1);
	nuevo_transfer.strCliente = this->itemp->strID;
	nuevo_transfer.isUpload = false;
	if (nuevo_archivo.iFP == nullptr) {
		error();
		p_Servidor->m_txtLog->LogThis("[X] No se pudo abrir el archivo " + strNombre, LogType::LogError);
		return;
	}

	
	int iClienteID = p_Servidor->IndexOf(this->itemp->strID);
	if (iClienteID == -1) {
		p_Servidor->m_txtLog->LogThis("[X] No se pudo encontrar el cliente " + this->itemp->strID, LogType::LogError);
		return;
	}

	std::unique_lock<std::mutex> lock(p_Servidor->vc_Clientes[iClienteID]->mt_Archivos);
	p_Servidor->vc_Clientes[iClienteID]->um_Archivos_Descarga.insert({ strID, nuevo_archivo });
	lock.unlock();

	std::unique_lock<std::mutex> lock2(p_Servidor->p_transfers);
	p_Servidor->vcTransferencias.insert({ strID, nuevo_transfer });
	lock2.unlock();

	this->itemp->EnviarComando(strComando);
}

void ListCtrlManager::OnEjecutarArchivo_Visible(wxCommandEvent& event) {
	std::string strComando = std::to_string(EnumComandos::FM_Ejecutar_Archivo);
	strComando.append(1, CMD_DEL);
	strComando += this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando.append(1, '1');
	this->itemp->EnviarComando(strComando);
}

void ListCtrlManager::OnEjecutarArchivo_Oculto(wxCommandEvent& event) {
	std::string strComando = std::to_string(EnumComandos::FM_Ejecutar_Archivo);
	strComando.append(1, CMD_DEL);
	strComando += this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando.append(1, '0');
	this->itemp->EnviarComando(strComando);
}

void ListCtrlManager::OnEditarArchivo(wxCommandEvent& event) {
	wxString strFile = this->ArchivoSeleccionado();
	wxEditForm* editor_txt = new wxEditForm(this, strFile, RandomID(4));
	editor_txt->Show(true);
}

void ListCtrlManager::OnEncriptarArchivo(wxCommandEvent& event) {
	frameEncryption* frm_crypt = new frameEncryption(this, this->ArchivoSeleccionado());
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
			strCommand = std::to_string(EnumComandos::FM_Dir_Folder);
			strCommand.append(1, CMD_DEL);
			strCommand += strPath;

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

			Sleep(100);

			itemp->EnviarComando(strCommand);
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

					strCommand = std::to_string(EnumComandos::FM_Dir_Folder);
					strCommand.append(1, CMD_DEL);
					strCommand += itemp->RutaActual();

					itemp->Enable(false); 

					Sleep(100); 

					itemp->EnviarComando(strCommand);
					
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
		menu.AppendSeparator();
		//menu.AppendSubMenu(crypt_Menu, "Crypt");
		menu.Append(EnumMenuFM::ID_Crypt, "Crypt");
		menu.Append(EnumMenuFM::ID_Eliminar, "Eliminar");

		PopupMenu(&menu, pos.x, pos.y);
	} else {
		//pop menu para folder
		wxMenu* new_menu = new wxMenu;
		new_menu->Append(EnumMenuFM::ID_New_Folder, "Folder/Carpeta");
		new_menu->Append(EnumMenuFM::ID_New_Archivo, "Archivo");

		menu.AppendSubMenu(new_menu, "Nuevo");
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

void ListCtrlManager::ListarDir(const std::vector<std::string> vcEntrys, const wxString strSize) {
	//Listar directorio
	//4 columnas para listar dir
	std::unique_lock<std::mutex> lock(this->mtx_fm);
	if (this->GetColumnCount() == 4) {
		int iCount = this->GetItemCount() > 0 ? this->GetItemCount() - 1 : 0;
		if (iCount == -1) { iCount = 0; }
		this->InsertItem(iCount, wxString("-"));
		this->SetItem(iCount, 1, wxString(vcEntrys[1]));
		this->SetItem(iCount, 2, strSize); //tama
		this->SetItem(iCount, 3, wxString(vcEntrys[3]));
	}else {
		std::cout << "No se han creado las columnas\n";
	}
	this->GetParent()->Enable(true);
}

void ListCtrlManager::ListarEquipo(const std::vector<std::string> vcDrives) {
	//Listar discos y almacenamiento
	//5 Columnas para drives
	std::unique_lock<std::mutex> lock(this->mtx_fm);
	if (this->GetColumnCount() == 5) {
		for (int iCount = 0; iCount<int(vcDrives.size()); iCount++) {
			std::vector<std::string> vDrive = strSplit(vcDrives[iCount], '|', 5);
			if (vDrive.size() >= 5) {
				this->InsertItem(iCount, wxString(vDrive[0]));
				this->SetItem(iCount, 1, wxString(vDrive[2]));
				this->SetItem(iCount, 2, wxString(vDrive[1]));
				this->SetItem(iCount, 3, wxString(vDrive[3]));
				this->SetItem(iCount, 4, wxString(vDrive[4]));
			}
		}
	}else {
		std::cout << "No se han creado las columnas\n";
	}
	this->GetParent()->Enable(true);
}