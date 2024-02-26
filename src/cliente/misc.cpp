#include "misc.hpp"

std::string strUserName() {
	std::string strOutput = "";
	char cUser[UNLEN + 1];
	char cMachineName[100];
	DWORD dLen = UNLEN + 1;
	if (GetUserName(cUser, &dLen) != 0) {
		strOutput = cUser;
	}
	else {
		strOutput = "unknown";
	}
	dLen = 100;
	if (GetComputerName(cMachineName, &dLen)) {
		strOutput.append(1, '@');
		strOutput += cMachineName;
	}
	else {
		strOutput = "@unknown";
	}

	return strOutput;
}

std::string strOS() {
	//os regedit
	std::string strOut = "";
	HKEY hKey;
	auto ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_QUERY_VALUE, &hKey);
	if (ret != ERROR_SUCCESS) {
#ifdef ___DEBUG_
		error();
#endif
		strOut = ":v";
		return strOut;
	}
	LPBYTE lBuffer = (LPBYTE)malloc(50);
	DWORD dLen = 50;
	if (RegQueryValueEx(hKey, "ProductName", nullptr, nullptr, lBuffer, &dLen) == ERROR_SUCCESS) {
		strOut.append((const char*)lBuffer);
	}
	else {
#ifdef ___DEBUG_
		std::cout << "Unable to retrieve product name\n";
		error();
#endif
		//no se pudo oibtener el SO
		strOut = "Windows :v";
	}
	free(lBuffer);
	lBuffer = nullptr;
	RegCloseKey(hKey);
	return strOut;
}

std::vector<std::string> strSplit(const std::string& strString, char cDelimiter, int iMax) {
	std::vector<std::string> vcOut;
	int istrLen = strString.length(), iIt = 0, iCounter = 0, iTmp = 0;
	for (; iIt < istrLen; iIt++) {
		std::string strTmp = "";
		while (strString[iIt] != cDelimiter && strString[iIt] != '\0') {
			strTmp.append(1, strString[iIt++]);
			iCounter++;
		}
		iCounter = 0;
		vcOut.push_back(strTmp);
		if (++iTmp == iMax) { break; }

	}
	return vcOut;
}

std::string strCpu() {
	std::string strOut = "";
	int CPUInfo[4] = { -1 };
	char CPUBrandString[200];
	__cpuid(CPUInfo, 0x80000000);
	unsigned int nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));
	for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
		__cpuid(CPUInfo, i);
		if (i == 0x80000002) {
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		}
		else if (i == 0x80000003) {
			memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		}
		else if (i == 0x80000004) {
			memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}
	strOut = CPUBrandString;
	
	SYSTEM_INFO sInfo;
	GetNativeSystemInfo(&sInfo);
	switch (sInfo.wProcessorArchitecture) {
	case 9:
		strOut += "x64 (AMD or INTEL)";
		break;
	case 5:
		strOut += "ARM";
		break;
	case 12:
		strOut += "ARM64";
		break;
	case 6:
		strOut += "Intel Itanium-based";
		break;
	case 0:
		strOut += "x86";
		break;
	default:
		strOut += "Unknow";
		break;
	}

	return strOut;
}