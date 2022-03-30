
#include <arpa/inet.h> 
#include <grp.h>
#include <pwd.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <strings.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <tar.h> 
#include <unistd.h> 

#include "mytar.h" 
#include "options.h"
#include "debug.h"

char *joinname(char *prefix, char *name, char *buffer, int size){
    int plen = 0, nlen =0; 
    char *res; 

    res = buffer; 
    buffer[0] = '\0'; 
    if( prefix ){
        plen = strlen(prefix); 
    }
    if (name) {
        nlen = strlen(name); 
    }
    if( plen + nlen + 1 <= size){
        if( plen){
            strcpy(buffer, prefix); 
            if( buffer[strlen(buffer) - 1] != '/'){
                strcat(buffer, "/");
            }
            strcat(buffer, name); 
        }
    }else{
        res = NULL; 
    }
    return res;    
}

int split_name(char *path, char *prefix, char *name){
    int ok = TRUE;
    int len, plen; 
    char *here, *splitpoint; 
    len = strlen(path); 
    if( len > MT_MAX_NAME) {
        ok = FALSE; 
    }else{
        if( len <= MT_NAMELEN){
            strcpy(name, path); 
            prefix[0] = '\0'; 
        }else{
            here = path + len - MT_NAMELEN - 1;
            splitpoint = index(here, '/');
            if( splitpoint ){
                plen = splitpoint - path; 
                if (plen <= MT_PFXLEN){
                    memmove(prefix, path, plen);
                    if(plen < MT_PFXLEN)
                        prefix[plen] = '\0'; 
                    strcpy(name, splitpoint + 1); 
                }else{
                    ok = FALSE; 
                }
            }else{
                ok = FALSE; 
            }
        }
    }
    return ok; 
}

char *build_name (char *path, char *tail, char *buffer, int size){
    char *res; 
    char pfx[MT_PFXLEN + 1]; //, rest[MT_NAMELEN + 1];
    res = joinname(path, tail, buffer, size); 
    if( res){
        if( !split_name(res, pfx, res)) {
            res = NULL; 
        }
    }
    return res;
}

static uint32_t extract_special_int ( char *where, int len){
    /*interoperability with GNU tar */
    int32_t val = -1; 
    if ( (len >= sizeof(val)) && (where[0] & 0x80)) {
        val = *(int32_t *)(where + len - sizeof(val)); 
        val = ntohl (val); 
    }
    return val; 
}

static int insert_special_int( char *where, size_t size, int32_t val){
    int err = 0; 
    if (val < 0 || (size < sizeof(val))){
        err++;
    }else{
        memset(where, 0, size);
        *(int32_t*) (where + size - sizeof(val)) = htonl(val); 
        *where |= 0x80; 
    } 
    return err;
}

int do_common_header(struct tar_header *hdr, char *name, struct stat *sb){
    int len, err = 0; 
    char namebuf [MT_MAX_NAME + 1]; 
    struct passwd *pwent; 
    struct group *grent; 
    memset(hdr, 0, sizeof (struct tar_header)); 
    len = strlen(name); 
    if( len <= MT_MAX_NAME){
        strcpy(namebuf, name); 
        if( S_ISDIR(sb -> st_mode)){
           if( len == MT_MAX_NAME){
              fprintf(stderr, "%s: path too long.\n", name);
                err++;
           }else{
                namebuf[len++] = '/'; 
               namebuf[len] = '\0';  
           }
        }
    }else{
        err++; 
    } 
    if( !err ){
        if( split_name(namebuf, hdr -> prefix, hdr -> name)){
            hdr -> mode = (sb -> st_mode & ALL_TAR_MODES); 
            hdr -> uid = sb -> st_uid; 
            hdr -> gid = sb -> st_gid; 
            hdr -> mtime = sb -> st_mtime; 
            strcpy( hdr->magic, TMAGIC);
            
            hdr -> version[0] = TVERSION[0]; 
            hdr -> version[1] = TVERSION[1]; 

            if( (pwent = getpwuid (hdr -> uid)) && 
                    (strlen(pwent->pw_name) < MT_UNAMLEN)){
                strncpy(hdr->uname, pwent->pw_name, MT_UNAMLEN);
            }
            if( (grent = getgrgid(hdr -> gid)) && 
                    (strlen(grent -> gr_name) < MT_UNAMLEN)){
                strncpy(hdr->gname, grent->gr_name, MT_UNAMLEN);
            }
        }else{
            if( opt_verbose > 1){
                fprintf(stderr, 
                        "%s: unable to partition name\n", name);
            }
            err++; 
        }
    }
    return err;
}

static unsigned int compute_checksum( char block[MT_BLOCKSIZE]){
    unsigned int i, sum; 
    sum = 0; 
    for(i = 0; i < MT_BLOCKSIZE; i++ ){
        sum += (unsigned char) block[i]; 
    }
    for( i = 0; i < MT_CSUMLEN; i++ ){
        sum -= block[MT_CSUMOFF + i]; 
        sum += ' '; 
    }
    return sum;  
}

int insert_octal (char *where, size_t size, int val){
    /* convert octal to string, and place it*/
    char buff[13]; 
    int err = 0, len; 
    sprintf(buff, "%0*o", (int)size - 1, val); 
    len = strlen(buff); 
    if(len > size - 1){
        if( opt_strict ){
            if(opt_verbose){
                //eprintf("octal too long. (0%o)\n", val); 
            }
            err++; 
        }else{
            if(insert_special_int(where, size, val)){
                if(opt_verbose)
                    //eprintf("(0%o)\n", val); 
                err++;
            } 
        }
    }else{
        memmove(where, buff, len); 
    }
    return err; 
}

int pack_header(struct tar_header *hdr, char *block){
    /* take header -> put into block*/
    int err = 0; 
    unsigned int checksum; 
    memset(block, 0, MT_BLOCKSIZE); 
    memcpy(block + MT_NAMEOFF, hdr -> name, MT_NAMELEN); 
    err += insert_octal( block + MT_MODEOFF, MT_MODELEN, (int)hdr -> mode);
    err += insert_octal(block + MT_UIDOFF, MT_UIDLEN, (int)hdr -> uid); 
    err += insert_octal(block + MT_GIDOFF, MT_GIDLEN, (int)hdr -> gid); 
    err += insert_octal(block + MT_SIZEOFF, MT_SIZELEN, (int)hdr-> size);
    err += insert_octal(block + MT_MTIMEOFF, MT_MTIMELEN, (int)hdr-> mtime);   
    err += insert_octal(block + MT_CSUMOFF, MT_CSUMLEN, (int)hdr -> chksum);   
    block[MT_TYPEOFF] = hdr -> typeflag; 
    memcpy(block + MT_LINKOFF, hdr -> linkname, MT_LINKLEN);
    memcpy(block + MT_MAGOFF, hdr -> magic, MT_MAGLEN); 
    memcpy(block+MT_VEROFF, hdr -> version, MT_VERLEN); 
    memcpy(block+MT_UNAMEOFF, hdr -> uname, MT_UNAMLEN); 
    memcpy(block+MT_GNAMEOFF, hdr -> gname, MT_GNAMLEN); 
    if(hdr -> typeflag == CHRTYPE || 
            hdr -> typeflag == BLKTYPE){
        err += insert_octal(block+MT_MAJOFF, MT_MAJLEN, hdr->devmajor); 
        err += insert_octal(block+MT_MINOFF, MT_MINLEN, hdr->devminor); 
    }
    memcpy(block + MT_PFXOFF, hdr -> prefix, MT_PFXLEN); 
    checksum = compute_checksum(block); 
    err += insert_octal(block+MT_CSUMOFF, MT_CSUMLEN, checksum);
    return err; 
}

unsigned long extract_octal( char *where, int len, int *err){
    char tmpstring[MT_BLOCKSIZE + 1]; 
    char *end;
    long num; 
    memcpy(tmpstring, where, len); 
    tmpstring[len] = '\0';
    num = strtol(tmpstring, &end, 8); 
    if(*end != '\0' && *end != ' '){
        if(opt_strict)
            (*err)++;
        else{
            num = extract_special_int(where, len); 
            if(num < 0)
                (*err)++; 
        }
    }
    return num; 
}

int unpack_header(struct tar_header *hdr, char block[MT_BLOCKSIZE]){
    /* basically make a header */
    int err = 0; 
    unsigned int checksum= 0; 
    int i; 
    memcpy(hdr -> name, block + MT_NAMEOFF, MT_NAMELEN);
    hdr-> name[MT_NAMELEN] = '\0'; 
    
    hdr -> mode = (mode_t)extract_octal(block + MT_MODEOFF, MT_MODELEN, &err);
    hdr -> uid = (uid_t) extract_octal(block + MT_UIDOFF, MT_UIDLEN, &err); 
    hdr -> gid = (gid_t) extract_octal (block + MT_GIDOFF, MT_GIDLEN, &err); 
    hdr -> size = (size_t) extract_octal(block + MT_SIZEOFF, MT_SIZELEN, &err); 
    hdr -> mtime = (time_t) extract_octal(block+MT_MTIMEOFF, 
            MT_MTIMELEN, &err); 
    hdr -> chksum = (int)extract_octal(block+MT_CSUMOFF, MT_CSUMLEN, &err); 

    hdr -> typeflag = block[MT_TYPEOFF]; 
    memcpy(hdr -> linkname, block + MT_LINKOFF, MT_LINKLEN); 
    hdr -> linkname[MT_LINKLEN] = '\0'; 
    memcpy(hdr -> magic, block + MT_MAGOFF, MT_MAGLEN); 
    memcpy(hdr -> version, block + MT_VEROFF, MT_VERLEN); 
    hdr -> version[MT_VERLEN] = '\0'; 
    
    memcpy(hdr -> uname, block+MT_UNAMEOFF, MT_UNAMLEN);
    memcpy(hdr -> gname, block+MT_GNAMEOFF, MT_UNAMLEN); 

    hdr -> devmajor = extract_octal(block + MT_MAJOFF, MT_MAJLEN, &err); 
    hdr -> devminor = extract_octal(block + MT_MINOFF, MT_MINLEN, &err); 

    memcpy(hdr -> prefix, block + MT_PFXOFF, MT_PFXLEN); 
    hdr -> prefix[MT_PFXLEN] = '\0'; 

    if(strncmp(hdr -> magic, TMAGIC, strlen(TMAGIC))){
        err++;
        if(opt_verbose > 1 ){
            //eprintf("bad magic number: %s\n", hdr->magic); 
        }   
    }
    if(opt_strict){
        if(hdr -> magic[strlen(TMAGIC)] != '\0'){
            err++; 
            if(opt_verbose > 1 ){
                //eprintf("%s\n", hdr -> magic); 
            }
        }

        for(i =0; i < TVERSLEN; i++ ){
            if( hdr -> version[i] != TVERSION[i]){
                err++; 
                if(opt_verbose > 1 ){
                    //eprintf("%c%c\n", hdr -> version[0], hdr -> version[1]); 
                }
                break; 
            }
        }
    }
    checksum = compute_checksum(block); 
    if( checksum != hdr -> chksum ){
        if( !opt_nosum ){
            err++; 
        }
        if( opt_verbose > 1){
            //eprintf("bad %d %d \n", checksum, hdr -> chksum); 
        }
    }

    if( !err && opt_verbose > 1 ){
        fprintf(stderr, "reading header: \n"); 
        debug_print_header(stderr, hdr); 
    }
    return err;
}
/*
#ifdef TESTMAIN 
int main(int argc, char *argv[]){
    char pfx[MT_PFXLEN+1], rest[MT_NAMELEN + 1]; 
    int i; 
    char *name; 
    for(i = 1; i< argc; i++ ){
        name = argv[i]; 
        printf("%s", name); 
        if(split_name(name, pfx, rest)){
            printf("ok\n"); 
        }else{
            printf("nope\n"); 
        }
    }
    return 0; 
}*/
//#endif 
    

