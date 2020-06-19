#-lz -ldl -static-libgcc
LINK=-lws2_32 -pthread -lssl -lcrypto
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g
OBJ=main.o Misc.o Client.o HttpDownload.o
BIN=Client.exe
CC=g++
RM=rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) *.o $(BIN)


