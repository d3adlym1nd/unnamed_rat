LINK=-pthread -lssl -lcrypto
CFLAG=-Wall -Wextra -std=c++11 -pedantic
OBJ=main.o Misc.o Client.o HttpDownload.o Info.o
BIN=Client
CC=g++
RM=/bin/rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) $(CFLAG) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) -f *.o $(BIN)

