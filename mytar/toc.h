

#ifndef TOCH
#define TOCH

#include "mytar.h" 

typedef struct toc *tar_toc; 
typedef struct toc_entry *toc_entry;

struct toc_entry{
    char path[MT_MAX_NAME + 1];
    ino_t inode; 

    toc_entry next; 
};

void destroy_toc(tar_toc toc);
tar_toc create_toc(); 
void add_to_toc(tar_toc toc, char *name, struct tar_header *hdr, ino_t ino);
toc_entry get_toc_name(tar_toc toc, char *name);
toc_entry get_toc_ino(tar_toc toc, ino_t ino); 

#define NO_INODE 0

#endif 
