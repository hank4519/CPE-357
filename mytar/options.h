#ifndef OPTIONH 
#define OPTIONH 

extern int opt_verbose; 
extern int opt_strict; 
extern int opt_mode;
extern int opt_preserve;
extern char *opt_tarfile; 
extern int opt_nosum; 

#define MT_UNKNOWN 0
#define MT_LIST 1 
#define MT_EXTRACT 2 
#define MT_CREATE 3 

#ifndef FALSE 
#define FALSE 0 
#endif 

#ifndef TRUE
#define TRUE 1 
#endif 

void print_options(FILE *where); 
int parse_options(int argc, char *argv[]); 

#endif
