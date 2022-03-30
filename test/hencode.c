#include<fcntl.h>
#include<stdio.h>
#include<string.h>
#include <stdint.h>
#define SIZE 4096
int main(int argc ,char *argv[]){
	char buff [4096]; 
	//int out;
	//out = 1;
 	//write(out, "abc", 3);
 	int in, sz;
	uint8_t one;  
	in = open(argv[1], O_RDONLY) ; 
	/*while (read(in, &one, 1) != 0){
		printf("%d  % \n", one); 
	}*/
 	sz= read (in, &buff, SIZE);
	printf("%d", sz);	
	close (in); 
	return 0; 

}
