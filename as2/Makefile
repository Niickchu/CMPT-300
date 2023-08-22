all: cshell

CC = gcc
CFLAGS = -Wall -g -Werror

cshell: cshell.o commands.o
	$(CC) $(CFLAGS) -o cshell cshell.o commands.o

cshell.o: cshell.c
	$(CC) $(CFLAGS) -c cshell.c

commands.o: commands.c
	$(CC) $(CFLAGS) -c commands.c

make clean:
	rm -f *.o cshell
