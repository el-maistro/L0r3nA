#include "headers.hpp"
#include "cliente.hpp"
#include "misc.hpp"

Cliente* cCliente;

int main(int argc, char** argv) {
	char* cHost = "127.0.0.1";
	char* cPort = "31337";
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		error();
		return -1;
	}

	cCliente = new Cliente();
	
	while (cCliente->isRunning) {
		if (cCliente->bConectar(argc == 3 ? argv[1] : cHost, argc == 3 ? argv[2]: cPort)) {
			cCliente->iniPacket();
			cCliente->MainLoop();
		} else {
			//no se pudo conectar
			DebugPrint("No se pudo conectar el host");
		}
		Sleep(6000); //Esperar 6 segundos para volver a intentar
	}

	delete cCliente;
	cCliente = nullptr;

	WSACleanup();
	return 0;
}
