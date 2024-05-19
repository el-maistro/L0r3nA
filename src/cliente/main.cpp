#include "headers.hpp"
#include "cliente.hpp"

Cliente* cCliente;

int main() {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		error();
		return -1;
	}

	cCliente = new Cliente();
	
	while (cCliente->isRunning) {
		if (cCliente->bConectar("127.0.0.1", "31000")) {
			cCliente->iniPacket();
			cCliente->MainLoop();
		} else {
			//no se pudo conectar
#ifdef ___DEBUG_
			std::cout<<"No se pudo conectar el host\n";
#endif
		}
		Sleep(6000); //Esperar cinco segundos para volver a intentar
	}

	delete cCliente;
	cCliente = nullptr;

	WSACleanup();
	return 0;
}
