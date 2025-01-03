#include "headers.hpp"
#include "cliente.hpp"
#include "misc.hpp"

#include "mod_escaner.hpp"

Cliente* cCliente;

#ifdef ___DBG__
int main(int argc, char** argv) {
#else
int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/) {
#endif

	const char* cHost = "127.0.0.1";
	const char* cPort = "65500";
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		error();
		return -1;
	}

	cCliente = new Cliente();
	
	while (cCliente->m_isRunning()) {
#ifdef ___DBG__
		if (cCliente->bConectar(argc == 3 ? argv[1] : cHost, argc == 3 ? argv[2]: cPort)) {
#else
		if(cCliente->bConectar(cHost, cPort)){
#endif
			cCliente->iniPacket();
			cCliente->MainLoop();

		} else {
			__DBG_("No se pudo conectar el host");
		}
		if (cCliente->m_isRunning()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(6000)); //Esperar 6 segundos para volver a intentar
		}
	}

	delete cCliente;
	cCliente = nullptr;

	WSACleanup();
	return 0;
}
