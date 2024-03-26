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