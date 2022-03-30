#include <unistd.h> 
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 

#include "blocks.h" 

#define CACHE_THRESHOLD 100 
static blockbuffer buffercache = NULL; 
static int cachecount= 0; 

static blockbuffer new_buffer(){
    blockbuffer new; 
    if (buffercache){
        new = buffercache; 
        buffercache = buffercache -> next; 
        cachecount--; 
    }else if(!(new = malloc(sizeof(struct blockbuffer)))){
        perror("malloc"); 
        exit(2); 
    }
    new -> next = NULL; 
    return new; 
}

static void release_buffer(blockbuffer b){
    if( cachecount >= CACHE_THRESHOLD){
        free(b); 
    }else{
        b -> next = buffercache; 
        cachecount++; 
    }
}

blocklist open_blocklist(int fd, const char *filename){
    blocklist new; 
    if(!(new = malloc(sizeof(struct blocklist)))){
        perror("malloc"); 
        exit(2); 
    }
    new -> fd = fd; 
    new -> filename = filename; 
    new -> block = NULL; 
    return new; 
}

void close_blocklist(blocklist list){
    blockbuffer b;
    for( b= list -> block; b; b = b -> next){
        release_buffer(b); 
    } 
    free(list); 
}

void unread_block(blocklist list, char block [MT_BLOCKSIZE]){
    blockbuffer b; 
    b = new_buffer(); 
    memcpy(b -> data, block, MT_BLOCKSIZE); 
    
    b -> next = list -> block; 
    list -> block = b; 
}

char *read_block(blocklist blist, char block[MT_BLOCKSIZE]){
    int num; 
    char *res = block; 
    blockbuffer b; 
    if(blist -> block){
        b = blist -> block; 
        memcpy(block, b -> data, MT_BLOCKSIZE); 
        blist -> block = blist -> block -> next; 
        release_buffer(b); 
    }else{
        if( (num = read(blist->fd, block, MT_BLOCKSIZE)) != MT_BLOCKSIZE){
            if(num < 0){
                perror("read"); 
            }
            res = NULL; 
        }
    }
    return res;
}
