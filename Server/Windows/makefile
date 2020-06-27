LINK=-pthread -lssl -lcrypto -lws2_32 -static-libgcc -static-libstdc++
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g
OBJ=main.o Misc.o Server.o
BIN=server.exe
CC=x86_64-w64-mingw32-g++
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<
$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) -f *.o $(BIN)


