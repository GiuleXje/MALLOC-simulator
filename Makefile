#2024 Pal Roberto Giulio

# compiler
CC = gcc

# compiler flags
CFLAGS = -Wall -Wextra -std=c99 -g

# object files
OBJ = main.o func.o

build: main func
	$(CC) $(OBJ) -o sfl

func: func.c func.h
	$(CC) $(CFLAGS) -c $@.c

main: main.c 
	$(CC) $(CFLAGS) -c $@.c

run_sfl: build
	valgrind --leak-check=full --track-origins=yes ./sfl

clean:
	rm -f *.o sfl

.PHONY: pack clean
