CC=gcc
ALL=tcpclient tcpserver udpclient udpserver

all : $(ALL)

tcpclient:
	$(CC) -o tcpclient tcpclient.c

tcpserver:
	$(CC) -o tcpserver tcpserver.c

udpclient:
	$(CC) -o udpclient udpclient.c

udpserver:
	$(CC) -o udpserver udpserver.c

.PHONY : clean
clean :
	rm -f $(ALL)