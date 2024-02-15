#include "misc.hpp"

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
