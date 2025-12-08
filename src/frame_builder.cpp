#include "frame_builder.hpp"

FrameBuilder::FrameBuilder(wxWindow* pParent)
: wxFrame(pParent, wxID_ANY, "Generar cliente", wxDefaultPosition, wxDefaultSize){

	wxPanel* pnl_Main = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* box_Server = new wxStaticBoxSizer(wxVERTICAL, pnl_Main, "Informacion de conexion");

	box_Server->Add(new wxStaticText(pnl_Main, wxID_ANY, "HOST:"));

	pnl_Main->SetSizer(box_Server);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(pnl_Main);
	main_sizer->Add(new wxStaticText(this, wxID_ANY, "Random data"));

	this->SetSizerAndFit(main_sizer);
}