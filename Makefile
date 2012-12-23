CC     = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -O3 -g
LIB    = -lm
OBJ    = serial-setup.o main.o
PROG   = dso_serial

all:	$(OBJ)
	$(CC) $(LIB) $(OBJ) -o $(PROG)

serial-setup.o: serial-setup.c
	$(CC) $(CFLAGS) -c serial-setup.c

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f dso_serial $(OBJ)
