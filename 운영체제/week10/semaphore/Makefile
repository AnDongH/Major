CC=gcc
CFLAGS=-g -o
TARGETS=print_semaphore
all: $(TARGETS)
.PHONY: all
%:
	$(CC) $(CFLAGS) $@ $@.c -lpthread
       
clean:
	rm $(TARGETS)
