#pragma once

#include "headers.hpp"

void printHex(const char* data, int length);
std::string RandomID(int iLongitud);
int RandomID();
std::string strCpu();
std::string strGetComputerName();
std::string strUserName();
std::string strOS();
std::string TimeToDays(const std::string& strtime);
std::string TimeToDays(const unsigned long long& strtime);
std::string TimeToString(unsigned long long ullTime);
std::vector<std::string> strSplit(const std::string& strString, char cDelimiter, int iMax);
std::vector<std::string> strSplit(const std::string& strString, std::string strDelimited, int iMax);
u64 GetFileSize(c_char* cPath);
bool Execute(const char *cCmdLine, int iOpt);

#ifdef __MOD_PM
bool EndProcess(int iPID);
std::string strProcessList();
#endif

u64 StrToUint(const std::string strString);

std::vector<std::string> IPSlocales();
int RAM();

//dynamic dll load wrap
HMODULE __stdcall wrapLoadDLL(const char* _cDLL);
FARPROC wrapGetProcAddr(HMODULE _hMod, LPCSTR _procName);
BOOL wrapFreeLibrary(HMODULE _hMod);


void* m_memcpy(void* _Dst, void const* _Src, size_t _size);
void* m_memset(void* __dst, int __val, size_t __n);