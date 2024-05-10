#ifndef _CRYPT_DB
#define _CRYPT_DB

#include "headers.hpp"


class ListCtrlManager3 : public wxListCtrl {
	public:
		ListCtrlManager3(wxWindow* parent, const wxWindowID id,
			const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {}

	private:
		void OnRefrescar(wxCommandEvent& event);
		void OnEliminar(wxCommandEvent& event);
		void OnCopiarPass(wxCommandEvent& event);

		void ShowContextMenu(const wxPoint& pos, bool isEmpty);
		void OnContextMenu(wxContextMenuEvent& event);

		wxDECLARE_EVENT_TABLE();
};

class frameCryptDB : public wxFrame {
	public:
		static int static_callback(void* NotUsed, int argc, char** argv, char** azColName) {
			frameCryptDB* self = static_cast<frameCryptDB*>(NotUsed);
			return self->callback(argc, argv, azColName);
		}
		int callback(int argc, char** argv, char** azColName) {
			int i;
			for (i = 0; i < argc; i++)
			{
				//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
				if (strncmp(azColName[i], "id", 2) == 0) {
					this->p_listctrl->InsertItem(this->iCount, wxString(argv[i]));
				}else if (strncmp(azColName[i], "ip", 2) == 0) {
					this->p_listctrl->SetItem(this->iCount, 1, wxString(argv[i]));
				}else if (strncmp(azColName[i], "nombre", 6) == 0) {
					this->p_listctrl->SetItem(this->iCount, 2, wxString(argv[i]));
				}else if (strncmp(azColName[i], "fecha", 5) == 0) {
					this->p_listctrl->SetItem(this->iCount, 3, wxString(argv[i]));
				}else if (strncmp(azColName[i], "pass", 4) == 0) {
					this->p_listctrl->SetItem(this->iCount, 4, wxString(argv[i]));
					this->iCount++;
				}
			}
			
			return 0;
		}

		void ObtenerData() {
			sqlite3* db;
			this->iCount = 0;
			this->p_listctrl->DeleteAllItems();
			char* zErrMsg = 0;
			if (sqlite3_open(DB_FILE, &db) == SQLITE_OK) {
				sqlite3_exec(db, "SELECT * FROM keys;", static_callback, this, &zErrMsg);
			}
			sqlite3_free(zErrMsg);
			sqlite3_close(db);
		}

		void Exec_SQL(const char* cCMD);
		
		frameCryptDB();
	private:
		int iCount = 0;
		ListCtrlManager3* p_listctrl = nullptr;
		
};

#endif // !_CRYPT_DB
