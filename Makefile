CFLAGS = -Wall -Werror


all: server client

server: server.c
	gcc $(CFLAGS) -o $@ $<

client: client.c
	gcc $(CFLAGS) -o $@ $<


clean:
	rm -rf server client
