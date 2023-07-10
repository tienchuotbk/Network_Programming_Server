CC = gcc
CFLAGS = -Wall

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c -lmysqlclient -ljansson

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client