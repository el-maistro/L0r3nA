#ifndef _NOTIFY_H
#define _NOTIFY_H

#include "headers.hpp"

#define W_WIDTH 300
#define W_HEIGHT 70

class MyNotify: public std::enable_shared_from_this<MyNotify> {
    private:
        wxStaticBitmap* imageCtrl = nullptr;
        std::thread th_close;
        void CloseThread(int iSecs, std::shared_ptr<MyNotify> self);
        int iSecDelay = 0;
        
    public:
        wxFrame* m_frame = nullptr;

        void SpawnThread(std::shared_ptr<MyNotify> self);

        static std::shared_ptr<MyNotify> Create(wxWindow* pParent, const std::string& strTitle, const std::string& strContent, int iSecDelay) {
            auto instance = std::shared_ptr<MyNotify>(new MyNotify(pParent, strTitle, strContent, iSecDelay));
            instance->SpawnThread(instance); // Llama a SpawnThread después de que el objeto esté completamente construido
            return instance;
        }

        MyNotify(wxWindow* pParent, const std::string strTitle, const std::string strContent, int iSecDelay);
        ~MyNotify() {
            if (imageCtrl) {
                delete imageCtrl;
                imageCtrl = nullptr;
            }
        }
};

#endif