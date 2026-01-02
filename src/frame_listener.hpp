#ifndef __FRM_LISTENER
#define __FRM_LISTENER

#include "headers.hpp"

namespace EnumIDSListeners {
	enum Enum {
		ID_GenerarPass = 12345,
		ID_CrearListener,
		ID_CM_Refrescar,
		ID_CM_Copiar,
		ID_CM_Eliminar,
		ID_CM_Habilitar,
		ID_CM_Deshabilitar,
		ID_TXT_Puerto
	};
}

class ListCtrlManagerListeners : public wxListCtrl {
	public:
		ListCtrlManagerListeners(wxWindow* parent, const wxWindowID id,
			const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {
		}

		void MostrarLista();
		int Callback(int argc, char** argv, char** azColName);

		bool isExiste(const char* _cnombre);

	private:
		int iCount = 0;
		int iSelectedIndex = 0;
		void OnRefrescar(wxCommandEvent& event);
		void OnEliminar(wxCommandEvent& event);
		void OnCopiarPass(wxCommandEvent& event);
		void OnToggle(wxCommandEvent& event);

		void ShowContextMenu(const wxPoint& pos, bool isEmpty);
		void OnContextMenu(wxContextMenuEvent& event);

		wxDECLARE_EVENT_TABLE();
};

class frameListeners : public wxFrame {
	public:
		frameListeners(wxWindow* pParent);
	
		void OnGenerarPass(wxCommandEvent& event);
		void OnCrearListener(wxCommandEvent& event);
		void OnInputPuerto(wxCommandEvent& event);
		void Exec_SQL(const char* cCMD);
	private:

		ListCtrlManagerListeners* list_ctrl = nullptr;
		void CrearLista();
		wxTextCtrl* txtNombre = nullptr;
		wxTextCtrl* txtPass = nullptr;
		wxTextCtrl* txtPuerto = nullptr;

		wxDECLARE_EVENT_TABLE();
};

#endif
