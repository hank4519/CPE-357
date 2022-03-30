
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>  
typedef struct Node{ 
	char c; 
	int count; 
	struct Node *left; 
	struct Node *right; 
	struct Node *next; 
}node;
node *leaf (char, int);
  
node* insertnew(node*, char, int);  
void fill_array (int arr [], FILE *fp ){
	int c; 
	while ( (c = getc(fp)) != EOF) {
		if( 0 <= c && c <256) {
			//int i = c;	 
			arr[c]++;
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
node* tie_c (node *head, node *new){

	if (new -> c < head -> c) {
		new -> next = head; 
		return new; 
	}
	 node *tmp = head;
while ((tmp->next->count==new->count)&&(tmp->next->c)<(new->c)){
	 	tmp = tmp -> next;
		if(tmp -> next == NULL) break; 
	  }
	  new -> next = tmp -> next; 
	  tmp -> next = new; 
	  return head; 
}
/*
node* insertnew (node *head, char ch, int frequency ){
	node *new = leaf(ch, frequency); 
	if (new -> count < head -> count ){
		new -> next = head ;
		return new; 
	}	
	else if (new -> count == head -> count){
		head = tie_c (head, new); 
		return head; 
	}
	else{
		node *tmp = head;  
		while(tmp -> next != NULL && (tmp->next->count) < (new->count)){
			tmp = tmp ->next; 
		}
		if(tmp ->next  == NULL) {
			new -> next = tmp -> next; 
			tmp -> next = new; 
			return head; 
		}	
		if (tmp->next->count == new -> count){
			tmp ->next = tie_c (tmp->next, new); 
			return head;
		}
		new -> next = tmp -> next; 
		tmp -> next = new;
		return head; 
			
	}
}*/
node* insertnew (node* head, char ch, int frequency){
	node *new = leaf(ch, frequency); 
	if(new -> count < head -> count ){
		new -> next = head;
		return new; 
	}
	node *tmp = head;
	while(tmp -> next != NULL && (new -> count >= tmp -> next -> count)) {
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
		int c = head -> c; 
		res[c] = (char*) malloc (strlen(str) + 1); 
		strcpy(res[c], str);  
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
}
