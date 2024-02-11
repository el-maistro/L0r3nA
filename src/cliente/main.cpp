#include "headers.hpp"
#include "cliente.hpp"

int main(int argc, char** argv) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		error();
		return -1;
	}

	Cliente* cCliente = new Cliente();

	if (cCliente->bConectar(argv[1], argv[2])) {
		cCliente->MainLoop();
	}

	delete cCliente;
	cCliente = nullptr;
	return 0;
}