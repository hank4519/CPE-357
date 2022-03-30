
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <ctype.h> 
#define table_size 32159
typedef struct node{
	char *key;
	int count; 	
	struct node *next; 
} node;



typedef struct{ 
	node ** entry;  
}table; 


void print_table (table *hashtable){
	int i; 
	for( i =0; i < table_size; i++ ){
		node *n = hashtable -> entry[i]; 
		if (n == NULL){
		continue; 
		} 
	printf("slot [%4d]: ", i);
	for(;;){
		printf("%s=%d", n ->key, n -> count); 
		if (n -> next == NULL) {
		break; 
		}
		n = n -> next ;  
	}
	printf("\n"); 
	}
}


table* create (void) {
	table *hashtable = malloc (sizeof(table)); 
	hashtable -> entry = malloc (sizeof(node*) * table_size); 
	int i; 
	for(i =0; i< table_size; i++){
		hashtable -> entry[i] = NULL; 
	} 
	return hashtable; 
}

 	
unsigned long hashcode(char *str){
	unsigned long hash = 7; 
	int i; 
	for(i =0; i < strlen(str); i++){
		hash = 31 * hash * str[i];  
	} 
	return hash % table_size  ; 
}

int insert(table *hashtable, char *str){
	unsigned int pos = hashcode(str); 
	node * entry  = hashtable -> entry[pos]; 
	if (entry == NULL){
		node *n = malloc (sizeof(n)); 
		n -> key =(char*) malloc (strlen(str) + 1);
		strcpy(n->key, str) ;
		n -> count = 1;
		n -> next = NULL;   
		hashtable -> entry[pos] = n;
		return 1; 
	}
	node *prev; 
	while (entry != NULL) {
		if((strcmp(entry -> key, str)) == 0 ){
			entry -> count = entry -> count + 1; 
			return 0; 	
		}
		prev = entry; 
		entry = prev -> next; 
	} 
	node *end = malloc (sizeof(end)); 
	end -> key = (char*) malloc (strlen(str) + 1); 
	strcpy (end -> key, str); 
	end -> count = 1; 
	end -> next = NULL; 
	prev -> next = end; 
	return 1; 	 	
}

char *readWord(FILE *file){
	int count = 0; 
	int size = 10;
	char *A;
	char c; 
	A = (char*)malloc(size*sizeof(char));
	if(!A) {
		perror("Failed malloc"); 
		exit(EXIT_FAILURE); 
	} 
	c = getc(file);

	while (isalpha(c)){
	c = tolower(c);  
	A[count] = c; 	
	if(count>= size - 2){
		if(( A = (char*) realloc (A, (size+10)*sizeof(char))) == 0) {
			perror("realloc() failed"); 
			exit(EXIT_FAILURE); 
		} 
		else{
			size+= 10;  
		} 
	} 
	count++; 
	c = getc(file); 
	}
	A[count] = '\0';
	return A;  
}

node* find_most (table *hashtable){	
	int i;
	int max =0;
	node *curr;   
	for( i =0; i < table_size; i++ ){
		node *node = hashtable -> entry[i]; 
		if (node == NULL){
		continue; 
		} 
				
	for(;;){
		if(node -> count > max ){
			max = node -> count;
			curr = node;  
		}
		else if (node -> count == max ) {
			if (strcmp (curr->key, node->key) < 0){
				curr = node; 
			}
		}
		if (node -> next == NULL) {
			break; 
		}
		node = node -> next ;  
	}
	}
	return curr;  	
}
int main(int argc, char **argv){
FILE *fp;
int k, f_index, total; 
char *str;
total = 0;

table *hashtable = create();  
if(argc < 2) {
	fp = stdin; 
	k = 10;
	if(fp == NULL ){
		perror("usage: fw [-n num] [ file1 [ file 2 ...] ]");
		return 1; 
	}
	if(feof(fp)){
		printf("The top %d words (out of %d) are:\n", k, total);
		return 0;  
	}	
	while (!feof(fp)){
		str = readWord(fp);
		if(isalpha(str[0])){
			if(insert(hashtable, str)){
			total++;  }
	}
	free(str);} 
}
else if(!strcmp (argv[1],"-n") && ( argc >= 3) ){
	k = atoi (argv[2]);
	if(k == 0) {
           fprintf(stderr, "usage:: fw [-n num] [ file1 [ file 2 ...] ]\n");
		return 1; 
	} 
	for (f_index = 3; f_index < argc; f_index++){
		fp = fopen(argv[f_index], "r"); 	
		if(fp == NULL ){
			perror("usage: fw [-n num] [ file1 [ file 2 ...] ]");
			continue;  
		}
		while (!feof(fp)){
			str = readWord(fp);
			if(isalpha(str[0])){
				if(insert(hashtable, str)){
				total++;  }
			}
		free(str);
		}
		fclose(fp);  
	}
	

}else{
	k = 10;
	for(f_index = 1; f_index < argc; f_index++){
		fp = fopen(argv[f_index], "r"); 
		if(fp == NULL ){
			perror("usage: fw [-n num] [ file1 [ file 2 ...] ]");
			continue;
		}
		while (!feof(fp)){
			str = readWord(fp);
			if(isalpha(str[0])){
				if(insert(hashtable, str)){
				total++;}  
			}
		free(str);
		}
		fclose(fp); 
	}
}
	printf("The top %d words (out of %d) are:\n", k, total);  
 	while ( k > 0){
		node *most = find_most(hashtable);
		if(most != NULL) {
			if(!(most -> count == 0 )){
			printf("%9d %s\n", most -> count, most -> key);
				most -> count = 0;
			} 
		}
		k--;  
	} 	

return 0;
}
