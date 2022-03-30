#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <dirent.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <pwd.h> 
#include <grp.h>
#include <time.h>  
#include <utime.h> 

#define INS_MAX 6 
#define BLOCK 512
#define NAME_BYTE 100 
#define PREFIX_BYTE 155
#define LINK_BYTE 100
#define OWNER_BYTE 17
#define HUNDRED 100
#define EIGHT 8 
#define TWELVE 12
#define SIX 6 
#define TW 32 

/*structure for header*/ 
typedef struct Header{
	char name [100]; 
	char mode [8]; 
	char uid [8]; 
	char gid [8]; 
	char size [12]; 
	char mtime [12];
	char chksum[8]; 
	char typeflag; 
	char linkname [100]; 
	char magic[6]; 
	char version[2]; 
	char uname [32]; 
	char gname [32];
	char devmajor [8]; 
	char devminor [8]; 
	char prefix [155]; 
}header;

/*structure for listing */
typedef struct Listing{
	char permissions[10]; 
	char owner_group[17]; 
	//char size[8];
	long int size; 
	char mtime[16];
	char *name; 
}listing; 

void process_path(FILE *out, struct stat st, 
	header *hd, char *path, int v_flag, int S_flag); 
int convert_octal (char size[]); 

/*Sort the option argument ctxvSf */
void sort_instruction(char str[]){
char temp; 
int i, j, n; 
n = strlen (str); 
for(i = 0; i < n; i++ ){
	for(j = 0; j < n - 1; j++ ){
		if(str[j] > str[j+1]){
			temp = str[j]; 
			str[j]= str[j+1]; 
			str[j+1] = temp; 
		}
	}
    }
}
/*adding the bytes of fields into checksum */
int check_sum(char var[], int len){
	unsigned int ans; 
	int i;
	ans = 0;  
	for (i =0; i < len; i++){
	   ans += var[i]; 
	}
	return ans; 
}
/*convert decimal number to octal number */
int convert_decimal( int decimal){
	int octal =0, tmp = 1; 
	while (decimal != 0){
		octal += (decimal % 8) * tmp; 
		decimal /= 8; 
		tmp *= 10; 
	}
	return octal; 
}

/*generate the header */
header* get_header (char *path, struct stat *st, int S_flag){
header *hd;
int len;
unsigned int chksum;  
char *subtext = "";
struct passwd *u_pd; 
struct group *g_gr;
//unsigned char *curr;
hd = (header*) malloc(sizeof(header)); 
/*check back, this header may not be initialized */ 
/*name field*/ 
chksum = 0; 
len = strlen(path);  
if(len > NAME_BYTE + PREFIX_BYTE){
	perror("Path name is too long for 'name' and 'prefix'\n"); 
	exit(EXIT_FAILURE); 
}else if(len > PREFIX_BYTE){ 
	strncpy(hd -> prefix, path, PREFIX_BYTE); 
	//check back
	strncpy(subtext, &path[PREFIX_BYTE], len - PREFIX_BYTE); 
	strncpy(hd -> name, subtext, strlen(subtext)); 
	
}else{
	strncpy(hd -> name, path, len); 	
}
/*mode*/
sprintf(hd -> mode, "%07o", st -> st_mode); 
hd -> mode[7] = '\0'; 

/*uid*/
sprintf(hd -> uid, "%07o", st -> st_uid); 
hd -> uid[7] = '\0'; 

/*gid*/
sprintf(hd -> gid, "%07o", st -> st_gid); 
hd -> gid[7] = '\0'; 

/*size and typeflag */
if (S_ISREG(st -> st_mode)){
	hd -> typeflag = '\0';
   	sprintf(hd -> size, "%011d", convert_decimal(st->st_size)); 
}else if (S_ISLNK(st -> st_mode)){
	hd -> typeflag = '2'; 
	memset(hd -> size, '0', 11); 
	/*linkname */
	readlink(path, hd -> linkname, LINK_BYTE);   
}else if(S_ISDIR(st -> st_mode)){
	hd -> typeflag = '5';
	memset(hd -> size, '0', 11); 
}else{
	perror("unidentified file type"); 
	exit(1); 
}
hd -> size[11] = '\0'; 

/* mtime */
sprintf(hd -> mtime, "%011ld", st -> st_mtime);
hd -> mtime[11] = '\0'; 

/*magic number*/
strncpy(hd -> magic, "ustar", 5);
/*if Strict is applied, nul-terminated */ 
if(S_flag) {
	hd ->magic[5] = '\0'; 
} 

/*version */ 
strncpy (hd -> version, "00", 2); 

/*uname*/
u_pd = getpwuid(st -> st_uid); 
strcpy (hd -> uname, u_pd -> pw_name); 
 
/*gname */ 
g_gr = getgrgid (st -> st_gid); 
strcpy( hd -> gname, g_gr -> gr_name);  

/*Check back*/ 
/*chksum, itself is treated as if it were filled with spaces */ 
chksum += 8 * ' ';
chksum += check_sum(hd -> name, HUNDRED); 
chksum += check_sum(hd -> mode, EIGHT); 
chksum += check_sum(hd -> uid, EIGHT); 
chksum += check_sum(hd -> gid, EIGHT); 
chksum += check_sum(hd -> size, TWELVE); 
chksum += check_sum(hd -> mtime, TWELVE); 
chksum += hd -> typeflag; 
chksum += check_sum(hd -> linkname, HUNDRED); 
chksum += check_sum(hd -> magic, 6); 
chksum += check_sum(hd -> version, 2); 
chksum += check_sum(hd -> uname, TW); 
chksum += check_sum(hd -> gname, TW); 
chksum += check_sum(hd -> devmajor, EIGHT); 
chksum += check_sum(hd -> devminor, EIGHT); 
chksum += check_sum(hd -> prefix, PREFIX_BYTE); 
/*curr = (unsigned char*) &hd;
for(i = 0; i < BLOCK; i++ ){
	chksum += *curr++; 
}*/ 
sprintf(hd -> chksum, "%07o", chksum); 
hd -> chksum[7] = '\0'; 
/*no handling of devmajor and devminor*/
return hd; 
}

/*write the body content of a regular file */
void write_body(const char *path, FILE *out ){
   char block [BLOCK] = {0}; 
   FILE *in; 
      /* read it in binary mode*/
      if ((in = fopen(path, "rb")) == NULL){
         perror("Failed  to open file");
	 return;  
      }	 
      while ( !feof(in) && 
	     fread(block, BLOCK, 1, in)) {
        fwrite(block, BLOCK, 1, out);
	/*clear buffer*/ 
	memset(block, 0, BLOCK); 
      }
      fclose(in); 	
}

void archive_dir(char *path, FILE *out, int v_flag, int S_flag){
	struct stat st; 
	DIR *dir;
	struct dirent *dt;
	char *file_path;
	header *hd; 
	if ( (dir = opendir(path)) == NULL){
		perror("failed to open dirctory");	
		return; 
	} 
	/* directory name must end in '/'*/
	strcat(path, "/"); 
	while( (dt = readdir(dir)) != NULL ) {
		/*continue when parent directory and itself*/ 
		if (!strncmp(dt -> d_name, ".", 1) || 
		    !strncmp(dt -> d_name, "..", 2)){
		   continue; 
		}
		/*Get the stat for the current file */ 
		if( (stat(dt->d_name, &st)) == -1){
			perror("failed to get status"); 
			continue; 
		}
		/*concatenate file to get full path name */	
		file_path = strcat (path, dt -> d_name); 
		if(v_flag){
		   printf("%s\n", file_path); 
		}
		hd = get_header(file_path, &st, S_flag);
		/*process path based on file type*/ 
		process_path(out, st, hd, file_path, v_flag, S_flag);
		free(hd);    
		
	}
	closedir(dir); 
	
}
void process_path(FILE *out, struct stat st, header *hd, 
		char *path, int v_flag, int S_flag){
	/*Regular file: write header and body*/
	if (S_ISREG(st.st_mode)){
		fwrite(hd, 1, sizeof(header), out); 
		write_body(path, out); 	
	}else if(S_ISLNK(st.st_mode)){
	/*SymLink file: write header*/
		fwrite(hd, 1, sizeof(header), out);  
	}else if(S_ISDIR(st.st_mode)){
	/*Directory: write header, and all its descendents */ 
		fwrite(hd, 1, sizeof (header), out); 
		archive_dir(path, out, v_flag, S_flag); 	
	}else{
		perror("Unidentified File type"); 
		exit(1); 
	} 	
	
}
void create_archive(int argc, char *argv[], int v_flag, int S_flag){
	FILE *out; 
	struct stat st; 
	int i; 
	char empty_set [512] = {0}; 
	/*open the write_to file*/
	out = fopen(argv[2], "wb");
	/*process argv starting from 3*/ 
	for (i = 3; i < argc; i++ ){
		if (lstat(argv[i], &st) == -1){
			perror("mytar"); 	
			//exit(EXIT_FAILURE); 
		}
		if(v_flag){
			printf("%s\n", argv[i]);
		}
		header *hd = get_header(argv[i], &st, S_flag);
		/*process path based on file type*/
		process_path(out, st, hd, argv[i], v_flag, S_flag);     
	}	
	/*Write two empty block: end of archive marker*/
	for(i = 0; i < 2; i++) {
		fwrite(empty_set, 1, BLOCK, out); 
	}		
	fclose(out);
}

/*file name: prefix + "/" + name (if there is a prefix)*/
void get_file_name(char name[], header *hd){
	//char *tmp; 
	if(strlen (hd -> prefix) ==0){
	    strncpy(name, hd -> name, strlen(hd ->name));  
	}else{
	    strncpy(name, hd -> prefix, strlen(hd -> prefix));
	    strcat(name, "/"); 
	    strcat(name, hd -> name); 
	}
	
}

/*generate the elements for listing */
listing *getListing(char *name, header *hd){ 
   listing *ls; 
   int mode;
   time_t mtime; 
   mode = convert_octal(hd -> mode);  
   mtime = convert_octal(hd -> mtime); 
   struct tm *time;
   /*allocate space */
   ls = (listing*) malloc (sizeof (listing));
   /*\0 or 0 is regular, 2 is symlink, 5 is directory*/  
   if (hd -> typeflag == '0' || hd -> typeflag == '\0'){
	 ls -> permissions[0] = '-'; 
   }else if (hd -> typeflag == '2'){
	 ls -> permissions[1] = 'l'; 
   }else if (hd -> typeflag == '5'){
	ls -> permissions[2] = 'd'; 
   }else{
 	perror("unidentified file type"); 
   }
   /*Do bit wise and to with permission to extract each permission*/
   /* user*/
   if (mode & S_IRUSR){
	ls -> permissions[1] = 'r'; 
   }else{
	ls -> permissions[1] = '-'; 
   } 
   if (mode & S_IWUSR){
	ls -> permissions[2] = 'w'; 
   }else{
	ls -> permissions[2] = '-'; 
   } 
   if(mode & S_IXUSR){
	ls -> permissions[3] = 'x'; 
   }else {
	ls -> permissions[3] = '-'; 
   } 
   /*group */
   if (mode & S_IRGRP){
	ls -> permissions[4] = 'r'; 
   }else{
	ls -> permissions[4] = '-'; 
   } 
   if (mode& S_IWGRP){ 
	ls -> permissions[5] = 'w'; 
   }else{
	ls -> permissions[5] = '-'; 
   } 
   if (mode & S_IXGRP) {
	ls -> permissions[6] = 'x'; 
   }else{
	ls -> permissions[6] = '-'; 
   } 
   /*other */
   if (mode & S_IROTH){
	ls -> permissions[7] = 'r';
   }else{
	ls -> permissions[7] = '-'; 
   } 
   if (mode & S_IWOTH){
	ls -> permissions[8] = 'w'; 
   }else{
	ls -> permissions[8] = '-'; 
   } 
   if (mode & S_IXOTH){
	ls -> permissions[9] = 'x' ;
   }else{
	ls -> permissions[9] = '-' ;
   } 
   
   /*user and group name. A width of 17 for this variable
       ,but uname and gname might add up to 64 bytes */
   /*if 17 bytes are enough */
   if (strlen(hd -> uname) > OWNER_BYTE){
	strncpy(ls -> owner_group, hd -> uname, OWNER_BYTE); 
   }else{
   /*store up to 17 bytes*/
   	strncpy(ls -> owner_group, hd -> uname, strlen(hd -> uname)); 
	strcat(ls -> owner_group, "/"); 
	strncat(ls -> owner_group, hd -> gname, 
		OWNER_BYTE - strlen(hd -> uname) - 1); 
   }  

   /*size of the file */
   //sprintf( ls -> size, "%07llu", convert_octal(hd -> size));
   ls -> size = convert_octal (hd -> size); 	

   /*mtime*/
   time = localtime(&mtime); 
   sprintf(ls -> mtime, "%4d-%2d-%2d %2d:%2d", 1900 + time -> tm_year, 
		1 + time->tm_mon, time ->tm_mday, time -> tm_hour,
 		time -> tm_min); 
   /*name*/ 
   ls -> name = name;   	
   return ls; 
}

/*converting octal stored in a char array to integer*/ 
int convert_octal(char size[]){
int i, ans, pow, times, tmp; 
int l = strlen (size);
pow = 8; 
for (i =0; i < l; i++ ){
	times = l - i - 1;
	for(tmp = 1; tmp < times; tmp++){
	   pow *= 8; 
	}
	/*check back for pow */
	ans += (int)((size[i] - '0') * pow);
  	pow = 8;
	tmp = 1;  
}	
return ans; 
}

/*check if a path is a prefix of any files in the archive 
 used for listing and extraction */
int check_prefix(char *archive, char *path ){
	return  strncmp(archive, path, strlen(path)); 
} 


void list_archive(int argc, char *argv[], int v_flag) {
   /*if no names given -> list all files in the archive 
     if names given -> list the given path and all the descendents of it*/
	FILE *archive; 
	header *hd = NULL; 
	listing *ls; 
	char zero_block [BLOCK] = {0}; 
	int file_size;
	char content_block[BLOCK] = {0};
	/*file name +2  accounts for slash and nul terminator */
	char file_name [NAME_BYTE + PREFIX_BYTE + 2];   
	int i; 
	if ( (archive = fopen(argv[2], "rb")) == NULL){
		perror("Failed to open tar file");
		exit(1); 
	} 
	if (argc == 3 ){ // no paths given 
	   for (;;){
	      /* read the header block*/
              fread(hd, BLOCK, 1, archive); 
              if(ferror(archive)){
   		perror("Error reading FILE"); 
              }
	      /* get the name of the file*/ 
	      get_file_name(file_name, hd);
	      
	      /*if v flag is off */
	      if(!v_flag){
	      	printf("%s\n", file_name); 
	      } else{
              /*get listing element and print them */
		ls = getListing(file_name, hd);
		printf("%10s %17s %8ld %16s %s", 
			ls -> permissions, ls -> owner_group, 
			ls -> size, ls -> mtime, ls -> name); 
		free(ls); 
	      }
	      if(feof(archive)) {
		break;
	      } 
	      /*skip the body content */
              file_size = convert_octal(hd -> size);
	      /*skip blocks = least multiple of 512 of file size + 1 */ 
              while (file_size - BLOCK > 0) {
		fread(content_block, BLOCK, 1, archive); 
		if( !memcmp(content_block, zero_block, BLOCK)){
		   //reached the end of file mark(zero block)
		   return; 
		}
		file_size -= BLOCK;       
              }
	      /*check end of file or zero block marker */
	      if(feof(archive)||(!memcmp(content_block, zero_block,BLOCK))) {
	      	break; 
	      } 
	   }	
	}else{
        /*If paths are given,
 	  check if it is a prefix of any archived files */ 
	   for (;;){
              fread(hd, BLOCK, 1, archive); 
              if(feof(archive)){
		break;
	      }else if(ferror(archive)){
		perror("Error reading FILE"); 
	      } 
	      get_file_name(file_name, hd); 
	      for(i = 3; i < argc; i++ ){	
	         /*check if this path is a prefix */
	         if(!check_prefix(file_name, argv[i])){
		 /*Do the same as above except check prefix*/
	    	    if(!v_flag){
		      printf("%s\n", file_name);   
		    }else{
		      ls = getListing(file_name, hd); 
		      printf("%10s %17s %8ld %16s %s",
			ls -> permissions, ls -> owner_group, 
			ls -> size, ls -> mtime, ls -> name); 
		      free(ls); 
		    }
		 }	 
	      }	
	      /* skips the body content of a file*/
	      file_size = convert_octal(hd -> size); 
	      while(file_size - BLOCK > 0){
	 	fread(content_block, BLOCK, 1, archive); 
		if ( !memcmp(content_block, zero_block, BLOCK)){
		  return; 
		}
		file_size -= BLOCK; 
	      }
              /*check if end of file or zero block */
	      if(feof(archive) ||
		 (!(memcmp(content_block, zero_block, BLOCK)))){
		break; 
	      }
	    }
 	}
}

void extract(FILE *archive, char *file_name, header *hd){
	FILE *out; 
	char type;
	char block [BLOCK] = {0};
	struct utimbuf new_time;
	struct stat st; 
	DIR *dir; 
	struct dirent *dt;
	header *new;  
	/*get the type of file we're dealing with*/   
	type = hd -> typeflag;
	/*open the file of extraction*/
	out = fopen (file_name, "wb");
	/*If it is a regular file */ 
	if (type == '0' || type == '\0') {
		/*read the body content of the file*/
		while(fread(block, BLOCK, 1, archive)){
		  if (ferror(archive)){
		  	perror("Error reading file"); 	
		  }else{
		  /* extract and empty the buffer*/
		  	fwrite(block, BLOCK, 1, out);
			memset(block, 0, BLOCK); 
			if( feof(archive)){	
			   /*finish extracting, break */
			   break; 
			}  
	          }
		}
		/*set the permission */
		chmod(hd-> name, convert_octal(hd->mode)); 
		/*get previous owner and group id */
		chown(hd -> name, convert_octal(hd -> uid),
				convert_octal(hd ->gid));
		/*restores the modification time*/ 
		new_time.modtime = time(NULL); 
		utime(hd -> name, &new_time);
	/*If it is a symbolic link */  
	}else if (type == '2'){
		/*extract the linkname, and link it to the name*/
		symlink(hd -> linkname, hd -> name); 
		/*set permission*/
		chmod(hd -> name, convert_octal(hd -> mode));
		/*set owner/group id */
		chown(hd -> name, convert_octal(hd -> uid),
				convert_octal(hd -> gid));
		/*restores the modification time */  
		new_time.modtime = time(NULL); 
		utime(hd -> name, &new_time);  
	/*If it is a directory */	
	}else if (type == '5'){
		/* create a directory*/ 	
		mkdir(hd -> name, convert_octal(hd -> mode));
		/*change ownership*/ 
		chown(hd -> name, convert_octal(hd -> uid),
				convert_octal(hd -> gid));
		/*restore modification time */ 
		new_time.modtime = time(NULL); 
		utime(hd -> name, &new_time);
		/*deal with all the descendents of the archive */
		if((dir = opendir(file_name))== NULL){
		   perror("archive"); 
		} 
		while ((dt = readdir(dir))!=NULL){
		   /*skip "." and ".." */
		   if(!strncmp(dt -> d_name, ".", 1) ||
			!strncmp(dt -> d_name, "..", 2)){
			continue; 
		   } 
		   if(stat(dt -> d_name, &st) == -1){
		      perror("failed to get stat");
		      return;  
		   }
		   strcat(file_name, dt->d_name); 
	   	   new = get_header(file_name, &st, 1);
		   /*Do a recursive call on extract*/ 
		   extract(archive, file_name, new);  
		}
	}else{
		perror("unidenified type for extraction"); 
		return; 
	}	
}

void extract_archive(int argc, char *argv[], int v_flag) {
	FILE *archive;
	int i;   
	header *hd =NULL; 
	/*file name + 2 accounts for / and null terminator */
	char file_name[NAME_BYTE + PREFIX_BYTE + 2];
	char zero_block[BLOCK] = {0};  
	archive = fopen(argv[2], "rb"); 
	for (;;) {
		/*read one header from the archive*/
		fread(hd, BLOCK, 1, archive); 
		/*If there are error reading */
		if(ferror(archive)){
		   perror("Error reading FILE"); 
		/*Check if reached the End of File mark*/
		}else if(!(memcmp(hd, zero_block, BLOCK))){
		   return; 
		}
		/*regenerate file name by prefix and name field */
		get_file_name(file_name, hd);
		/*For no paths are given*/
		if (argc == 3){ 
			if (v_flag){
			   printf("%s\n", file_name); 
			}
			extract(archive, file_name, hd); 
			
		}else{ 
		 /* If there are paths given*/
		    for (i = 3; i < argc; i++){
			/* check if it's a prefix of any archived files*/
			if(!check_prefix(file_name, argv[i])){
			    if(v_flag) {
				printf("%s\n", file_name); 
			    } 
			    extract(archive, file_name, hd); 
			} 
		    }
		}
	     if(feof(archive)){
		break; 
	     }		
	}
}

int main (int argc, char *argv[] ){
	char inst [INS_MAX];
	char *instruction;
	int i;  
	/*if argument is less than 2, perror usage */ 
	if ( argc < 2 ){
		perror("usage: mytar [ctxvS]f tarfile [path[...]]"); 
		exit(1); 	
	}
	/*store ctxvSf instruction in instruction */
	instruction = argv [1];
	i = 0; 
	/*copy it over to char array with a max length of 6*/
	while (*instruction){
		inst[i++] = *instruction++;  
	} 	
	inst[i] = '\0'; 
	/* Only one of ctx, so the length of insturction is max at 4*/
	if (strlen(instruction) > 4 ){
		perror("usage: only one of 'ctx' instruction is allowed"); 
		exit(1); 
	}
	
	/*Sort the char array, so we can parse instruction */
	sort_instruction(inst); 
	if (!strcmp(inst, "f")){
	/* For all possible c instruction*/	
	}else if (!strcmp(inst, "cf")){
		create_archive(argc, argv, 0, 0); 	
	}else if (!strcmp(inst, "cfv")){
		create_archive(argc, argv, 1, 0); 
	}else if (!strcmp(inst, "Scf")){
		create_archive(argc, argv, 0, 1); 
	}else if (!strcmp(inst, "Scfv")){
		create_archive(argc, argv, 1, 1);
	/*For all possible t instruction */ 
	}else if (!strcmp(inst, "ft")){
		list_archive(argc, argv, 0); 		
	}else if (!strcmp(inst, "ftv")){
	 	list_archive(argc, argv, 1); 	
	}else if (!strcmp(inst, "Sft")){
		 //Assumption: S flag has no impact on listing 
		list_archive(argc, argv, 0); 
	}else if (!strcmp(inst, "Sftv")){
		list_archive(argc, argv, 1); 
	/* for all possible x instruction*/
	}else if (!strcmp(inst, "fx")){
		extract_archive(argc, argv, 0);  
	}else if (!strcmp(inst, "fvx")){
		extract_archive(argc, argv, 1);	
	}else if (!strcmp(inst, "Sfx")){
		extract_archive(argc, argv, 0); 
	}else if (!strcmp(inst, "Sfvx")){
		extract_archive(argc, argv, 1);
	/*Instruction mismatched*/
	}else{
		perror("invalid [ctxvS]f instruction"); 
		exit(1); 
	} 
			
return 0; 
} 
