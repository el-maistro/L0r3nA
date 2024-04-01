#ifndef ___FM
#define ___FM
#include "headers.hpp"

struct sDrives {
	char cLetter[10];
	char cLabel[50];
	char cType[20];
	double dFree;
	double dTotal;
};

std::vector<struct sDrives> Drives();
std::vector<std::string> vDir(const char* cPath);

#endif
