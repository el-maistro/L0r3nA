#ifndef __CAM_
#define __CAM_

#include "headers.hpp"

//Hacer que el frame tenga menu para tomar single-shot o hacer captura en vivo, ademas de opciones para grabar y guardar
class panelPictureBox : public wxFrame {
	public:
		panelPictureBox(wxWindow* parent) : wxFrame(parent, wxID_ANY, "Camara") {}

		bool isCalled = false;
		wxBitmap bmp_Obj;
		
		void OnPaint(wxPaintEvent& event);
	private:
		
		wxDECLARE_EVENT_TABLE();
};

class panelCamara : public wxPanel{
	public:
		panelCamara(wxWindow* pParent);

		void TestBtn(wxCommandEvent& event);

	private:
		panelPictureBox* pictureBox = nullptr;
		wxDECLARE_EVENT_TABLE();

};

#endif