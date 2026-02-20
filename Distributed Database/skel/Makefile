CC=gcc
CFLAGS=-Wall -Wextra

LOAD=load_balancer
SERVER=server
CACHE=lru_cache
UTILS=utils

# Add new source file names here:
LIST=list
DATABASE=database
TASK_QUEUE=task_queue

.PHONY: build clean

build: tema2

tema2: main.o $(LOAD).o $(SERVER).o $(CACHE).o $(UTILS).o $(LIST).o $(DATABASE).o $(TASK_QUEUE).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(CACHE).o: $(CACHE).c $(CACHE).h
	$(CC) $(CFLAGS) $^ -c

$(UTILS).o: $(UTILS).c $(UTILS).h
	$(CC) $(CFLAGS) $^ -c

$(LIST).o: $(LIST).c $(LIST).h
	$(CC) $(CFLAGS) $^ -c

$(DATABASE).o: $(DATABASE).c $(DATABASE).h
	$(CC) $(CFLAGS) $^ -c

$(TASK_QUEUE).o: $(TASK_QUEUE).c $(TASK_QUEUE).h
	$(CC) $(CFLAGS) $^ -c

# $(EXTRA).o: $(EXTRA).c $(EXTRA).h
# 	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
