all: my_userid my_storage_use my_shells

CC = gcc
CFLAGS = -Wall -g -Werror

my_userid: my_userid.o
	$(CC) $(CFLAGS) -o my_userid my_userid.o

my_storage_use: my_storage_use.o
	$(CC) $(CFLAGS) -o $@ $^

my_shells: my_shells.c
	$(CC) $(CFLAGS) -o $@ $^

my_userid.o: my_userid.c
	$(CC) $(CFLAGS) -c my_userid.c

my_storage_use.o: my_storage_use.c
	$(CC) $(CFLAGS) -c my_storage_use.c

my_shells.o: my_shells.c
	$(CC) $(CFLAGS) -c my_shells.c

make clean:
	rm -f *.o *.txt my_userid my_storage_use my_shells
