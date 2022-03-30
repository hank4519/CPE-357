#include "debug.h" 
#include <time.h> 

void debug_print_header(FILE *where, struct tar_header *hdr){
    fprintf(stderr, "%s\n", hdr -> name);  
    fprintf(stderr, "0%04o\n", hdr -> mode); 
    fprintf(stderr, "%d\n", (int) hdr -> uid);  
    fprintf(stderr, "%d\n", (int) hdr -> gid);  
    fprintf(stderr, "%d\n", (int) hdr -> size); 
    fprintf(stderr, "%d --- %s\n", (int)hdr->mtime, ctime(&hdr -> mtime));
    fprintf(stderr, "%d\n", hdr-> chksum);  
     
}

void px(unsigned char *p, int size){
    int i; 
    for(i = 0; i < size; i++ ){
        printf("0x%02x ", p[i]); 
    }
    putchar('\n'); 
}

