CC = gcc 
CFLAGS = -c -g -Wall 
LD = gcc
LDFLAGS = -g -Wall

mush2: mush2.c
	$(CC) $(CFLAGS) -I ~pn-cs357/Given/Mush/include mush2.c 
	$(LD) $(LDFLAGS) -L ~pn-cs357/Given/Mush/lib64 -o mush2 mush2.o -lmush

clean: 
	rm *.o mush2
