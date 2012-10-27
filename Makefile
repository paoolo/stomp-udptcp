CC = gcc
CFLAGS = -g -ansi -Wall -Wextra -pedantic
LIBS = -pthread
OBJS = map.o queue.o tools.o udp_connection.o tcp_connection.o

all: main.out

%.out: %.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

clean:
	rm -rf *.o

purge: clean
	rm -rf *.out
