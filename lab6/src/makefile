CC=gcc
CFLAGS=-I.

all: client server

client : client.o libmultimodulo.a
	$(CC) client.o -L static -l multimodulo -lpthread -o client $(CFLAGS)

server : server.o libmultimodulo.a
	$(CC) server.o -L static -l multimodulo -lpthread -o server $(CFLAGS)

client.o : 
	$(CC) -c client.c o client.o $(CFLAGS)

server.o : 
	$(CC) -c server.c o server.o $(CFLAGS)

multimodulo.o : 
	$(CC) -c multimodulo.c o multimodulo.o $(CFLAGS)

libmultimodulo.a : multimodulo.o
	ar rcs static/libmultimodulo.a multimodulo.o