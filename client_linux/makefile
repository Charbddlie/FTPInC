CC = gcc
CFLAGS = -g
OBJS = client.o ../common/common.o
all : client

client : $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY:
clean:
	rm -f *.o client
	rm -f ../common/*.o
