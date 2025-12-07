#ifdef __MOD_RP

#ifndef _MOD_REVERSE_PROXY
#define _MOD_REVERSE_PROXY 1

#define RETRY_COUNT 10

#include "headers.hpp"
#include "mod_dynamic_load.hpp"

namespace TipoRequestHTTP {
	enum Enum {
		GET = 1,
		POST,
		HTTPS
	};
}

struct HTTPRequest {
	std::string strHost;
	std::string strPath;
	std::string strPort;
	int iTipoRequest;
};

class ReverseProxy{
	public:
		ReverseProxy(st_Ws2_32& _ws2_32);
		void m_ProcesarDatosProxy(std::vector<char>& _vcdata, int _recibido);
	private:
		st_Ws2_32 WS32;
		
		std::map<int, SOCKET> map_sockets;

		std::mutex mtx_MapSockets;

		SOCKET m_sckConectar(const char* _host, const char* _puerto);

		std::vector<char> readAllLocal(SOCKET& _socket, int& _out_recibido);
		int sendAllLocal(SOCKET& _socket, const char* _cbuffer, int _buff_size);
		int cSend(SOCKET& _socket, const char* _cbuffer, size_t _buff_size, int _id_conexion);
		
		std::vector<char> strParseIP(const uint8_t* addr, uint8_t addr_type);
		HTTPRequest parseHTTPrequest(const std::vector<char>& _vcdata);

		void th_Handle_Session(int _id_conexion);

		//Tipos de peticiones
		bool isSocksPrimerPaso(const std::vector<char>& _vcdata, int _recibido);
		bool isSocksSegundoPaso(const std::vector<char>& _vcdata, int _recibido);
		bool isHTTP(const std::vector<char>& _vcdata, int _recibido);

		//Mapa de sockets
		SOCKET getLocalSocket(int _id);
		void addLocalSocket(int _id, SOCKET _socket);
		bool eraseLocalSocket(int _id);
		int getSocketID(SOCKET _socket);

};


#endif

#endif