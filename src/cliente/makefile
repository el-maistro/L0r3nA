LINK=-lws2_32 -Wl,-Bstatic -s -w -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libgcc -Wl,-Bstatic
CFLAG=-Wall -Wextra -std=c++11 -pedantic
OBJ=main.o misc.o cliente.o
BIN=cliente.exe
CC=x86_64-w64-mingw32-g++.exe
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) *.o $(BIN)


