CC = gcc 
CFLAGS = -Wall -g 
LD = gcc
LDFLAGS = 

mytalk: mytalk.c 
	gcc -c -g -Wall -I ~pn-cs357/Given/Talk/include mytalk.c 
	gcc -g -Wall -L ~pn-cs357/Given/Talk/lib64 -o mytalk mytalk.o -ltalk -lncurses

test: mytalk
	~pn-cs357/demos	
clean: 
	rm *.o mytalk
