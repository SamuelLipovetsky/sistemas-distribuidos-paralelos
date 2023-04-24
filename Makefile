CC = gcc
CFLAGS = -Wall -g

ex1: main.o spend_time.o
	$(CC) $(CFLAGS) $^ -o $@ -lm 

spend_time.o: spend_time.c spend_time.h
	$(CC) $(CFLAGS) -c spend_time.c -lm
	
main.o: main.c spend_time.h
	$(CC) $(CFLAGS) -c main.c -lm 


clean:
	rm -f ex1 *.o


