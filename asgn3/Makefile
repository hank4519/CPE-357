CC = gcc 
CFLAGS = -Wall -g
LD = gcc 
LDFLAGS = 


all: hencode hdecode 

hencode: hencode.o htable.o
	$(LD) $(CFLAGS) -o hencode  hencode.o htable.o
hencode.o: hencode.c 
	$(CC) $(CFLAGS) -c hencode.c 

hdecode: hdecode.o htable.o
	$(LD) $(CFLAGS) -o hdecode  hdecode.o htable.o 

hdecode.o: hdecode.c
	$(CC) $(CFLAGS) -c  hdecode.c

htable.o: htable.c htable.h 
	$(CC) $(CFLAGS) -c htable.c 

test: all 
	~pn-cs357/demos/tryAsgn3	
	
clean: 
	rm *.o all 

