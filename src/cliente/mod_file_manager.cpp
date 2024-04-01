#include "mod_file_manager.hpp"

std::vector<struct sDrives> Drives() {
	std::vector<struct sDrives> vcOutput;
	char cDrives[512];
	memset(cDrives, 0, sizeof(cDrives));
	int iRet = GetLogicalDriveStrings(sizeof(cDrives), cDrives);
	if (iRet > 0) {
		char* p1 = cDrives;
		char* p2;
		while (*p1 != '\0' && (p2 = strchr(p1, '\0')) != nullptr) {
			char cLabel[128]; memset(cLabel, '\0', 128);
			char cType[128]; memset(cType, '\0', 128);
			iRet = GetVolumeInformationA(p1, cLabel, 128, nullptr, nullptr, nullptr, cType, 128);
			if (strlen(cLabel) <= 0) {
				UINT dt = GetDriveTypeA(p1);
				switch (dt) {
				case 0:
					strncpy(cLabel, "Desconocido", 12);
					break;
				case 1:
					strncpy(cLabel, "Volumen no montado", 19);
					break;
				case 2:
					strncpy(cLabel, "USB", 5);
					break;
				case 3:
					strncpy(cLabel, "Disco Duro", 11);
					break;
				case 4:
					strncpy(cLabel, "Disco Remoto", 13);
					break;
				case 5:
					strncpy(cLabel, "CD-ROM", 7);
					break;
				case 6:
					strncpy(cLabel, "RAM Disk", 9);
					break;
				default:
					strncpy(cLabel, "Desconocido", 12);
					break;
				}
			}
			struct sDrives sDrive;
			if (iRet != 0) {
				__int64 FreeBytesAvaiableToUser, TotalFreeBytes;
				GetDiskFreeSpaceEx(p1, (PULARGE_INTEGER)&FreeBytesAvaiableToUser, (PULARGE_INTEGER)&TotalFreeBytes, nullptr);
				double dFreegigs = (((double)(FreeBytesAvaiableToUser / 1024) / 1024) / 1024);
				double dTotalgigs = (((double)(TotalFreeBytes / 1024) / 1024) / 1024);
				strncpy(sDrive.cLetter, p1, 10);
				strncpy(sDrive.cLabel, cLabel, 50);
				strncpy(sDrive.cType, cType, 20);
				sDrive.dFree = dFreegigs;
				sDrive.dTotal = dTotalgigs;
			}
			else {
				strncpy(sDrive.cLetter, p1, 10);
				strncpy(sDrive.cLabel, cLabel, 50);
				strncpy(sDrive.cType, "-", 2);
				sDrive.dFree = 0;
				sDrive.dTotal = 0;
			}
			vcOutput.push_back(sDrive);
			p1 = p2 + 1;
		}

	}

	return vcOutput;

}

std::vector<std::string> vDir(const char* cPath) {
	std::vector<std::string> vcFolders;
	std::vector<std::string> vcFiles;
	struct stat info;
	WIN32_FIND_DATA win32Archivo;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	snprintf(szDir, MAX_PATH, "%s*", cPath);
	SYSTEMTIME FileDate;
	char cFecha[10];
	char cTmpdir[2048];
	hFind = FindFirstFileA(szDir, &win32Archivo);

	if (!hFind) {
#ifdef ___DEBUG_		
		error();
#endif
	}
	do {
		//cFecha tmpdir baia.cFileName win32Archivo.nFilesizeLow

		FileTimeToSystemTime(&win32Archivo.ftCreationTime, &FileDate);
		
		ZeroMemory(cFecha, sizeof(cFecha));
		ZeroMemory(cTmpdir, sizeof(cTmpdir));

		snprintf(cFecha, 10, "%d/%d/%d", FileDate.wDay, FileDate.wMonth, FileDate.wYear);
		
		strncpy(cTmpdir, cPath, 2047);
		strncat(cTmpdir, win32Archivo.cFileName, 2047 - strnlen(cTmpdir, 2047));
		
		stat(cTmpdir, &info);

		std::string strEntry = "";

		if (info.st_mode & S_IFDIR) {
			//Directorio
			strEntry = "0>";
			strEntry += win32Archivo.cFileName;
			strEntry += ">->";
			strEntry += cFecha;
			vcFolders.push_back(strEntry);
		} else {
			strEntry = "0<";
			strEntry += win32Archivo.cFileName;
			strEntry += "<";
			strEntry += std::to_string(win32Archivo.nFileSizeLow);
			strEntry += "<";
			strEntry += cFecha;
			vcFiles.push_back(strEntry);

		}

	} while (FindNextFile(hFind, &win32Archivo) != 0);
	if (hFind) {
		FindClose(hFind);
	}

	for (auto item : vcFiles) {
		vcFolders.push_back(item);
		
	}

	return vcFolders;
}