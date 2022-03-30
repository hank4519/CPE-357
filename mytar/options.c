
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include "options.h" 

int opt_verbose = 0; 
int opt_strict = FALSE; 
int opt_mode = MT_UNKNOWN; 
int opt_preserve = FALSE; 
char *opt_tarfile = NULL; 
int opt_nosum = FALSE; 

void print_options(FILE *where){
    fprintf(where, "opt_verbose = %d\n", opt_verbose); 
}

static void usage_exit(char *name){
    char *usage = "usage: %s [ctxvSpn[f tarfile]] [file1 [file2 [...] ]]\n";
    fprintf(stderr, usage, name); 
    exit(EXIT_FAILURE); 
}   

int parse_options(int argc, char *argv[]){
    /*parse command, and set parameters */
    char c; 
    int i; 
    int err = 0 ;
    int toomany = 0; 
    int optind = 2; 
    if (argc < 2 ){
        fprintf(stderr, "%s: at least one of 'ctx'. \n", argv[0]);
        usage_exit(argv[0]);
    }
    for(i = 0; i < strlen(argv[1]); i++ ){
        c = argv[1][i]; 
        switch(c){
            case'v': 
                opt_verbose++; 
                break;
            case 'n': 
                opt_nosum++; 
                break;
            case 'p': 
                opt_preserve = TRUE; 
                break;
            case 'S': 
                opt_strict = TRUE;
                break;
            case 'f': 
                if(argv[2] != NULL){
                    opt_tarfile = argv[2];
                    optind++; 
                }else{
                    fprintf(stderr, "%s: 'f' archi name \n", argv[0]);
                    err++; 
                }
                break; 
            case 't': 
                if(opt_mode == MT_UNKNOWN){
                    opt_mode = MT_LIST; 
                }else{
                    toomany++; 
                    err++; 
                }
                break; 
            case 'x': 
                if(opt_mode == MT_UNKNOWN){
                    opt_mode = MT_EXTRACT; 
                }else{
                    toomany++; 
                    err++; 
                }
                break; 
            case 'c': 
                if( opt_mode == MT_UNKNOWN){
                    opt_mode = MT_CREATE; 
                }else{
                    toomany++; 
                    err++; 
                }
                break; 
            default: 
                fprintf(stderr, "%s: unrecognized option '%c'.\n",
                        argv[0], c); 
                err++; 
                break; 
        }
    }
   if(toomany){
        fprintf(stderr, "%s: you may only choose one of the 'ctx' \n", 
               argv[0]); 
   }else if( opt_mode == MT_UNKNOWN){
        fprintf(stderr, "%s: you must choose one of the 'ctx'\n", 
               argv[0]);
       err++;  
   } 
   if( err){
       usage_exit(argv[0]); 
   }
   return optind; 
}



