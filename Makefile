CC=gcc
CFLAGS=-g -Wall -Wextra -std=gnu99
LDFLAGS=-lSDL2 `sdl2-config --cflags --libs` -lpthread -lX11
FILE=src/
SRC=$(wildcard $(FILE)*.c)

OBJ=$(SRC:.c=.o)
EXEC=con

all: $(SRC) $(EXEC)

%.o: %.c
	@echo "Compiling $?"
	@$(CC) $(CFLAGS) -c $? -o $@

$(EXEC): $(OBJ)
	@echo "Build executable $(EXEC)"
	@$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

fclean: clean
	@echo "Clean executable $(EXEC)"
	@rm -rf $(EXEC)

clean:
	@echo "Clean objects"
	@rm -rf $(OBJ)

re: clean all
