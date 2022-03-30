

#include <sys/stat.h> 
#include <stdio.h> 
#include <tar.h> 
#include "perms.h" 

static char suidbit(mode_t mode, int xflag){
    char res; 
    if( (mode & S_ISUID) && xflag)
        res = 's'; 
    else if( (mode & S_ISUID))
        res = 'S'; 
    else if(xflag)
        res = 'x'; 
    else 
        res = '-'; 
    return res;
}

static char sgidbit(mode_t mode, int xflag){
    char res; 
    if((mode & S_ISGID) && xflag)
        res = 's'; 
    else if((mode & S_ISGID))
        res = 'S'; 
    else if (xflag)
        res = 'x'; 
    else 
        res = '-'; 
    return res; 
}

static char stickybit(mode_t mode, int xflag){
    char res;
    if ((mode & S_ISVTX) && xflag)
        res = 't'; 
    else if(mode & S_ISVTX)
        res = 'T'; 
    else if(xflag)
        res = 'x'; 
    else 
        res = '-'; 
    return res; 
}

char *mode2str(char perms[MODELEN], mode_t mode, char typeflag){
    char type; 
    if((typeflag == REGTYPE) || (typeflag == AREGTYPE))
        type = '-';
    else if(typeflag == LNKTYPE)
        type = 'h';
    else if(typeflag == SYMTYPE)
        type = 'l'; 
    else if(typeflag == BLKTYPE)
        type = 'b'; 
    else if(typeflag == CHRTYPE)
        type = 'c';
    else if(typeflag == DIRTYPE)
        type = 'd'; 
    else if(typeflag == FIFOTYPE)
        type = 'p';
    else
        type = '?'; 

    sprintf(perms, "%c%c%c%c%c%c%c%c%c%c", 
            type,
            (mode & S_IRUSR) ? 'r': '-',
            (mode & S_IWUSR) ? 'w': '-',
            suidbit(mode, (mode & S_IXUSR)),
            (mode & S_IRGRP) ? 'r': '-',
            (mode & S_IWGRP) ? 'w': '-',
            sgidbit(mode, (mode & S_IXGRP)),
            (mode & S_IROTH) ? 'r': '-',
            (mode & S_IWOTH)? 'w': '-',
            stickybit(mode, (mode & S_IXOTH ))
    ); 
    return perms;
}   
