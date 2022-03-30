#include <unistd.h> 
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <stdlib.h> 

int main(int argc, char *argv[]){
int id;
/*Check the usage of forkit*/
if( argc != 1 ){
    perror("Usage: ./forkit");
    exit(1); 
}
/*Greet the world*/
printf("Hello, world!\n"); 
if ((id = fork()) == -1){
    perror("fork failed"); 
    exit(1); 
}
/*If not child, wait*/
if (id != 0){
    wait(NULL); 
}
/*If it's child, print out its pid */
if(id == 0){
    printf("This is the child, pid %d\n", getpid());
    exit(0); 
}else{
/*It is the parent*/
    printf("This is the parent, pid %d\n", getpid());    
}
/*Say goodbye*/
printf("This is the parent, pid %d, signing off.\n", getpid()); 

return 0; 
}
