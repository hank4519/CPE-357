
#include <unistd.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h> 
#include <arpa/inet.h>
#include <sys/stat.h> 
#include "htable.h"  
#define SIZE 8193 
void write_body(int out, char *buff, char *res[]){
	uint8_t one; 
	int shift, i, index, co;
	char *code; 
	shift = 7;
	one = 0;
	/* Process each character*/
	for(i = 0; i < strlen(buff); i++ ){
		code = res [((int)buff[i])]; // Get Huff code  
		for(index = 0; index < strlen(code); index++){
			if(code[index] == 48){co = 0;}
			else if(code[index] == 49){co = 1;} 
			one = one | (co << shift);
			shift--; 
			if(shift== -1){	 //conclude one byte
				write (out, &one, 1); 
				shift = 7;
				one = 0;
			}
		}
	}
	/*Take care of trailing byte*/
	if(shift != 7){	
		write(out, &one, 1); 
	}
}


int main (int argc, char *argv[]){


int in, out, sz, i;
char buff[SIZE]; 
int arr[256];
node *head = NULL; 
char str[8]; 
char *res[256];  
int num;
uint8_t one; 
uint32_t four;  
if (argc < 2 || argc > 3) { //Check file format 
	printf("Format: hencode infile [outfile]"); 
	exit(1); 
}
if((in = open(argv[1], O_RDONLY)) == -1){
	perror(argv[1]); 
	exit(EXIT_FAILURE); 
} 
if(argc == 3){
	out = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IRGRP); 
	if(out == -1){
		perror(argv[2]); 
		exit(EXIT_FAILURE); 
	}
}else{
	out = 1; 
}
sz = read(in, &buff, SIZE);
if(sz ==0){ //If it is an empty file 
	close(in); 
	close(out); 
	return 0;
}
buff[sz] = '\0';
//printf("Actual bytes read:  %d", sz); 
fill_array(arr, buff, &num);
head = create_list(arr, head); //build tree 
ini_res(res); 
printPreorder(head, str, 0, res); //record huff code
one = num -1; 
write(out, &one, 1); 
/* Encoding the header part*/
for(i = 0; i < 256; i++){
	if(arr[i] != 0){
		one = i;
		write(out, &one, 1); 
		four = htonl(arr[i]); 
		write(out, &four, 4); 
	}
}
write_body(out, buff, res); //encode the body
freeTree(head); 
freeRes(res); 
close(in); 
close(out); 
return 0; 
}








