CC=gcc
CFLAGS=-I.
OBJECTS=find_sum.o ../../lab3/src/utils.o
ALL=zombie process_memory parallel_sum

all : $(ALL)



zombie :
	$(CC) -o zombie zombie.c

process_memory :
	$(CC) -o process_memory process_memory.c -Wno-implicit-int -Wno-format



parallel_sum : $(OBJECTS) find_sum.h ../../lab3/src/utils.h
	$(CC) -o parallel_sum $(OBJECTS) parallel_sum.c -lpthread $(CFLAGS)

find_sum.o : find_sum.h
	$(CC) -o find_sum.o -c find_sum.c $(CFLAGS)



.PHONY : clean
clean :
	rm -f $(OBJECTS) $(ALL)
