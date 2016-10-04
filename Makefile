CC = gcc
CCFLAGS = -g -Wall
LDFLAGS = -lSDL2 -lSDL2_image

BIN = pong
SRC = main.c
INC = 

$(BIN): $(SRC) $(INC)
	$(CC) $(SRC) -o $(BIN) $(CCFLAGS) $(LDFLAGS) 
