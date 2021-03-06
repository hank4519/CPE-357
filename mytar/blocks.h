#ifndef BLOCKSH 
#define BLOCKSH 

#include "mytar.h" 

typedef struct blockbuffer *blockbuffer; 

struct blockbuffer{
    char data[MT_BLOCKSIZE]; 
    blockbuffer next; 
}; 

typedef struct blocklist *blocklist; 
struct blocklist{
    int fd; 
    const char *filename; 
    blockbuffer block; 
};

blocklist open_blocklist(int fd, const char *filename);
void close_blocklist(blocklist list); 
char *read_block (blocklist list, char block[MT_BLOCKSIZE]); 
void unread_block(blocklist list, char block[MT_BLOCKSIZE]); 

#endif 
