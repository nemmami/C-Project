CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all: server client pdr #maint

#maint :
#maint.o:

pdr : pdr.o 
	$(CC) $(CCFLAGS) -o pdr pdr.o utils_v1.o

pdr.o : pdr.c utils_v1.h message.h
	$(CC) $(CCFLAGS) -c pdr.c

server : server.o 
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o

server.o: server.c utils_v1.o
	$(CC) $(CCFLAGS) -c server.c 

client : client.o 
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o

client.o: client.c utils_v1.o
	$(CC) $(CCFLAGS) -c client.c



utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c 


clean :
	clear

clear:
	rm -rf *.o
	
	rm -rf server
	rm -rf client
	# rm -rf maint
	rm -rf pdr