LINK=-pthread -lws2_32 -static-libgcc -static-libstdc++ -lwx_baseu-3.2 -lwx_baseu_net-3.2 -lwx_baseu_xml-3.2 -lwx_mswu_adv-3.2 -lwx_mswu_aui-3.2 -lwx_mswu_core-3.2 -lwx_mswu_gl-3.2 -lwx_mswu_html-3.2 -lwx_mswu_media-3.2 -lwx_mswu_propgrid-3.2 -lwx_mswu_qa-3.2 -lwx_mswu_ribbon-3.2 -lwx_mswu_richtext-3.2 -lwx_mswu_stc-3.2 -lwx_mswu_webview-3.2 -lwx_mswu_xrc-3.2
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g `wx-config --cppflags`
OBJ=main.o misc.o Server.o
BIN=Servidor.exe
CC=x86_64-w64-mingw32-g++
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<
$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) -f *.o $(BIN)


#g++ `wx-config --cppflags` wx.cpp -lwx_baseu-3.2 -lwx_baseu_net-3.2 -lwx_baseu_xml-3.2 -lwx_mswu_adv-3.2 -lwx_mswu_aui-3.2 -lwx_mswu_core-3.2 -lwx_mswu_gl-3.2 -lwx_mswu_html-3.2 -lwx_mswu_media-3.2 -lwx_mswu_propgrid-3.2 -lwx_mswu_qa-3.2 -lwx_mswu_ribbon-3.2 -lwx_mswu_richtext-3.2 -lwx_mswu_stc-3.2 -lwx_mswu_webview-3.2 -lwx_mswu_xrc-3.2
