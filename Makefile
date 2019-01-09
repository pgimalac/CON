CC=gcc
CFLAGS=-Wall -Wextra -std=c11
LDFLAGS=-lSDL2 `sdl2-config --cflags --libs`
FILE=src/
SRC=$(wildcard $(FILE)*.c)

OBJ=$(SRC:.c=.o)
EXEC=con

all: $(SRC) $(EXEC)

%.o: %.c
	@echo "Compiling $?"
	@$(CC) $(CFLAGS) -c $? -o $@ $(LDFLAGS)

$(EXEC): $(OBJ)
	@echo "Build executable $(EXEC)"
	@$(CC) $(CFLAGS) $(OBJ) -o $(EXEC) $(LDFLAGS)

run: $(EXEC)
	@echo "Executing $?"
	@./$(EXEC)

fclean: clean
	@echo "Clean executable $(EXEC)"
	@rm -rf $(EXEC)

clean:
	@echo "Clean objects"
	@rm -rf $(OBJ)

re: clean all
