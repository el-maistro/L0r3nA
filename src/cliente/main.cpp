#include "headers.hpp"
//#include "config.hpp"
#include "cliente.hpp"
#include "misc.hpp"

Cliente* cCliente;

#ifdef ___DBG__
int main(int argc, char** argv) {
#else
int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/) {
#endif

	const char* cHost = __CUSTOM_HOST;
	const char* cPort = __CUSTOM_PORT;

	cCliente = new Cliente();

#ifdef TEST_MOD
	cCliente->TEST();
	goto GOTO_Test;
#endif

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

	
GOTO_Test:

	delete cCliente;
	cCliente = nullptr;

	return 0;
}
