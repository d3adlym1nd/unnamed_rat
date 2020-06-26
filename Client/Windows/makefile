LINK=-lws2_32 -pthread -lssl -lcrypto -lnetapi32
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g
OBJ=main.o Misc.o Client.o HttpDownload.o Info.o
BIN=Client.exe
CC=x86_64-w64-mingw32-g++.exe
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) *.o $(BIN)


