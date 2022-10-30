CC = gcc
CFLAGS = -g
OBJS = server.o ../common/common.o
all : server

server : $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	rm -f *.o server
	rm -f ../common/*.o
