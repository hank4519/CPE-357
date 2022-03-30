

#include "toc.h"
#include "tar.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 

struct toc{
    toc_entry list; 
};


static toc_entry new_entry(char *path, ino_t ino){
    toc_entry new; 
    if(!(new = malloc(sizeof(struct toc_entry)))){
        perror("malloc");
        exit(3); 
    }
    return new; 
}

void destroy_toc(tar_toc toc){

}

tar_toc create_toc(){
    tar_toc toc; 
    if( !(toc = malloc(sizeof(struct toc)))){
        perror("malloc"); 
        exit(3); 
    }
    toc-> list = NULL; 
    return toc; 
}

void add_to_toc(tar_toc toc, char *name, struct tar_header *hdr, ino_t ino){
    toc_entry new; 
    if( toc){
        new = new_entry(name, ino);
        new -> next = toc -> list; 
        toc -> list = new; 
    }
}


toc_entry get_toc_name(tar_toc toc, char *name){
    toc_entry e = NULL; 
    if(toc){
        for( e = toc -> list; e ; e = e -> next){
            if(!strcmp(name, e -> path))
                break; 
        }
    }
    return e; 
}

toc_entry get_toc_ino(tar_toc toc, ino_t ino){
    toc_entry e = NULL; 
    if( toc) {
        for( e = toc -> list; e ; e = e -> next){
            if(e -> inode == ino)
                break;
        }
    }
    return e ; 
}
