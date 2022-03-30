#include <unistd.h> 
#include <fcntl.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h> 
#include <arpa/inet.h>
#include <sys/stat.h> 
#include "htable.h"  
#define SIZE 4096
void iniArr(int arr []){
	int i;
	for(i =0; i < 256; i++ ){
		arr[i] = 0; 	
	}
}
int main(int argc, char *argv[]){
	int in, out, num, i, shift;
	int arr[256];
	node *head, *tmp;   
	char buff[SIZE];
	uint8_t one, mask, and; 
	uint32_t four;
	head = NULL;   
	if(argc > 3){
		printf("Format: hdecode[(infile|-) [outfile]]"); 
	}
	if (argc <= 1){ // no infile/outfile
		in = 0; 
		out = 1;
	}	
	else{
		if(!strcmp(argv[1], "-")){
			in = 0;
		}else{
			in = open(argv[1], O_RDONLY);
			if(in == -1){
				perror(argv[1]); 
				exit(EXIT_FAILURE); 
			}	
		} 	
		if (argc == 2){
			out = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 
				S_IRUSR|S_IRGRP); 
			if(out == -1){
				perror(argv[2]); 
				exit(EXIT_FAILURE); 
			}
		}else{
			out = 1; 
		}
	}
	if(read(in, &buff, SIZE) == 0){
		perror("Failed to read"); 
		exit(EXIT_FAILURE);
	} 
	read(in, &one, 1); 
	num = one;
	iniArr(arr);  
	for(i =0; i <= num ;i++ ){
		read(in, &one, 1); 
		read(in, &four, 4);
		four = ntohl(four);  
		arr[one] = four; 	
	} 
	head = create_list(arr, head);  
	tmp = head;
	while(read(in, &one, 1) != 0){
		shift = 7; 
		while (shift != -1 ){
			and = 0; 
			mask = 1 << shift; 
			//one = ((mask & one) >>> shift); 
			and = mask & one; 
			and = and >> shift; 
			if(and == 1){ 
				tmp = tmp -> right;
			} else{
				tmp = tmp -> left;
			} 
			if(isleaf (tmp)){
				write(out, &(tmp -> c), 1);
				tmp = head;  	
			}
			shift--; 
		}			
	}
	
	freeTree(head); 
	close(in); 
	close(out); 
	
return 0; 
} 
