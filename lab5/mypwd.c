#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <limits.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>  

/*Global variable flag check the statring directory*/
int flag = 1;
 
void traverse(struct stat p, char path[]){
int len1, len2; 
DIR *folder; 
struct dirent *entry; 
struct stat st;
/* Get the stat of the root directory*/ 
if (lstat("/", &st) == -1){
	perror("mypwd"); 
	exit(EXIT_FAILURE); 
}
/* If the previous stat's inode/device number match root's stat, done*/
if (p.st_ino == st.st_ino && p.st_dev  == st.st_dev){
	return;  
} 
/*Starting directory, need to move one directory up */
if(flag){
	if(lstat(".", &st) == -1 || chdir("..") == -1) {
		perror("mypwd"); 
		exit(EXIT_FAILURE); 
	} 
	p = st;
	flag = 0;  
} 
/*Open the current diretory*/
if((folder = opendir(".")) == NULL){
	perror("mypwd"); 
	exit(EXIT_FAILURE); 
} 
/*Go through each sub-directory */
while((entry = readdir(folder)) != NULL){
	lstat(entry -> d_name, &st);
	/*check both i_node number and device number*/ 
	if(p.st_ino == st.st_ino && p.st_dev == st.st_dev){
		if (lstat(".", &st) == -1 || chdir("..") == -1){
			perror ("mypwd");
			exit(EXIT_FAILURE); 	
		}
		/*st stores the stat info about the previous one 
		chdir goes one dir up, the recursive call */
		traverse(st, path);
		len1 = strlen(path); 
		len2 = strlen(entry -> d_name);
		/*check if path is too long*/ 
		if (len1 + len2 + 1 > PATH_MAX){
			perror("path too long"); 
			exit(1); 
		}		 
		strcat(path, entry -> d_name);
		strcat(path, "/");
		closedir(folder); 
		return; 
	}
}
}



int main(int argc, char *argv[] ){

char path [PATH_MAX]; 
struct stat p, st;
if(lstat(".",&p) == -1 || lstat("/", &st) == -1){
	perror("mypwd");
	exit(EXIT_FAILURE);
}
/*check if we are at the root*/
if (p.st_ino == st.st_ino && p.st_dev == st.st_dev){
	printf("/\n");
	return 0;  
}   
/*p stores the stat info about the previous directory*/
traverse(p, path); 
path[strlen(path) - 1] = '\0'; 
printf("/%s\n", path);
 
return 0; 
}
