#ifndef _MISC
#define _MISC

#include "headers.hpp"

bool isEscribirSalidaShell(std::string stdID, std::string strSalida);
std::string RandomTestLen();
std::string RandomID(int iLongitud);
std::vector<std::string> strSplit(const std::string& strString, char cDelimiter, int iMax);

#endif