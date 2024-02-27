#include "misc.hpp"

std::string RandomTestLen() {
	const char* Map = "abcdefghijklmnopqrstuvwxyz1234567890-@";
	std::string strSalida = "";

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<> dis(0, 37);
	std::uniform_int_distribution<> dis2(10, 40);

	for (int i = 0; i < dis2(gen); i++) {
		int random_number = dis(gen);
		strSalida += Map[random_number];
	}
	return strSalida;
}

std::string RandomID(int iLongitud){
    const char *Map = "abcdefghijklmnopqrstuvwxyz1234567890-";
    std::string strSalida = "";
    
    std::random_device rd;
    std::mt19937 gen(rd()); 
    
    std::uniform_int_distribution<> dis(0, 36);

    for(int i=0; i<iLongitud; i++){
        int random_number = dis(gen);
        strSalida += Map[random_number];
    }
    return strSalida;
}

std::vector<std::string> strSplit(const std::string& strString, char cDelimiter, int iMax) {
	std::vector<std::string> vcOut;
	int istrLen = strString.length(), iIt = 0, iCounter = 0, iTmp = 0;
	for (; iIt < istrLen; iIt++) {
		std::string strTmp = "";
		while (strString[iIt] != cDelimiter && strString[iIt] != '\0') {
			strTmp.append(1, strString[iIt++]);
			iCounter++;
		}
		iCounter = 0;
		vcOut.push_back(strTmp);
		if (++iTmp == iMax) { break; }

	}
	return vcOut;
}