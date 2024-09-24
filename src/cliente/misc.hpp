#include "headers.hpp"
void printHex(const char* data, int length);
std::string RandomID(int iLongitud);
std::string strCpu();
std::string strUserName();
std::string strOS();
std::vector<std::string> strSplit(const std::string& strString, char cDelimiter, int iMax);
u64 GetFileSize(c_char* cPath);
bool Execute(const char *cCmdLine, int iOpt);
bool EndProcess(int iPID);
std::string strProcessList();
u64 StrToUint(const std::string strString);

//void _DEBUG(const std::string strMsg, int iValor=0);
//void _DEBUG(const char* cMsg, int iValor=0);


