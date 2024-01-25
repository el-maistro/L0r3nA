#include "misc.hpp"

std::string RandomID(int iLongitud){
    const char *Map = "_a1b2c3d4e5f6g7h8i9j0";
    std::string strSalida = "";
    srand((unsigned) time(nullptr));
    for(int i=0; i<iLongitud; i++){
        Sleep(44);
        strSalida += Map[1 + (rand() % 20)];
    }
    return strSalida;
}
