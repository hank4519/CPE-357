
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "htable.h"   
node *leaf (char, int);
  
node* insertnew(node*, char, int);  
void fill_array (int arr [], char buff [], int *num ){
	int c, i; 
	for(i =0; i < 256; i++ ){
		arr[i] = 0; 
	}  
	while ( (c = *buff++) != '\0')  {
		if( 0 <= c && c <256) {	 
			arr[c]++;
			if(arr[c] == 1)
				(*num)++; 
		}
	}
}

node* create_list(int arr [], node *head){
   int i;
   for(i =0 ; i <256; i++ ){
	if (arr[i] != 0) {
		if(head == NULL) {
			char t = (char)i ; 
			head = leaf(t, arr[i]);  
		}
		else{
			char t = (char)i ; 
			head = insertnew(head, t, arr[i]); 
		}
	}
   }
    	 
	while (head != NULL && head -> next != NULL) {
		node *first = head; 
		node *second = head -> next; 
		head = head -> next -> next;
		node *merge; 
		merge = inter_node(first, second);  
		if (head == NULL ){
			head = merge; 
			break;
		} 
		head = insert(head,merge); 				
	}
     return head; 
		
}

node* leaf (char ch, int frequency) {
	node *new = (node*)malloc (sizeof(node)); 
	new -> c = ch; 
	new -> count = frequency; 
	new -> left = NULL; 
	new -> right = NULL; 
	new -> next = NULL; 
	return new; 
}
node* inter_node (node *f, node*s){
	if (f == NULL || s == NULL) return NULL; 
	f -> next = NULL; 
	s -> next = NULL; 
	node *inter = (node*) malloc (sizeof(node));
	inter -> left = f; 
	inter -> right = s; 
	inter -> count = f -> count + s -> count ; 
	return inter;  
}
int isleaf (node *n){
	return n -> left == NULL && n -> right == NULL; 
}
node* tie_c (node *head, node *n){

	if (n -> c < head -> c) {
		n -> next = head; 
		return n; 
	}
	 node *tmp = head;
while ((tmp->next->count==n->count)&&(tmp->next->c)<(n->c)){
	 	tmp = tmp -> next;
		if(tmp -> next == NULL) break; 
	  }
	  n -> next = tmp -> next; 
	  tmp -> next = n; 
	  return head; 
}
node* insertnew(node* head, char ch, int frequency){
	node *new = leaf (ch, frequency); 
	if(new -> count < head -> count ){
		new -> next = head; 
		return new; 
	}	
	node *tmp = head; 
	while ( tmp -> next != NULL && (new -> count >= tmp -> next -> count) ){
		tmp = tmp -> next;
	} 
	new -> next = tmp -> next; 
	tmp -> next = new; 
	return head; 
	
}
void print_table(node* head){
	node *curr =head;
	while ( curr != NULL ){
		printf("%c -> %d\n", curr->c, curr ->count);
		curr = curr->next;
	}
}
void reverse(char *str){
	int i, len, tmp; 
	len = strlen(str);
	for(i=0; i < len /2 ; i++){
		tmp = str[i];
		str[i] = str[len - i -1]; 
		str[len -i - 1] = tmp; 
	}	
	for(i = 0; i < len; i++ ){
		if(isalpha (str[i])){
			str[i] = tolower(str[i]); 
		
		}
	}	
}

node* insert (node *head, node*merge) {
	if (head == NULL || merge == NULL) return NULL;
	node *curr = head; 
	if(merge -> count <= head -> count ){
		merge -> next = head; 
		return merge; 
	}
	while(curr-> next!= NULL && (curr -> next -> count < merge -> count )){
		curr = curr -> next; 
	}
	merge -> next = curr -> next;
	curr -> next = merge; 
	return head;  
}
char* pad_zero(char* tmp ){
	char* new = (char*)malloc(15*sizeof(char)); 
	int len, zero, i,j, s;
	i= 0; 
	s = 0;  
	new[i++] = ' '; 
	len = strlen(tmp); 
	zero = 8 - len; 
	while(i <= zero){
		new [i++] = '0'; 
		s++; 
		if(s == 2){
			new[i++]= ' '; 
			s = 0; 
			zero++; 
		}	
	}
	j =0; 
	while(len > 0){
		new[i++] = tmp[j++]; 
		s++; 
		if(s == 2){
		new[i++] = ' '; 
		s = 0; 			
		}
		len--;
	}	 		
	return new; 		
}
char* freq_str (char* num_in_hex, int num){
	char *res = (char*)malloc(15 * sizeof(char)); 
	char *tmp; 
	res = strcat(res, num_in_hex); 
	tmp = getHex(num); 
	reverse(tmp); 
 	tmp = pad_zero(tmp);
	res = strcat (res, tmp); 
	return res;  
	
}	
char *getHex (int num){
	int tmp, i;
	i = 0; 
	char *str = (char*) malloc (2*sizeof(char)); 
	while(num != 0){
		tmp = num % 16; 
		if( tmp < 10){
			tmp += 48;
		}
		else{
			tmp += 55; 
		}
		str [i++] = tmp; 
		num = num / 16; 
		if(i > 2) break ; 
	}
	if(strlen(str) < 2){
		str[1] = '0';  
	}
	return str; 
}
void printPreorder(node *head, char *str, int level, char *res[]){
	if(isleaf(head)){
		int len; 
		int c = head -> c; 
		str[level] = '\0';  
		len = strlen(str); 
		res[c] = (char*) malloc (len + 1);
		strcpy(res[c], str);
		return; 
	}
	if(head -> left != NULL ){
		str[level] = '0'; 
		printPreorder(head -> left, str , level + 1, res);
		str[level] = '\0'; 
	}
	if(head -> right != NULL){
		str[level] = '1';  
		printPreorder(head -> right, str , level +1, res ); 
		str[level] = '\0';  	
	}
}
void printRes(char *res[]){
	int i;
	for(i =0; i <256; i++ ){
		if (res [i] != NULL){
			char *hex = getHex(i);
			reverse(hex);
			printf("0x%s: %s\n", hex, res[i]);
			free(hex); 
		}
	}
}
void freeTree(node *head){
	if(head){
		freeTree(head -> left); 
		freeTree(head -> right);  
		free(head);
	}
}
void freeRes(char *res []) {
	int i;
	for(i = 0; i < 256 ; i++ ){
		free(res [i]); 
	}
} 
void print_arr (int  arr []){
	int i; 
	for (i =0; i < 256; i++) {
		printf("%d - - %d\n", i, arr[i]); 
	}	
}
void ini_res (char *res[]){
	int i; 
	for(i = 0; i < 256; i++ ){
		res[i] = NULL;
	}
}


/*
int main(int argc, char **argv){
	int arr [256];
	char *res [256]; 
	node *head = NULL;
	node *merge = NULL;
	int i;
	char str[8]; 
	FILE *fp;
	if(argc > 1) {
		fp = fopen (argv[1], "r"); 
	}else{
		fp = stdin; 
	} 
	if( fp == NULL) {
		fprintf(stderr, "Failed to open file"); 
		return 1; 
	}  
	for(i =	0; i < 256; i++ ){
		arr [i] = 0; 
	}
	fill_array(arr, fp);
	head = create_list(arr, head);
	while (head != NULL && head -> next != NULL) {
		node *first = head; 
		node *second = head -> next; 
		head = head -> next -> next;
		merge = inter_node(first, second);  
		if (head == NULL ){
			head = merge; 
			break;
		} 
		head = insert(head,merge); 				
	}
	ini_res(res); 
	printPreorder(head, str, 0, res);
	printRes(res);
	freeTree(head); 
	freeRes(res);   
	fclose(fp); 
return 0; 
}*/
