CC=gcc
CFLAGS=-I.
OBJECTS=utils.o find_min_max.o
ALL=sequential_min_max parallel_min_max exec_wrapper

all : $(ALL)

sequential_min_max : $(OBJECTS) utils.h find_min_max.h
	$(CC) -o sequential_min_max $(OBJECTS) sequential_min_max.c $(CFLAGS)

parallel_min_max : $(OBJECTS) utils.h find_min_max.h
	$(CC) -o parallel_min_max $(OBJECTS) parallel_min_max.c $(CFLAGS)

exec_wrapper : sequential_min_max
	$(CC) -o exec_wrapper exec_wrapper.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

.PHONY : clean
clean :
	rm -f $(OBJECTS) $(ALL)
