#ifndef __TRANSFERS
#define __TRANSFERS 1

#include "headers.hpp"

extern struct TransferStatus;

class panelTransferencias : public wxPanel {
	public:
		panelTransferencias(wxWindow* pParent, std::string strID);
		~panelTransferencias();

		void OnClose(wxCloseEvent& event);

		bool isActive();
		void SetActive(bool status);

	private:
		std::string strClienteID = "";
		bool _isActive = false;
		std::thread th_monitor;
		std::mutex mtx_global;
		std::mutex mtx_vector;
		wxDataViewListCtrl* dataView = nullptr;

		//Thread monitor
		void SpawnThread();
		void JoinThread();
		void thMonitor();

		int m_IndexOf(const wxString& strID);

		void m_InsertarTransfer(const TransferStatus& transferencia);
		
		//wxDECLARE_EVENT_TABLE();

};

#endif