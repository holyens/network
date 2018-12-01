all: server client

server: server.c
	cc -o server server.c -lpthread
client: client.c
	cc -o client client.c -lpthread
clean:
	rm server client

