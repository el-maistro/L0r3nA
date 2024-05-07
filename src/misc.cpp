#include "misc.hpp"
#include "frame_client.hpp"

void printHex(const char* data, int length) {
	std::cout << std::hex << std::setfill('0'); // Establece la base hexadecimal y el relleno con ceros

	for (int i = 0; i < length; ++i) {
		// Imprime cada byte en hexadecimal
		std::cout << std::setw(2) << static_cast<int>(static_cast<unsigned char>(data[i])) << " ";
	}

	std::cout << std::dec << std::endl; // Restablece la base decimal
}

std::string RandomID(int iLongitud){
    const char *Map = "abcdefghijklmnopqrstuvwxyz1234567890_";
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

std::string RandomPass(int iLongitud) {
	const char* Map = "ABCDEFGHIJKLMNÑOPQRSTUVWXYZ{'[]'}/<>-!@$%^&*(.:;)abcdefghijklmnñopqrstuvwxyz1234567890_";
	std::string strSalida = "";

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<> dis(0, 86);

	for (int i = 0; i < iLongitud; i++) {
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

u64 StrToUint(const char* strString) {
	u_int uiLen = strlen(strString);
	u_int uiLen2 = uiLen;
	u64 uiRet = 0;
	for (u_int uiIte0 = 0; uiIte0 < uiLen; uiIte0++) {
		u_int uiTlen = 1;
		--uiLen2;
		for (u_int uiIte = 0; uiIte < uiLen2; uiIte++) {
			uiTlen *= 10; //decimal  uiTlen *= 8;  octal
		}
		u_int uiT = strString[uiIte0] - 48;
		uiRet += (uiTlen * uiT);
	}
	return uiRet;
}

u64 GetFileSize(const char* cPath) {
	std::ifstream strmInputFile(cPath, std::ios::binary);
	if (!strmInputFile.is_open()) {
		return 0;
	}
	std::filebuf* pBuf = strmInputFile.rdbuf();
	u64 uTmp = 0;
	uTmp = pBuf->pubseekoff(0, strmInputFile.end, strmInputFile.in);
	pBuf->pubseekpos(0, strmInputFile.in);
	strmInputFile.close();
	return uTmp;
}

int FilterSocket(std::string cID) {
	int npos = cID.find('-', 0);
	return atoi(cID.substr(npos + 1, cID.size()).c_str());
}