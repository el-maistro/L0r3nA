#ifndef __PNL_ESCANER
#define __PNL_ESCANER 1

#include "headers.hpp"

namespace EnumEscanerIDS {
	enum Enum {
		Main_Window = 100,
		ListCtrl,
		BTN_Scan,
		BTN_SynScan,
		BTN_SckScan,
		BTN_FullScan_Syn,
		BTN_FullScan_Sck
	};
}

class panelEscaner : public wxFrame {
	public:
		panelEscaner(wxWindow* pParent, SOCKET _sck, std::string _strID, ByteArray c_key);

		void AddData(const char* _buffer);
		
	private:
		std::mutex mtx_carga;
		ByteArray enc_key;
		wxActivityIndicator* m_indicator = nullptr;
		wxDataViewColumn* m_col;
		wxTextCtrl* txtHostBase = nullptr;
		wxTextCtrl* txtPortFrom = nullptr;
		wxTextCtrl* txtPortTo   = nullptr;
		wxListCtrl* list_ctrl   = nullptr;
		wxComboBox* cmb_Subnet  = nullptr;
		wxComboBox* cmb_Tipo    = nullptr;

		void CrearListView();
		void OnScan(wxCommandEvent& event);
		void OnAgregarDatos(wxCommandEvent& event);
		void OnMostrarPuertos(wxListEvent& event);

		void MostrarCarga();
		void OcultarCarga();

		SOCKET sckSocket = INVALID_SOCKET;

		wxDECLARE_EVENT_TABLE();

};

class framePorts : public wxFrame {
	public:
		framePorts(wxWindow* pParent, wxString _ports, wxString _title);
};

#endif
