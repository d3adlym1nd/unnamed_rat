LINK=-pthread -lssl -lcrypto
CFLAG=-Wall -Wextra -std=c++11 -pedantic -g
OBJ=main.o Misc.o Server.o
BIN=server
CC=g++
RM=/bin/rm

%.o: %.cpp
	$(CC) $(CFLAG) -c -o $@ $<
$(BIN): $(OBJ)
	$(CC) $(CFLAG) -o $(BIN) $(OBJ) $(LINK)
clean:
	$(RM) -f *.o $(BIN)


