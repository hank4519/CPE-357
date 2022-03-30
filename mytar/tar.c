

#include <dirent.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/stat.h>
#include <sys/types.h> 
#include <tar.h> 
#include <time.h> 
#include <unistd.h> 
#include <utime.h> 
#include "blocks.h" 
#include "mytar.h"
#include "options.h" 
#include "perms.h" 
#include "debug.h" 
#include "toc.h"

#define DATELEN 18

int add_tree_to_archive(int fd, char *path, tar_toc toc);
int add_file_to_archive(int fd, char *fullname, tar_toc toc); 

int create_outfile(char name[MT_NAMELEN + MT_PFXLEN + 2], 
        struct tar_header *hdr){
    char path [MT_NAMELEN + MT_PFXLEN + 2];
    mode_t mode; 
    char *there; 
    int fd = -1;
    int err = 0; 
    strcpy(path, name); 
    
    there = path; 
    while((there = index(there, '/')) && !err){
        *there = '\0'; 
        if(strlen (path)){
            if (-1 == mkdir(path, MT_DEF_PERMS)){
                if(errno != EEXIST) {
                    perror(path); 
                    err++; 
                }
            }
        }
        if( opt_verbose > 2 ){
            //eprintf("5s\n", path); 
        }
        *there++ = '/'; 
    }
    if( !err ){
        if (hdr -> mode & MT_ALLX_PERMS)
            mode = MT_DEF_PERMS; 
        else 
            mode = MT_DEF_PERMS & ~MT_ALLX_PERMS; 

        switch(hdr -> typeflag){
            case REGTYPE:
            case AREGTYPE: 
            case CONTTYPE: 
                fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
                if(fd == -1)
                    perror(path);
                if(opt_verbose > 2)
                    //eprintf("%s\n", path); 
                break; 
            case DIRTYPE: 
                if(path[strlen(path)-1] == '/'){
                    if(-1 == mkdir(path, MT_DEF_PERMS)){
                        if(errno!= EEXIST){
                            perror(path);
                            err++; 
                        }
                    };
                }
                break; 
            case LNKTYPE: 
                if(-1 == link(hdr -> linkname, hdr -> name)){
                    perror(hdr -> name);
                    err++; 
                }
                break;
            case SYMTYPE: 
                if(-1 == symlink(hdr -> linkname, hdr -> name)){
                    perror(hdr -> name); 
                    err++; 
                }
                break; 
            case CHRTYPE:
            case BLKTYPE: 
            case FIFOTYPE: 
        printf("%c\n", hdr -> typeflag);
        }
    }
    return fd; 
}



int write_end_of_archive(int fd){
    int err = 0; 
    char marker[2 * MT_BLOCKSIZE]; 

    memset(marker, 0, 2 * MT_BLOCKSIZE);
    if( (2 * MT_BLOCKSIZE) != write( fd, marker, 2 * MT_BLOCKSIZE)){
        perror("write"); 
        err++; 
    }
    return err; 
}


int end_of_archive (blocklist bl){
    char first[MT_BLOCKSIZE], second[MT_BLOCKSIZE];
    int i, res; 

    if( !read_block(bl, first) || !read_block(bl, second)){
        fprintf(stderr, "%s\n", bl-> filename); 
        res = 1; 
    }
    else{
        res = 1 ; 
        for ( i = 0; i < MT_BLOCKSIZE; i++ ){
            if(first[i] != '\0'){
                res = 0; 
                break; 
            }
        }

        if(!res){
            unread_block(bl, second); 
            unread_block(bl, first); 
        }
    }
    return res; 
}


int file_in_list( char *candidate, char *list[]){
    int res = FALSE; 
    int i, len; 
    if(list){
        if(opt_verbose > 1) {
        
        }
        for( i = 0; list[i] && !res; i++ ){
            len = strlen(list[i]); 
            if(!strncmp (list[i] , candidate, len)){
                if(list[i][len-1] == '/' || 
                        candidate[len] == '/')
                    res = TRUE; 
            }
        }  
        if (opt_verbose > 1 ){
            
        } 

    }   
    return res;
}

int process_entry(blocklist bl, char *list[], int how, tar_toc toc){
    char block[MT_BLOCKSIZE]; 
    char namebuf[MT_MAX_NAME + 1]; 
    char modeline[MODELEN];
    char groupname[MT_GNAMLEN + 1]; 
    char username [MT_UNAMLEN + 1]; 
    char ogstring [MT_UNAMLEN + MT_GNAMLEN + 2]; 
    char datestring[DATELEN + 1]; 
    struct tar_header header; 
    int err = 0; 
    int num; 
    int listall = FALSE ; 
    int copyout; 

    if(!list || !list[0])
        listall = TRUE; 
    if (!read_block(bl, block)){
        err++; 
    }else{
        if(! (num = unpack_header(&header, block))){
            if(opt_verbose > 1){
            
            }
            if(joinname(header.prefix, header.name, namebuf, MT_MAX_NAME)){
                copyout = FALSE; 
                if(listall || file_in_list(namebuf, list)){
                    copyout = TRUE;
                    if (opt_verbose && (how == MT_LIST)){
                        if(strlen(header.uname)){
                            strncpy(username, header.uname, MT_UNAMLEN);
                        }else{
                            snprintf(username, 
                                    MT_UNAMLEN + 1, "%d", header.uid);
                        }
                        if(strlen(header.gname)){
                            strncpy(groupname, header.gname, MT_GNAMLEN);
                        } else{
                            snprintf(groupname, MT_UNAMLEN + 1,
                                    "%d", header.gid); 
                        }
                        snprintf(ogstring, MT_UNAMLEN + MT_GNAMLEN + 2,
                                "%.*s/%.*s", 
                                MT_UNAMLEN, username, 
                                MT_GNAMLEN, groupname); 

                        strftime(datestring, DATELEN, "%Y-%m-%d %H:%M",
                                localtime(&header.mtime)); 
                        printf("%s %-17s %8d %s %s\n",
                                mode2str(modeline, 
                                    header.mode, header.typeflag),
                                ogstring, 
                                (int)header.size, datestring, namebuf); 
                    }else if((how == MT_LIST) || opt_verbose){
                        
                    }
                }
            }else{
                fprintf(stderr, "s\n");
                err++;  
            }
        }else{
            exit(1); 
        }
            if(!err){
                int i, blks; 
                int fd; 
                if(how == MT_EXTRACT && copyout){
                    fd = create_outfile(namebuf, &header); 
                }else{
                    fd = -1; 
                }

                blks = header.size / MT_BLOCKSIZE + 
                    ((header.size % MT_BLOCKSIZE)? 1: 0);
                for(i =0; i< blks; i++ ){
                    if(!read_block(bl, block)){
                        break;
                        err++; 
                    }
                    if(fd != -1){
                        size_t size = MT_BLOCKSIZE; 
                        if( i == blks - 1){
                            if(header.size % MT_BLOCKSIZE)
                                size = header.size % MT_BLOCKSIZE; 
                        }
                        if (-1 == write(fd, block, size)){
                            perror("wtf"); 
                            err++; 
                        }
                    }
                }
                if (fd != -1 ){
                    int res; 
                    struct stat sb; 
                    struct utimbuf ut; 
                    res = fstat(fd, &sb); 
                    close(fd); 

                    if (res!= -1){
                        ut.modtime = header.mtime;
                        ut.actime = sb.st_atime; 
                        res = utime (namebuf, &ut);
                    }
                    if(res == -1){
                        perror(namebuf); 
                        err++; 
                    }
                }

                add_to_toc(toc, namebuf, &header, NO_INODE); 
            }
       
        }
        return err; 
}

int list_archive( char *filename, char *list[]){
    int fd; 
    int err = 0;
    blocklist bl; 
    if( (fd =open(filename, O_RDONLY)) == -1){
        perror(filename); 
        err++; 
    }else{
        bl = open_blocklist(fd, filename); 
    while( !end_of_archive(bl)){
        err += process_entry(bl, list, MT_LIST, NULL); 
    }
    close_blocklist(bl); 
    if( -1 == close(fd)) {
        perror(filename);
        err++;
    }
    }
    return err; 
}


int extract_archive(char *filename, char *list[]){
    int fd; 
    int err= 0; 
    blocklist bl; 
    tar_toc toc; 

    if( -1 == (fd = open(filename, O_RDONLY))){
        perror(filename); 
        err++; 
    }else{
        if(opt_preserve){
            toc = create_toc(); 
        }else{
            toc = NULL; 
        }

        bl = open_blocklist(fd, filename);
        while(!end_of_archive(bl)){
            err += process_entry(bl, list, MT_EXTRACT, toc); 
        }
        close_blocklist(bl); 
        if ( close(fd) == -1){
            perror(filename);
            err++; 
        }        
        if (toc){
            destroy_toc(toc); 
        }
    }
    return err;    
}


int add_file_to_archive(int fd, char *fullname, tar_toc toc){
    struct stat sb; 
    struct tar_header header; 
    char block[MT_BLOCKSIZE]; 
    int file, num, len;
    int err = 0 ;
    if( lstat(fullname , &sb) != -1 ){
        err += do_common_header(&header, fullname, &sb); 
        if(err){
        }else{
            if(S_ISDIR(sb.st_mode)){
                header.typeflag = DIRTYPE; 
                header.size = 0; 
            }else if(S_ISREG(sb.st_mode)){
                header.typeflag = REGTYPE; 
                header.size = sb.st_size; 
            }else if( S_ISLNK(sb.st_mode)){
                header.typeflag = SYMTYPE; 
                len = readlink(fullname, header.linkname, MT_LINKLEN + 1);
                if( len == -1){
                    perror(fullname); 
                    err++;
                }else if(len > MT_LINKLEN){
                    err++; 
                }else{
                    header.linkname[len] = '\0'; 
                }
            }else if(S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) || 
                        S_ISFIFO(sb.st_mode) || S_ISSOCK(sb.st_mode)){
                err++;        
            }else{
                err++; 
            }
        }

        if(pack_header(&header, block)){
            err++; 
        }
        if( !S_ISREG(sb.st_mode)){
            if(MT_BLOCKSIZE != write(fd, block, MT_BLOCKSIZE)){
                err++;
            }
        }else{
            if( (file = open(fullname, O_RDONLY)) == -1 ){
                perror(fullname); 
                err++; 
            }
            else{
                if(MT_BLOCKSIZE != write(fd, block, MT_BLOCKSIZE)){
                    err++;
                }else{
                    num = 0;
                    while(header.size && (num = read(file, block,
                                    MT_BLOCKSIZE)) > 0){
                        if(num != MT_BLOCKSIZE){
                            memset(block+num, 0, MT_BLOCKSIZE -num);    
                        }
                        if(MT_BLOCKSIZE != write(fd, block, MT_BLOCKSIZE)){
                            err++; 
                        }
                    }
                    if(num < 0){
                        err++; 
                    }
                    
            }
            close(file); 
            }
        }
      }
    if(!err && opt_verbose){
        printf("%s%s\n", fullname, (S_ISDIR(sb.st_mode) ? "/":"")); 
    }
    return err; 
}

int add_tree_to_archive(int fd, char *path, tar_toc toc){
    struct stat sb; 
    DIR *subd; 
    struct dirent *ent; 
    int len, err = 0 ;
    char namebuf[MT_MAX_NAME + 1] ;
    len = strlen(path);
    if(path[len - 1] == '/')
        path[len -1] = '\0'; 
    err += add_file_to_archive( fd, path, toc); 
    if( !(subd = opendir (path))){
        perror(path); 
    }else{
        while((ent = readdir(subd))){
            if( !strcmp(".", ent-> d_name) || !strcmp("..", ent-> d_name)){
                continue; }
            if (build_name(path, ent-> d_name, namebuf, MT_MAX_NAME)){
                if( lstat(namebuf, &sb) == -1){
                    perror(namebuf); 
                    err++; 
                }else{
                    if(S_ISDIR(sb.st_mode))
                        err+= add_tree_to_archive(fd, namebuf, toc);
                    else
                       err += add_file_to_archive(fd, namebuf, toc);  
                }
            }else{
                fprintf(stderr, "som  %s \n", ent-> d_name);
            }
        }
        closedir(subd); 
    }
    return err; 
}



int create_archive(char *filename, char *list[] ){
    int err= 0; 
    int i, fd ;
    tar_toc toc; 
    struct stat sb; 
    if( -1 == (fd = open(filename, O_WRONLY| O_CREAT | O_TRUNC,
                    (MT_DEF_PERMS & ~MT_ALLX_PERMS)))){
        perror(filename); 
        err++;    
    }
    toc = create_toc(); 

    for(i = 0; list [i]; i++ ){
        if(lstat(list[i], &sb) != -1){
            if( S_ISDIR(sb.st_mode)){
                err+= add_tree_to_archive(fd, list[i], toc);
            }
            else{
                err += add_file_to_archive(fd, list[i], toc); 
            }
        }else{
            perror(list[i]);
            err++; 
        }
    }

    err += write_end_of_archive(fd); 
    if(toc){
        destroy_toc(toc); 
    }

    if(-1 == close(fd)){
        perror(filename); 
        err++; 
    }

    return err;

}
