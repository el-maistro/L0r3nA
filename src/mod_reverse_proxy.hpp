#pragma once

#ifndef _MOD_PROXY
#define _MOD_PROXY 1
#define RETRY_COUNT 10
#include "headers.hpp"

class ReverseProxy {
	public:
		ReverseProxy();

		void InitHandler(int _puerto, SOCKET _socket); // invocar thread para esperar por peticiones locales en X puerto
		void StopHandler(int _puerto);

		void procRespuestaProxy(int _recibido, const std::vector<char>& _vcdata);
	private:
		//Mapa global que almacena cada socket
		std::map<int, SOCKET> map_sockets;
		std::map<int, bool> map_is_proxy_running;

		std::mutex mtx_MapSockets;
		std::mutex mtx_MapProxyRunning;

		WSADATA wsa;
		int iPuertoEscucha = 6666;

		SOCKET sckLocalSocket = INVALID_SOCKET;

		void EsperarConexiones(int _puerto, SOCKET _socket); // thread para esperar por conexiones en X puerto
	
		bool  m_InitSocket(SOCKET& _socket, int _puerto);
		SOCKET m_Aceptar(SOCKET& _socket);
		std::vector<char> readAllLocal(SOCKET& _socket, int& _out_recibido);
		int sendAllLocal(SOCKET& _socket, const char* _cbuffer, int _buff_size, bool dbg = false);
		int cSend(SOCKET& _socket, const char* _cbuffer, size_t _buff_size, int _id_conexion);
	
		bool isRespuestaSegundoPaso(const std::vector<char>& _vcdata, int _recibido);
		void th_Handle_Session(SOCKET _socket_proxy_remoto, int _id_conexion, SOCKET _socket_local);

		std::string strTestBanner();

		//Mapa de sockets activos
		SOCKET getLocalSocket(int _id);
		void addLocalSocket(int _id, SOCKET _socket);
		bool eraseLocalSocket(int _id);
		int getSocketID(SOCKET _socket);

		//Mapa de lista de proxys activos
		bool isProxyRunning(int _puerto);
		void addProxyRunning(int _puerto, SOCKET _socket);
};

#endif
