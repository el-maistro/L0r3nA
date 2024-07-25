#include "zlib.h"
#include<iostream>
#include<cstring>
#include<cstdlib>
#include<vector>

int main(int argc, char** argv){
    unsigned long input_len = strlen(argv[1]);
    unsigned long output_len = input_len * 3;
    unsigned long out_len2 = output_len;

    std::vector<unsigned char> input(input_len);
    std::vector<unsigned char> output(output_len);
    std::vector<unsigned char> output2(output_len);

    memcpy(input.data(), argv[1], input_len);
//ZEXTERN int ZEXPORT compress(Bytef *dest,   uLongf *destLen,
                             //const Bytef *source, uLong sourceLe
//ZEXTERN int ZEXPORT uncompress(Bytef *dest,   uLongf *destLen,
                               //const Bytef *source, uLong sourceLen);
    int err = compress(output.data(), &output_len, input.data(), input_len);

    if(err == Z_OK){
        std::cout<<"Compreso\nOriginal: "<<input_len<<"\nCompreso:"<<output_len<<"\n";
        err = uncompress(output2.data(), &out_len2, output.data(), output_len);
        if(err == Z_OK){
            std::cout<<"Descompreso: "<<out_len2<<"\n";
            std::cout<<output2.data()<<"\n";
        }else if(err == Z_MEM_ERROR) {
            std::cout<<"No habia memoria suficiente\n";
        } else if(err == Z_BUF_ERROR){
            std::cout<<"El output no tiene memoria suficiente\n";
        }
    } else if(err == Z_MEM_ERROR) {
        std::cout<<"No habia memoria suficiente\n";
    } else if(err == Z_BUF_ERROR){
        std::cout<<"El output no tiene memoria suficiente\n";
    }
    return 0;
}