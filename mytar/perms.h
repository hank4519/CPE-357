#ifndef PERMSH 
#define PERMSH
#include <sys/types.h> 
#include <sys/stat.h> 
#include <unistd.h> 

#define MODELEN 11 
char *mode2str(char perms[MODELEN], mode_t mode, char typeflag); 
#endif
