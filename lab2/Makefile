CC = gcc 
CFLAGS = -Wall -g 
LD = gcc 
LDFLAGS = 

all: uniq.o 
	$(LD) $(LDFLAGS) -o all uniq.o

uniq: uniq.c 
	$(CC) $(CFLAGS) -c -o uniq.o uniq.c 

test: all
	~pn-cs357/demos/tryLab02 
clean: 
	rm *.o all 
