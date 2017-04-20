all: server client
.PHONY: all

server: server.o
client: client.o
	gcc -o $@ $^

.c.o:
	gcc -c $<

clean:
	rm -f *.o client server
