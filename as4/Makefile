all: myls

CC = gcc
CFLAGS = -Wall -g -Werror
# LIBS = -pthread -lm

myls: myls.o
	$(CC) $(CFLAGS) -o myls myls.o $(LIBS)

myls.o: myls.c
	$(CC) $(CFLAGS) -c myls.c

make clean:
	rm -f *.o myls