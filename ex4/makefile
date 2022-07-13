RM=rm -f
CC = gcc
CFLAGS = -Wall -g

all: client.out server.out

client.out: ex4_client.c
	$(CC) $(CFLAGS) -o client.out ex4_client.c

server.out: ex4_srv.c
	$(CC) $(CFLAGS) -o server.out ex4_srv.c

clean:
	$(RM) client.out server.out
