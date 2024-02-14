#include "misc.hpp"


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