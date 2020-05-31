LINK=-pthread
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g
OBJ=main.o Misc.o Cipher.o Client.o
BIN=Client
CC=g++
RM=/bin/rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<

$(BIN): $(OBJ)
	$(CC) $(CFLAG) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) -f *.o $(BIN)


