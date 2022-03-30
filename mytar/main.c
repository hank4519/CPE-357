
/* 
Author: Dr. Phillip Nico, not me


*/ 

#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 

#include "options.h"
#include "mytar.h" 

int main(int argc, char *argv[]){
    int i; 
    int errors = 0; 
    /* i = index of first non-option argument*/
    i = parse_options(argc, argv);
    if( opt_verbose > 1){
        print_options(stderr); 
    }
   switch(opt_mode){
    case MT_LIST: 
       errors += list_archive(opt_tarfile, argv + i); 
        break; 
    case MT_EXTRACT: 
       errors += extract_archive(opt_tarfile, argv + i); 
      break;
    case MT_CREATE:
        errors += create_archive(opt_tarfile, argv + i); 
       break;
    default: 
        fprintf(stderr, "unknown mode: %s\n", argv[0]);
        errors++; 
   }
   return errors;  
}
