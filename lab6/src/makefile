CC=gcc
OBJECTS=utils.o
ALL=client server

all : $(ALL)

client: $(OBJECTS)
	$(CC) -o client $(OBJECTS) client.c -lpthread

server: $(OBJECTS)
	$(CC) -o server $(OBJECTS) server.c -lpthread

utils.o : utils.h
	$(CC) -o utils.o -c utils.c

.PHONY : clean
clean :
	rm -f $(ALL) $(OBJECTS)