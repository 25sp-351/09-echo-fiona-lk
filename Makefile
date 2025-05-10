CC = gcc
CFLAGS = -Wall -Wextra -pthread
OBJS = echo_server.o

all: echo_server

echo_server: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

echo_server.o: echo_server.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f echo_server $(OBJS)