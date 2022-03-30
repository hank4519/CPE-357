#ifndef HTABLE 
#define HTABLE


typedef struct Node{
        char c;
        int count;
        struct Node *left;
        struct Node *right;
        struct Node *next;
}node;



void fill_array (int arr [], char buff[], int* num);
node* create_list(int arr[], node *head );
node* leaf (char ch, int frequency); 
node* inter_node(node *f, node*s); 
node* tie_c(node *head, node *n); 
void print_table(node *head); 
void reverse(char *str); 
node* insert(node *head, node *merge); 
int isleaf (node *n); 
node* insertnew (node *head, char ch, int frequency); 
char* getHex (int num); 
void printPreorder(node*head, char *str, int level, char *res[] ); 
void printRes (char *res[]); 
void freeTree(node *head);
void freeRes(char *res []); 
void print_arr(int arr []); 
void ini_res(char *res[]); 
char* pad_zero(char *tmp); 
char* freq_str(char* num_in_hex, int num);  
#endif
