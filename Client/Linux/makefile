#-lz -ldl -static-libgcc
LINK=-pthread -lssl -lcrypto
<<<<<<< HEAD
CFLAG=-Wall -Wextra -std=c++11 -pedantic
OBJ=main.o Misc.o Cipher.o Client.o HttpDownload.o Info.o
=======
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g
OBJ=main.o Misc.o Client.o HttpDownload.o Info.o
>>>>>>> 5c7331a580c9a45d63ab8edb0e8b1657c99a51c4
BIN=Client
CC=g++
RM=/bin/rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) $(CFLAG) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) -f *.o $(BIN)


