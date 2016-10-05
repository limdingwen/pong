CC = gcc
CCFLAGS = -g -Wall
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

BIN = pong
SRC = main.c
INC = 

$(BIN): $(SRC) $(INC)
	$(CC) $(SRC) -o $(BIN) $(CCFLAGS) $(LDFLAGS) 
