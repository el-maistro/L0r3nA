#include "misc.hpp"
#include "frame_client.hpp"
#include "notify.hpp"

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

int RandomID() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1000, 9999);
	return dis(gen);
}

std::string RandomPass(int iLongitud) {
	const char* Map = "ABCDEFGHIJKLMNÑOPQRSTUVWXYZ{[]}/<>-!@$%^&*(.:;)abcdefghijklmnñopqrstuvwxyz1234567890_";
	std::string strSalida = "";

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<> dis(0, 84);

	for (int i = 0; i < iLongitud; i++) {
		int random_number = dis(gen);
		strSalida += Map[random_number];
	}
	return strSalida;
}

std::string TimeToString(unsigned long long ullTime) {
	std::time_t timestamp = ullTime;

	std::tm* timeInfo = std::localtime(&timestamp);

	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeInfo);

	return std::string(buffer);
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

std::vector<std::string> strSplit(const std::string& strString, std::string strDelimited, int iMax) {
	std::vector<std::string> vcOut;
	std::string strTemp = strString;
	while (iMax-- > 0) {
		size_t newpos = strTemp.find(strDelimited);
		if (newpos == std::string::npos) {
			if (strTemp.size() > 0) {
				vcOut.push_back(strTemp);
			}
			break;
		}
		std::string temp = strTemp.substr(0, newpos);
		vcOut.push_back(temp);

		strTemp.erase(0, newpos + strDelimited.size());
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

void ChangeMyChildsTheme(wxWindow* parent, wxColour background, wxColour foreground, wxFont font) {
	parent->SetBackgroundColour(background);
	parent->SetForegroundColour(foreground);
	wxWindowList& children = parent->GetChildren();
	for (wxWindow* child : children) {
		child->SetFont(font);
		child->SetBackgroundColour(background);
		child->SetForegroundColour(foreground);
		//Es un listCtrl
		if (child->IsKindOf(wxCLASSINFO(wxListCtrl))) {
			child->Refresh();
			continue;
		}
		//Es un combobox
		else if (child->IsKindOf(wxCLASSINFO(wxComboBox))) {
			child->Refresh();
			continue;
		}
		child->Refresh();
		ChangeMyChildsTheme(child, background, foreground, font); // Llamada recursiva para subcontroles
	}
}