CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all: server client pdr maint

maint : maint.o utils_v1.o
	$(CC) $(CCFLAGS) -o maint maint.o utils_v1.o

maint.o: maint.c message.h
	$(CC) $(CCFLAGS) -c maint.c

pdr : pdr.o utils_v1.o
	$(CC) $(CCFLAGS) -o pdr pdr.o utils_v1.o

pdr.o : pdr.c message.h
	$(CC) $(CCFLAGS) -c pdr.c

server : server.o utils_v1.o
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o

server.o: server.c utils_v1.o message.h
	$(CC) $(CCFLAGS) -c server.c 

client : client.o utils_v1.o
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o

client.o: client.c message.h
	$(CC) $(CCFLAGS) -c client.c


utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c 


clean :
	clear

clear:
	rm -rf *.o
	rm -rf server
	rm -rf client
	rm -rf maint
	rm -rf pdr