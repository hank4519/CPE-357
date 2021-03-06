
#ifndef MYTARH 
#define MYTARH 

#include <sys/stat.h> 
#include <sys/types.h> 
#include <tar.h> 
#include <unistd.h> 
#define TSVTX 01000 

#define MT_NAMELEN 100 
#define MT_MODELEN 8 
#define MT_UIDLEN 8
#define MT_GIDLEN 8
#define MT_SIZELEN 12
#define MT_MTIMELEN 12 
#define MT_CSUMLEN 8 
#define MT_TYPELEN 1 
#define MT_LINKLEN 100 
#define MT_MAGLEN 6 
#define MT_VERLEN 2 
#define MT_UNAMLEN 32
#define MT_GNAMLEN 32
#define MT_MAJLEN 8
#define MT_MINLEN 8
#define MT_PFXLEN 155

#define MT_NAMEOFF 0 
#define MT_MODEOFF 100
#define MT_UIDOFF 108
#define MT_GIDOFF 116
#define MT_SIZEOFF 124
#define MT_MTIMEOFF 136
#define MT_CSUMOFF 148
#define MT_TYPEOFF 156
#define MT_LINKOFF 157
#define MT_MAGOFF 257 
#define MT_VEROFF 263 
#define MT_UNAMEOFF 265
#define MT_GNAMEOFF 297
#define MT_MAJOFF 329
#define MT_MINOFF 337
#define MT_PFXOFF 345

#define MT_BLOCKSIZE 512

#define MT_MAX_NAME (MT_NAMELEN + MT_PFXLEN + 1) 

#define MT_DEF_PERMS (S_IRWXU | S_IRWXG | S_IRWXO)
#define MT_ALLX_PERMS (S_IXUSR | S_IXGRP | S_IXOTH)

#define ALL_TAR_MODES (TSUID|TSGID|TSVTX|TUREAD|TUWRITE|TUEXEC|TGREAD\
            |TGWRITE|TGEXEC|TOREAD|TOWRITE|TOEXEC)

struct tar_header{
    char name[MT_NAMELEN + 1];
    mode_t mode; 
    uid_t uid; 
    gid_t gid; 
    size_t size; 
    time_t mtime; 
    int chksum; 
    char typeflag; 
    char linkname[MT_LINKLEN + 1]; 
    char magic [MT_MAGLEN]; 
    char version [MT_VERLEN + 1]; 
    char uname [MT_UNAMLEN]; 
    char gname [MT_GNAMLEN]; 
    int devmajor; 
    int devminor; 
    char prefix [MT_PFXLEN + 1] ;
};

int list_archive(char *filename, char *list[]); 
int extract_archive(char *filename, char *list[]); 
int create_archive(char *filename, char *list[]); 
int read_tarfile(char *filename); 

char *joinname(char *prefix, char *name, char *buffer, int size); 
int do_checksum(unsigned char block [MT_BLOCKSIZE]); 
int unpack_header(struct tar_header *hdr, char block [MT_BLOCKSIZE]); 
int pack_header(struct tar_header *hdr, char *block); 
unsigned long extract_octal (char *where, int len, int *err);
char *build_name(char *path, char *tail, char *buffer, int size); 
int do_common_header(struct tar_header *hdr, char *name, struct stat *sb); 
#endif
/*
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wvariadic-macros\"")
#define eprintf(format,...)\
    do{\
        _Pragma("GCC diagnostic push")      \
        _Pragma("GCC diagnostic ignored \"-Wpedantic\"")     
        fprintf(stderr, "%s: %d" format, _FUNCTION_, _LINE_, _VA-ARGS_);\
        fprintf(stderr, format, _VA_ARGS_);\
        _Pragma("GCC doagnostic pop"); \
    }while(0);
    _Pragma("GCC diagnostic pop")
#endif 
*/
