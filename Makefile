CC = gcc
CFLAGS = -g -ansi -Wall -Wextra -pedantic
LIBS = -lrt -pthread
OBJS = queue.o tools.o

all: main.out

%.out: %.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf *.o

purge: clean
	rm -rf *.out
