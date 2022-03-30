CC = gcc 
CFLAGS = -Wall -g 
LD = gcc
LDFLAGS = 

mypwd: mypwd.o 
	$(LD) $(LDFLAGS) -o mypwd mypwd.o 
mypwd.o: mypwd.c 
	$(CC) $(CFLAGS) -c -o mypwd.o mypwd.c
test: all 

clean: 
	rm *.o all  
