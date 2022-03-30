#include <stdio.h> 
#include <fcntl.h>

int main(int agrc, char *argv){
    int fd1; 
    fd1 = open("hank.txt", O_RDONLY | O_CREAT | O_TRUNC, 0666); 
    if(fd1 < 0){
        perror("Prblem"); 
    }

    return 0;
}
