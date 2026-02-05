#pragma once
#ifndef __PNL_TRANSFER
#define __PNL_TRANSFER 1

#include "headers.hpp"
#include <wx/gauge.h>



class panelTransfer : public wxFrame {
	private:
		
		std::mutex mtx_progress;
		wxStaticText* lblProgress = nullptr;
		wxGauge* gProgress = nullptr;
		//wxDECLARE_EVENT_TABLE();
	public:
		panelTransfer(wxWindow* _pParent, const std::string _strNameID, const std::string _strNombreArchivo, const std::string _strCliente, bool esSubida);

		void AgregarProgreso(unsigned long long descargado, unsigned long long total);
		void DoneOmar();
};

#endif