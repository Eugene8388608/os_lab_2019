CC=gcc
ALL=mutex real_mutex parallel_factorial deadlock

all : $(ALL)

mutex:
	$(CC) -o mutex mutex.c -lpthread

real_mutex:
	$(CC) -o real_mutex real_mutex.c -lpthread

parallel_factorial:
	$(CC) -o parallel_factorial parallel_factorial.c -lpthread

deadlock:
	$(CC) -o deadlock deadlock.c -lpthread

.PHONY : clean
clean :
	rm -f $(ALL)