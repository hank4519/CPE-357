CC = gcc 
#CFLAGS = 
CFLAGS = -Wall -g 

PROG = mytar 
OBJS = main.o tar.o blocks.o options.o perms.o debug.o toc.o header.o
SRCS = main.c tar.c blocks.c options.c perms.c debug.c toc.c header.c
HDRS = mytar.h options.h blocks.h perms.h debug.h toc.h

all: $(PROG)

allclean: clean 

clean: 
	rm -f $(OBJS) *~TAGS

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

depend: 
	@echo Regenerating local dependencies 
	@makedepend -Y $(SRCS) $(HDRS) > &/dev/null

tags: $(SRCS) $(HDRS)
	etags $(SRCS) $(HDRS)

test:
	$(PROG)

main.o: options.h mytar.h 
tar.o: blocks.h mytar.h options.h perms.h debug.h toc.h
blocks.o: blocks.h mytar.h
options.o: options.h
perms.o: perms.h
debug.o: debug.h mytar.h
toc.o: toc.h mytar.h
blocks.o: mytar.h 
debug.o: mytar.h
toc.o: mytar.h 
