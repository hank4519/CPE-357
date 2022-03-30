CC = gcc
CFLAGS = -Wall -g 
LD = gcc
LDFLAGS =

timeit: timeit.o 
	$(LD) $(LDFLAGS) -o timeit timeit.o 
timeit.o: timeit.c
	$(CC) $(CFLAGS) -c -o timeit.o timeit.c
test: timeit
	~pn-cs357/demos/timeit 5
clean: 
	rm *.o timeit 
