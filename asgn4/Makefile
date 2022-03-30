CC = gcc 
CFLAGS = -Wall -g 
LD = gcc
LDFLAGS = 

mytar: mytar.o
	$(LD) $(LDFLAGS) -o mytar mytar.o 
mytar.o: mytar.c 
	$(CC) $(CFLAGS) -c -o mytar.o mytar.c 
test: mytar 
	~pn-cs357/demos/tryAsgn4
clean: 
	rm *.o mytar
