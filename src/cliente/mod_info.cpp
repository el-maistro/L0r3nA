#include "mod_info.hpp"

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