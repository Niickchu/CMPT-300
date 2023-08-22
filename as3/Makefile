all: myChannels

CC = gcc
CFLAGS = -std=c11 -Wall -g -Werror
LIBS = -pthread -lm

myChannels: myChannels.o
	$(CC) $(CFLAGS) -o myChannels myChannels.o $(LIBS)

myChannels.o: myChannels.c
	$(CC) $(CFLAGS) -c myChannels.c

make clean:
	rm -f *.o myChannels