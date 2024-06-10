#include "panel_camara.hpp"

wxBEGIN_EVENT_TABLE(panelCamara, wxPanel)
	EVT_BUTTON(EnumIDS::ID_CM_Test, panelCamara::TestBtn)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(panelPictureBox, wxPanel)
	EVT_PAINT(panelPictureBox::OnPaint)
wxEND_EVENT_TABLE()

void panelPictureBox::OnPaint(wxPaintEvent& event) {
	if (this->isCalled) {
		wxBufferedPaintDC dc(this);
		dc.Clear();
		
		//Cargar el archivo o buffer
		this->bmp_Obj.LoadFile("C:\\Users\\user\\Desktop\\logo.bmp", wxBITMAP_TYPE_BMP);
		if (this->bmp_Obj.IsOk()) {
			dc.DrawBitmap(this->bmp_Obj, 0, 0, false);
		} else {
			dc.DrawText("Error cargando al bana :v", 50, 50);
		}
		this->isCalled = false;
	}
}

panelCamara::panelCamara(wxWindow* pParent):
	wxPanel(pParent, wxID_ANY){
	wxButton* btn_Test = new wxButton(this, EnumIDS::ID_CM_Test, "Test");
	this->pictureBox = new panelPictureBox(nullptr);
	this->pictureBox->Show(true);

	wxBoxSizer* nSizer = new wxBoxSizer(wxVERTICAL);
	nSizer->Add(btn_Test, 0, wxALL, 1);
	nSizer->AddSpacer(20);
	//nSizer->Add(this->pictureBox, 1, wxEXPAND | wxALL, 1);

	this->SetSizerAndFit(nSizer);
}

void panelCamara::TestBtn(wxCommandEvent& event) {
	this->pictureBox->isCalled = true;
	this->pictureBox->Refresh();
}

