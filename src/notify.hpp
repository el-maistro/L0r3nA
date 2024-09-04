#ifndef _NOTIFY_H
#define _NOTIFY_H

#include "headers.hpp"

#define CLOSE_BTN_ID 31337
#define W_WIDTH 300
#define W_HEIGHT 70

class MyNotify{
    private:
        wxStaticBitmap* imageCtrl = nullptr;
        std::thread th_close;
        void CloseThread(int iSecs);
        int iSecDelay = 0;
        
    public:
        wxFrame* m_frame = nullptr;

        void SpawnThread();

        void Join() {
            if (th_close.joinable()) {
                th_close.join(); 
            }
        }

        MyNotify(wxWindow* pParent, const std::string strTitle, const std::string strContent, int iSecDelay);
        ~MyNotify() {
            if (imageCtrl) {
                delete imageCtrl;
                imageCtrl = nullptr;
            }
            Join();
        }
};

#endif