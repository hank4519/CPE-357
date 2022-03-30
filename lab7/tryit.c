#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/stat.h> 
#include <sys/types.h> 

int main(int argc, char *argv[]){
    int id, status; 
    struct stat st;   
    /*check usage */
    if(argc != 2){
        fprintf(stderr, "uasge: tryit command\n");
        exit(1); 
    }
    /*check if file exits*/
    if ((stat(argv[1], &st)) != 0){
        fprintf(stderr, "%s: No such file or directory\n", argv[1]);
        id = fork(); 
        /*Exited with child's status*/
        if (id != 0){
            printf("Process %d exited with an error value.\n", id); 
        }
        exit(1); 
    }
    /*construct execvp's argument*/
    char *argument[] = {argv[1], NULL}; 
    id = fork();
    if(id == -1){
        perror("fork fails"); 
        exit(1);
    } 
    /*If child, exec the given program*/
    if(id == 0){ 
        if (execvp(argv[1], argument) < 0 ){
            perror(argv[1]); 
            exit(EXIT_FAILURE); 
        }
        printf("This shouldn been printed\n" ); 
    }else{
    /*If parent, report on child's success */
        wait(&status);
        /*check child's status*/
        if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
            printf("Process %d succeeded.\n", id); 
        } else{
            printf("Process %d exited with an error value.\n", getpid());
        }
    }
    
    return 0; 
}
