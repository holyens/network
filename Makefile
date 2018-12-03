all: server client httpserver

cs: server client

server: server.c
	cc -o server server.c -lpthread
client: client.c
	cc -o client client.c -lpthread
httpserver: httpserver.c
	cc -o httpserver httpserver.c -lpthread #-std=gnu99

clean:
	rm -f server client httpserver

