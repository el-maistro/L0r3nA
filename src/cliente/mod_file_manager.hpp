#ifndef ___FM
#define ___FM
#include "headers.hpp"
#include "cliente.hpp"

struct sDrives {
	char cLetter[10];
	char cLabel[50];
	char cType[20];
	double dFree;
	double dTotal;
};

std::vector<struct sDrives> Drives();
std::vector<std::string> vDir(c_char* cPath);
void CrearFolder(c_char* cPath);
void CrearArchivo(c_char* cPath);
void BorrarArchivo(c_char* cPath);
void BorrarFolder(c_char* cPath);
void EnviarArchivo(const std::string& cPath, const std::string& cID, Cliente* copy_ptr);
#endif
