
CC = gcc 
CFLAGS = -Wall -g 
LD = gcc 
LDFLAGS =

 
all: fw.o
	$(LD) $(LDFLAGS) -o all fw.o

fw.o: fw.c 
	$(CC) $(CFLAGS) -c -o fw.o fw.c 

test: all
	~pn-cs357/demos/tryAsgn2 
clean: 
	rm *.o all 
