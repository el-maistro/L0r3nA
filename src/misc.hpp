#pragma once

#ifndef _MISC
#define _MISC

#include "headers.hpp"

void printHex(const char* data, int length);

std::string RandomTestLen();
std::string RandomID(int iLongitud);
int RandomID();
std::string RandomPass(int iLongitud);
std::string TimeToString(unsigned long long ullTime);
std::vector<std::string> strSplit(const std::string& strString, char cDelimiter, int iMax);
std::vector<std::string> strSplit(const std::string& strString, std::string strDelimited, int iMax);
u64 StrToUint(const char* strString);
u64 GetFileSize(const char* cPath);
int FilterSocket(std::string cID);

//Tema
void ChangeMyChildsTheme(wxWindow* parent, wxColour background, wxColour foreground, wxFont font);

#endif