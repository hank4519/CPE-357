CC = gcc 
CFLAGS = -g -Wall 
LD = gcc
LDFLAGS = 

all: forkit tryit

forkit: forkit.o 
	$(LD) $(LDFLAGS) -o forkit forkit.o
forkit.o: forkit.c
	$(CC) $(CFLAGS) -c forkit.c 

tryit: tryit.o
	$(LD) $(LDFLAGS) -o tryit tryit.o
tryit.o: tryit.c
	$(CC) $(CFLAGS) -c tryit.c 

test: all

clean: 
	rm *.o all


