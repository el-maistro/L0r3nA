LINK2=-lmfplat -lmfreadwrite -lmfuuid -lole32 -lmf -lwmcodecdspuuid -lstrmiids -luuid -lws2_32 -lwinmm -lwtsapi32 -lgdi32 -lgdiplus -lshlwapi -Wl,-Bstatic -s -w -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libgcc -Wl,-Bstatic
LINK=-lmfplat -lmfreadwrite -lmfuuid -lole32 -lmf -lwmcodecdspuuid -lstrmiids -luuid -lws2_32 -lwinmm -lwtsapi32 -lgdi32 -lgdiplus -lshlwapi
CFLAG=-Wall -Wextra -std=c++14 -Wpedantic
OBJ=main.o misc.o cliente.o mod_mic.o mod_file_manager.o mod_keylogger.o mod_camara.o mod_remote_desktop.o
#ZLIB= ../zlib/compress.o ../zlib/inflate.o ../zlib/infback.o ../zlib/inftrees.o ../zlib/trees.o ../zlib/inffast.o ../zlib/adler32.o ../zlib/deflate.o ../zlib/crc32.o ../zlib/zutil.o ../zlib/uncompr.o
BIN=cliente.exe
CC=x86_64-w64-mingw32-g++.exe
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

zlib/%.o: ../zlib/%.c
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN):aes256.o $(OBJ) #$(ZLIB)
	#$(CC) -o $(BIN) $(OBJ) $(ZLIB) aes256.o $(LINK2)
	$(CC) -o $(BIN) $(OBJ) aes256.o $(LINK2)

aes256.o: ../aes256/aes256.cpp
	$(CC) $(CFLAG) -c -o $@ $<
	


clean:
	$(RM) *.o $(BIN)

