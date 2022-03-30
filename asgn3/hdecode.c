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

/*Initialize array*/
void iniArr(int arr []){
	int i;
	for(i =0; i < 256; i++ ){
		arr[i] = 0; 	
	}
}
int main(int argc, char *argv[]){
	int in, out, sz, num, i, shift,  count, total;
	int arr[256];
	node *head, *tmp;   
	uint8_t one, mask, and; 
	uint32_t four;
	head = NULL;
	/* Check file format: */   
	if(argc > 3){
		printf("Format: hdecode[(infile|-) [outfile]]"); 
	}
	if (argc <= 1){ // no infile/outfile
		in = 0; 
		out = 1;
	}	
	else{ //Checking optional file argument 
		if(!strcmp(argv[1], "-")){
			in = 0;
		}else{
			in = open(argv[1], O_RDONLY);
			if(in == -1){
				perror(argv[1]); 
				exit(EXIT_FAILURE); 
			}	
		} 	
		if (argc == 3){
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
	sz = read(in, &one, 1);
	/*If no byte is read*/
	if(sz==0){return 0;}
	num = one;
	iniArr(arr);
	/*Only contains one char in the file*/ 
	if(num == 0) { 
		read(in, &one,1); 
		read(in, &four, 4); 
		four = ntohl(four);
		for(i = 0; i < four; i++){
			write(out, &one, 1); 
		}
	}
	total = 0;
	/*Get frequency for each character to rebuild the tree*/
	for(i =0; i <= num ;i++ ){
		read(in, &one, 1); 
		read(in, &four, 4);
		four = ntohl(four);  
		arr[one] = four;
		total += four; 
	}
	//Create Tree
	head = create_list(arr, head);  
	tmp = head;
	count = 0; 
	/*Decode the body part*/
	while(read(in, &one, 1) != 0){
		shift = 7; 
		while (shift != -1 ){
			and = 0; 
			mask = 1 << shift; 
			and = mask & one; 
			and = and >> shift; 
			if(and == 1){
				if(tmp -> right != NULL)
				  tmp = tmp -> right;
			} else{
				if(tmp -> left != NULL)
				  tmp = tmp -> left;
			} 
			if(isleaf (tmp)){
				count++; 
				if(count <= total){
				  write(out, &(tmp -> c), 1);
				} 	
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
