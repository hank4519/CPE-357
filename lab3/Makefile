CC = gcc 
CFLAGS = -Wall -g 
LD = gcc 
LDFLAGS = 

all: htable.o
	$(LD) $(LDFLAGS) -o all htable.o
htable.o: htable.c 
	$(CC) $(CFLAGS) -c -o htable.o htable.c
test: all 
	~pn-cs357/demos/tryLab03
clean: 
	rm *.o all 
