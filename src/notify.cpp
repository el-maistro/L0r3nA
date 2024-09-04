#include "notify.hpp"
#include "frame_main.hpp"

void MyNotify::CloseThread(int iSecs) {
    std::this_thread::sleep_for(std::chrono::seconds(iSecs));
    m_frame->CallAfter([this] {
        m_frame->Close();
      });
}

void MyNotify::SpawnThread() {
    this->th_close = std::thread(&MyNotify::CloseThread, this, this->iSecDelay);
}

MyNotify::MyNotify(wxWindow* pParent, const std::string strTitle, const std::string strMessage, int _iSecDelay){
    
    this->m_frame = new wxFrame(pParent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFRAME_TOOL_WINDOW | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP);
    this->m_frame->SetBackgroundColour(wxColor(0, 0, 0));
    
    wxDisplay* scr = new wxDisplay(this->m_frame);
    wxRect scr_Size = scr->GetClientArea();
    delete scr;
    scr = nullptr;

    this->iSecDelay = _iSecDelay;

    wxPanel* nPanel = new wxPanel(this->m_frame, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    wxButton* btn_close = new wxButton(nPanel, CLOSE_BTN_ID, "X", wxDefaultPosition, wxSize(12, 12));
    btn_close->SetForegroundColour(wxColor(255, 255, 255));
    btn_close->SetBackgroundColour(wxColor(0, 0, 0));

    wxStaticText* txt_title = new wxStaticText(nPanel, wxID_ANY, wxString(strTitle), wxDefaultPosition);
    txt_title->SetForegroundColour(wxColor(255, 255, 255));
    
    wxStaticText* txt_message = new wxStaticText(nPanel, wxID_ANY, wxString(strMessage), wxDefaultPosition);
    txt_message->SetForegroundColour(wxColor(255, 255, 255));
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(btn_close, 0, wxALIGN_RIGHT | wxALL, 0);
    sizer->Add(txt_title, 0, wxALIGN_LEFT | wxALIGN_TOP, 1);
    sizer->AddSpacer(10);
    sizer->Add(txt_message, 0, wxEXPAND | wxALL, 1);

    nPanel->SetSizer(sizer);

   this->imageCtrl = new wxStaticBitmap(this->m_frame, wxID_ANY, wxBitmap(W_HEIGHT, W_HEIGHT));

    wxImage img("./imgs/logo_512x512.jpg", wxBITMAP_TYPE_ANY);
    if (img.IsOk()) {
        img.Rescale(W_HEIGHT, W_HEIGHT);
        wxBitmap bmp_Obj(img);
        this->imageCtrl->SetBitmap(bmp_Obj);
    }

    wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
    sizer2->Add(this->imageCtrl, 0, wxEXPAND | wxALL, 0);
    sizer2->Add(nPanel, 1, wxEXPAND | wxALL, 1);

    this->m_frame->SetSizer(sizer2);
    
    wxPoint new_pos(scr_Size.GetWidth() - W_WIDTH, scr_Size.GetHeight() - W_HEIGHT);
    this->m_frame->SetSize(wxSize(W_WIDTH, W_HEIGHT));
    this->m_frame->SetPosition(new_pos);

    this->m_frame->Show();

    this->SpawnThread();
}