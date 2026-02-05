#include "panel_transfer.hpp"

panelTransfer::panelTransfer(wxWindow* _pParent, const std::string _strNameID, const std::string _strNombreArchivo, const std::string _strCliente, bool esSubida)
	: wxFrame(_pParent, wxID_ANY, "Transferencia") {
	this->SetTitle((esSubida ? "Enviando archivo a " : "Descargando archivo desde ") + _strCliente);
	this->SetName("TRANSFER-" + _strNameID);
	
	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	this->gProgress = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(400, 30), wxGA_HORIZONTAL | wxGA_PROGRESS | wxGA_TEXT);
	
	main_sizer->AddSpacer(10);
	main_sizer->Add(new wxStaticText(this, wxID_ANY, (esSubida ? "Enviado " : "Descargando ") + _strNombreArchivo), 1, wxCENTER);
	main_sizer->AddSpacer(10);
	main_sizer->Add(this->gProgress, 0, wxALL);
	main_sizer->AddSpacer(10);
	
	this->SetSizerAndFit(main_sizer);
	this->SetSizeHints(this->GetSize(), this->GetSize());

	this->lblProgress = new wxStaticText(this, wxID_ANY, "35%", wxPoint(this->gProgress->GetPosition().x + (this->gProgress->GetSize().GetWidth() /2), this->gProgress->GetPosition().y + (this->gProgress->GetSize().GetHeight() / 2 / 2)));
}

void panelTransfer::AgregarProgreso(unsigned long long descargado, unsigned long long llTotal) {
	std::unique_lock<std::mutex> lock(this->mtx_progress);

	double total = (static_cast<double>(descargado) / static_cast<double>(llTotal)) * 100;

	this->gProgress->SetValue(static_cast<int>(total));
	this->lblProgress->SetLabelText(std::to_string(total) + "%");
}

void panelTransfer::DoneOmar() {
	std::unique_lock<std::mutex> lock(this->mtx_progress);

	this->gProgress->SetValue(100);
	this->lblProgress->SetLabelText("100%");
	wxMessageBox("Transferencia completa", "Transferencia de arhivos", 5L, this);
}
