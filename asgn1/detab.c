#include <stdio.h> 
int main(){

	int c;
	int count=1; 
	int spaces; 
	while((c=getchar()) != EOF){
	if(c=='\t'){
		spaces = 8 - ((count-1)%8); 
		while(spaces>0){
		putchar(' '); 
		spaces--; 
		count++; 
	}
	}
	else if(c== '\n'){
	putchar(c); 
	count=1; 
	}
	else if(c== '\b'){
	if(count>1){
	count--; }
	putchar(c);   
	}
	else if(c=='\r'){
	putchar(c); 
	count =1; 
	}
	else{
	putchar(c); 
	count++; 
	}
	}
	
	return 0; 
}
