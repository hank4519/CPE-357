#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
char *read_long_line( FILE *file); 

int main(int argc, char * argv[] ){
FILE *fp;
if(argv[2] != NULL){
	fp = fopen(argv[2], "r");
}
else{
	fp = fopen (*++argv, "r"); 
} 
if(fp == NULL){
	printf("Cannot find file. Try use \"Testfile\" '\n'" ); 
	exit(0); 
}
char *curr = read_long_line(fp);
char *prev = NULL; 

//while(*curr != EOF) {
while(!feof(fp)){
	prev = curr; 
	curr = read_long_line(fp);
	int cmp = strcmp(prev, curr);
	if(cmp == 0 ){	 
		continue; 
	}	
	//char *tmp = prev; 
	//for( ; *tmp != '\0' ;tmp++) {
	//	putchar(*tmp); 
	//}
	//putchar('\n');
	printf("%s", prev); 
}  
free(prev); 
fclose(fp); 
return 0; 
}

char *read_long_line(FILE *file){
	const int val = 50 ;
	int count = 0; 
	int size = 50;  
	char *A;
	A = (char*)malloc(val*sizeof(char));
//	while (( *(A+count) = getc(file)) != '\n' ){
	while((A[count] = getc(file)) != '\n'){ 
		//if(*(A+count) == EOF){
	if(A[count] == EOF){
		break; 
	
	}
	if(count>= size -1){
		if(( A = (char*) realloc (A, (size+val)*sizeof(char))) == 0) {
			perror("realloc() failed"); 
			exit(EXIT_FAILURE); 
		} 
		else{
			size+= 50;  
		} 
	} 
	count++; 

	}
//	*(A+count)='\0'; 
//	A[count] = '\0';  
	return A; 
} 
