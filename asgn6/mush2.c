
/*
 *Hank Tsai CPE 357 
 *Asgn6: mush2.c 
*/

#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <pwd.h> 
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h> 
#include <signal.h>

#include <mush.h>

#define PROMPT "8-P "
#define TRUE 1 
#define FALSE 0
#define READ_END 0
#define WRITE_END 1

/*Prototype */
void printusage(char *name); 
void prompt(char *pr);
void process_file(int argc, char *argv[]);
void process_usr();
void execute_command(pipeline pl);
void cd_command(pipeline pl);
int check_atty();
void reg_command(pipeline pl); 
void pipe_command(pipeline pl); 

/*Signal handler*/
void sig_handler(int s){
    printf("\n" ); 
}
/*Instead of exit ^C,
simply goes to the next line,
and wait for more commands*/


int main(int argc, char *argv[]){

/*struct reliable signal handling: sigaction */
struct sigaction sa; 
sa.sa_handler = sig_handler; 
sigemptyset(&sa.sa_mask); 
sa.sa_flags = 0; 
/*catch ^C*/
sigaction(SIGINT, &sa, NULL);
/*parse arguments */
if(argc > 2){
    printusage(argv[0]); 
    exit(1); 
}else if(argc == 2){
    /*process file, no printing */
    process_file(argc, argv);  
}else{
    /* Interactive mode*/
    process_usr(); 
}
pause();

//yylex_destroy(); 
return 0;
}

void process_file(int argc, char *argv[]){
    FILE *fp; 
    pipeline pl; 
    char *buffer; 
    size_t len = 0; 
    ssize_t read; 
    /*open read only file for command processing*/
    fp = fopen(argv[argc - 1], "r"); 
    if( fp == NULL) {
        exit(-1); 
    }
    /*read a line into the buffer*/
    while((read = getline(&buffer, &len, fp)) != -1){
        /*parsing argument */
        pl = crack_pipeline(buffer); 
        if(pl != NULL){
            execute_command(pl); 
        }else{
            /* Just abandon this command*/
        }
    }
    /*close file pointer*/
    fclose(fp); 
    if(buffer){
        free(buffer); 
    } 
    /*access success */
    exit(0);
}

void process_usr(){
    char *promptstr; 
    pipeline pl; 
    int run; 
    char *buffer; 
    /*Initialize prompt to 8-P  */
    promptstr = PROMPT;
    run = TRUE;
    /*Get the first prompt before looping*/
    if(run){ 
        prompt(promptstr);
    } 
    while(run){
        /*Read a line from the user */
        /*if no input */
        if( (buffer = readLongString(stdin)) == NULL){
            /*End of file, alter the flag*/
            if (feof(stdin)){
                printf("\n");
                run = FALSE; 
            }
        }else{
            /*parse the input */
            pl = crack_pipeline(buffer); 
            if(pl != NULL){
                /*debug using print function*/
                /*print_pipeline(stdout, pl); 
                  printf("cline: %s. Length: %d\n\n", 
                        pl->cline, pl->length); */

                /*pass on*/
                execute_command(pl); 
            }else{
                /*error check for null command */
                if(clerror == E_EMPTY){
                    fprintf(stderr, "Invalid null command %d.\n", lineno);
                }
            }
            /*increment lineno what's the use? */
            lineno++; 
            free(buffer); 
        }
        /*Get another prompt*/
        if(run){
            prompt(promptstr);
        }
    }
    /*destroy valgrind warning*/
    yylex_destroy();

    /*exit success */
    exit(0);  
}

void execute_command(pipeline pl){
    /*special case: cd command*/
    if(pl -> length == 1 && 
            (!strcmp(pl -> stage -> argv[0], "cd"))){
        cd_command(pl); 
        //printf("This is a cd command\n"); 
    }
    /*command that does not have pipes in it*/
    else if(pl -> length == 1){
        /*No pipe involves*/
        reg_command(pl); 
    }
    /*command with different stages for piping*/
    else if(pl -> length > 1){
        /*Pipe command */
        pipe_command(pl);  
    }

}


void pipe_command(pipeline pl){
    int num, i; 
    pid_t pid; 
    /*two pipes: old/next */
    int old[2], next[2]; 
    /*num = number of stages */
    num = pl -> length; 
    char *command, *filename; 
    /*pipe old first*/
    if(pipe(old)){
        perror("pipe"); 
        exit(-1);
    } 
    /*Connecting the head of the pipe to stdout */
    if( (pid = fork()) == 0){
        /*child stuff */
        /*dup fd to stdout */
        if(dup2(old[WRITE_END], STDOUT_FILENO) == -1){
            perror("dup2"); 
        } 
        /*close unused fds */
        close(old[READ_END]); 
        close(old[WRITE_END]); 
        /*command is the first argument*/
        command = pl -> stage[0].argv[0];
        /*check for redirection */
        if(pl -> stage[0].inname != NULL){
            filename = pl -> stage[0].inname; 
            /*create file accordingly*/
            if(freopen(filename, "r", stdin) == NULL ){ 
                perror("open");
            }
        }
        /*exec the command */
        execvp(command, pl -> stage[0].argv); 
        perror(command); 
        _exit(-1); 
    }
    
    /*for n stages, we need n - 1 pipes */
    for (i = 1; i < num ; i++){
        if(pipe(next)){
            perror("pipe");
            exit(-1);
        } 
        if( (pid = fork()) == 0 ){
            /*child stuff*/
            /*Except for the last stage: dup output to stdout*/
            if( i < num - 1){
                if(dup2(next[WRITE_END], STDOUT_FILENO) == -1){
                    perror("dup2"); 
                }
            }
            /*check for potential redirection > */
            if (i == num - 1){
                if(pl -> stage[i].outname != NULL){
                    filename = pl -> stage[i].outname; 
                    /*create file accordingly */
                    if(freopen(filename, "w+",stdout) == NULL){ 
                        perror("open"); 
                    }
                }
            }
            /*dup the old pipe's read end to stdin*/
            if(dup2(old[READ_END], STDIN_FILENO) == -1){
                perror("dup2"); 
            }
            /*close all the unused file descriptors 
                inherited from its parent */
            close(old[0]);
            close(old[1]); 
            close(next[0]); 
            close(next[1]); 

            /*execting the command */
            command = pl -> stage[i].argv[0]; 
            execvp(command, pl -> stage[i].argv); 
            perror(command);
            _exit(-1); 
        }
        /*Also close parent's file descriptors
         so that commands not hang*/
        close(old[0]);
        close(old[1]);

        /*updating old pipes with next pipes */
        old[0] = next[0];
        old[1] = next[1]; 
    }

    /*Wait for the child of number n(num) */
    while(num--){
        /*if wait fails */
        if(wait(NULL) == -1){
            perror("wait"); 
        }
    }
}


/*This is for processing command 
        without piping stages */
void reg_command(pipeline pl){
    pid_t id; 
    int status;
    char *filename;
    char *command ; 
    /*fork one child is enough*/
    id = fork();
    /*error check fork() */
    if( id == -1){
        perror("fork");
        return; 
    } else if( id == 0 ){
        /* child stuff */
        /*check redirection for '<' */
        if(pl -> stage -> inname != NULL){
            filename = pl -> stage -> inname; 
            /*create file accordingly, 
                and link it to stdin*/
            if(freopen(filename, "r", stdin) == NULL ){ 
                perror("open");
            }
            //in = open(filename, O_RDONLY); 
        }
        /*check redirection for '>' */
        if(pl -> stage -> outname != NULL){
            filename = pl -> stage -> outname;
            /*create file accordingly 
                and link stdout to it*/
            if(freopen(filename, "w+",stdout) == NULL){ 
                perror("open"); 
            }
            //out = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666); 
        }
        /*get the command */
        command = pl -> stage -> argv[0];  

        /*exec it use execvp */
        execvp(command, pl -> stage -> argv);
        /*error check*/
        perror(command);  
        _exit(1); 
    }else{
        /*wait only once: one child */
        wait(&status);

        /*check status: but no pratical use in this case */
        if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
             
        }else{
          //perror("wait");
        }   
    }
       
}

/*handling the special "cd" command*/
void cd_command(pipeline pl){
    /*env variable for "HOME" */
    char *home; 
    /*look up user's password */
    struct passwd *pw; 
    struct stat st;
    char *path; 
    /*If just cd -> change to home directory */
    if(pl -> stage -> argc == 1){
        /*Get environmental variable: getenv()*/
        home = getenv("HOME"); 
        /*If "HOME" is not defined*/
        if(home == NULL){
            /*getpwuid -> pointer to struct passwd
              getuid -> real uid of the calling process
              geteuid -> effective uid of the calling pcs*/
            pw = getpwuid(getuid()); 

            /*pw_dir -> directory path*/ 
            home = pw -> pw_dir; 
            /*For some reasons, path not dertermined*/
            if(home == NULL){
                perror("unable to determine home directory");
                return;  
            }
        }
        /*chdir the given path*/
        if( -1 == chdir(home)){
            /*error check */
            perror("chdir fails"); 
        }
    }else{
        /*Change to the specified directory*/
        path = pl -> stage -> argv[1]; 
        /*stat the given path */
        if( stat(path, &st) == -1){
            perror(path); 
            return; 
        }
        /*If it is a regular file */
        if(S_ISREG(st.st_mode)){
            /*If interactive mode is on */
            if(check_atty()){
                printf("%s: Not a directory\n", path);
            }
        /*If it is a directory */
        }else if(S_ISDIR(st.st_mode)){
            /*Try to change to the given path*/
            if(-1 == chdir(path)){
                perror("chdir fails"); 
            }
        }else{
            /*If path not exist */ 
            if(check_atty()){
                /*print msg */
                printf("%s: No such file or directory\n", path);
            }
        }
    }
}

/*Basically check if interactive mode or file mode*/ 
int check_atty(){
    if(isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)){
        return 1; 
    }
    return 0; 
}

/*Print the prompt: 8-P  */
void prompt(char *pr){
    if(isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)){
        printf("%s", pr);
        // flush stdout  
        fflush(stdout);
    } 
}

/*Print usage msg when arguments aren't right*/
void printusage(char *name){
    fprintf(stderr, "usage: %s\n", name); 
}
































































/*
    clstage tmp_stage = malloc(sizeof(clstage)); 
    int child_num, count, i; 
    int start[2]; 
    int end[2]; 
    int between[2];
    pid_t first, last, middle; 
    child_num = pl -> length; 
    count = 0;
    //printf("Initial child count: %d\n", child_num); 
    char *command; 
    if(pipe(start)){
        perror("First pipe"); 
        exit(-1); 
    }
    if(pipe(end)){
        perror("End pipe"); 
        exit(-1); 
    }
    if(pipe(between)){
        perror("In-between pipe"); 
        exit(-1); 
    } 
    if(!(first = fork())){
        // First child 
        if( -1 == dup2(start[WRITE_END], STDOUT_FILENO)){
            perror("dup2"); 
            exit(-1); 
        }
        close(start[READ_END]);
        close(start[WRITE_END]); 
        close(between[READ_END]); 
        close(between[WRITE_END]); 
        close(end[READ_END]); 
        close(end[WRITE_END]); 

        command = pl -> stage -> argv[0]; 
        execvp(command, pl -> stage -> argv);
        perror(command); 
        exit(-1); 
    }else{
        if (-1 == wait (NULL)){
            perror("wait"); 
        } 
        child_num--; 
    }
    while(child_num != 1){
        if(!(middle = fork())){
            if(count == 0){
                if(-1 == dup2(start[READ_END], STDIN_FILENO)){
                    perror("dup2"); 
                    exit(-1); 
                }
                if(-1 == dup2(between[WRITE_END], STDOUT_FILENO)){
                    perror("dup2"); 
                    exit(-1); 
                }
                close(start[READ_END]); 
                close(start[WRITE_END]); 
                close(between[READ_END]); 
                close(between[WRITE_END]);
                close(end[READ_END]);
                close(end[WRITE_END]); 
                count++; 
                tmp_stage = pl -> stage -> next; 
                command = tmp_stage -> argv[0]; 
                execvp(command, tmp_stage -> argv); 
                perror(command); 
                exit(-1); 
            }else if(count > 0){
                if (-1 == dup2(between[READ_END], STDIN_FILENO)) {
                    perror("dup2"); 
                    exit(-1); 
                }
                if (-1 == dup2(between[WRITE_END], STDOUT_FILENO)){
                    perror("dup2"); 
                    exit(-1); 
                }
                close(start[READ_END]);
                close(start[WRITE_END]); 
                close(between[READ_END]); 
                close(between[WRITE_END]);
                close(end[READ_END]) ;
                close(end[WRITE_END]);
                count++; 
                tmp_stage = pl -> stage -> next; 
                for(i = 1; i < count; i++){
                    tmp_stage = tmp_stage -> next; 
                } 
                command = tmp_stage -> argv[0]; 
                execvp(command, tmp_stage -> argv); 
                perror(command); 
                exit(-1); 
            }   
        }else{
            // If it is parent
            if( -1 == wait(NULL)){
                perror("wait");
            }    
            child_num--;
        }
    }
    //Dealing with the end of the pipe 
    if(!(last = fork())){
        if(-1 == dup2(between[READ_END], STDIN_FILENO)){
            perror("dup2"); 
            exit(-1); 
        } 
        close(start[READ_END]);
        close(start[WRITE_END]); 
        close(between[READ_END]); 
        close(between[WRITE_END]); 
        close(end[READ_END]); 
        close(end[WRITE_END]); 
        tmp_stage = pl -> stage; 
        for(i = 1; i < count; i++ ){
            tmp_stage = tmp_stage -> next; 
        }
        command = tmp_stage -> argv[0];
        execvp(command, tmp_stage -> argv); 
        perror(command); 
        exit(-1); 
    }else{
        close(start[READ_END]); 
        close(start[WRITE_END]);
        close(between[READ_END]); 
        close(between[WRITE_END]); 
        close(end[READ_END]); 
        close(end[WRITE_END]);
        free(tmp_stage);  
        if(-1 == wait(NULL)){
            perror("wait"); 
        }
        child_num--; 
    }
    if(child_num == 0){
        printf("Success with 0 child count at exit\n"); 
    }else{
        //printf("Failure\n"); 
    }

*/




/*
    int stage_count = pl -> length; //4
    int fd [stage_count - 1][2];
    pid_t children [stage_count]; 
    int i, count; 
    char *command;
    count = 0; 
    for(i = 0; i < stage_count; i++){
        if(pipe(fd[i])){
            perror("pipe");
            exit(-1); 
        }
    }
    //start end of a pipe: stdout
    if(!(children[count] = fork())){
        if( dup2 (fd[count][WRITE_END], STDOUT_FILENO) == -1){
            perror("dup2XXXX"); 
            exit(-1); 
        }
        close_other_fd(fd, stage_count); 
        command = pl -> stage[count].argv[0]; 
        printf("First exec: %s\n", pl -> stage[count].argv[0]);
        count++; 
        execvp(command, pl -> stage[count].argv); 
        perror(command); 
        exit(-1);  
    }
    //intermediate: stdin/stdout
    for(i = 0; i < stage_count - 1; i++){
        if(!(children[count] = fork())){
            if(dup2(fd[count][READ_END], STDIN_FILENO)){
                perror("dup2OOOO"); 
                exit(-1); 
            }
            if(dup2(fd[count][WRITE_END], STDOUT_FILENO)){
                perror("dup2");
                exit(-1); 
            }
            close_other_fd(fd, stage_count); 
            command = pl -> stage[count].argv[0];  
            printf("What am I exec: %s\n", pl -> stage[count].argv[0]);
            count++; 
            execvp(command, pl -> stage[count].argv); 
            perror(command); 
            exit(-1); 
        }
    }
    //end of a pipe: stdin 
   if(!(children[count] = fork())){
        //last child
        if(dup2(fd[count][READ_END], STDIN_FILENO) == -1){
            perror("dup2FFF"); 
            exit(-1); 
        } 
        close_other_fd(fd, stage_count); 
        command = pl -> stage[count].argv[0];
        printf("Last exec: %s\n", pl->stage[count].argv[0]);
        count++;
        execvp(command, pl -> stage [count].argv);
        perror(command); 
        exit(-1); 
   }
    //parent
    close_other_fd(fd, stage_count); 
    for(i =0; i < stage_count; i++ ){
        if(wait (NULL) == -1){
            perror("wait");
        }

    }
    printf("Get to the END\n");  
    printf("Count is %d\n", count); 


    */





/*


    int num = pl -> length;
    printf("Number of stage: %d\n", num);  
    char *command; 
    int old[2], next[2]; 
    int count, tmp, i; 
    count = 0; 
    pid_t child; 
    if(pipe(old)){
        perror("pipe"); 
        exit(-1); 
    }
    child = fork(); 
    if(child == 0){
        if( dup2(old[WRITE_END], STDOUT_FILENO) == -1){
            perror("dup2");
            exit(-1); 
        }
        close(old[WRITE_END]); 
        close(old[READ_END]); 
        command = pl -> stage[count].argv[0];
        count++; 
        tmp = count - 1; 
        printf("First exec:  %s\n", command); 
        execvp(command, pl -> stage[tmp].argv); 
        perror(command); 
        exit(-1); 
    }
    for(i =1; i <num ;i++ ){
        close(old[WRITE_END]); 
        printf("HEY\n");
        if( i < num - 1) {
            if(pipe(next)){
                perror("pipe next"); 
                exit(-1); 
            }
        }
        child = fork(); 
        if(child == 0){
            printf("It should go three times\n" );
            if(dup2(old[READ_END], STDIN_FILENO) == -1){
                perror("dup2"); 
            }
            if( i < num - 1 ){
                if( dup2(next[WRITE_END], STDOUT_FILENO) == -1){
                    perror("dup2"); 
                }
            }
            printf("Also should TRHEE\n");
            close(old[READ_END]); 
            close(old[WRITE_END]);
            close(next[READ_END]); 
            close(next[WRITE_END]);
            printf("Value of the count: %d\n", count);  
            command = pl -> stage[count].argv[0]; 
            count++;
            tmp = count - 1;
            printf("What am I execing:  %s\n", command); 
            execvp(command, pl -> stage[tmp].argv); 
            perror(command);
            exit(-1); 
        }

        close(old[READ_END]); 
        close(old[WRITE_END]); 
        old[READ_END] = next[READ_END]; 
        old[WRITE_END] = next[WRITE_END];
    }
    while(num --){
        if (wait(NULL) == -1){
            perror("wait"); 

        }
    }


*/

/* 
 
    pid_t pid; 
    int fd[2]; 
    int num, i, tmp;
    num = pl -> length;
    for(i = 0; i < num; i++ ){
        if(pipe(fd)){
            perror("pipe"); 
            exit(-1); 
        }
        //printf("It should run 3 times\n");
        if ( ( pid = fork()) == 0) {
            //printf("ALSO THREE\n");
            //if( i != 0 ){ 
                close(fd[WRITE_END]);
                //fd[READ_END] = STDIN_FILENO; 
            //}
        
            if( i == 0 ){
                //printf("DUP FIRST\n");
                dup2(fd[WRITE_END], STDOUT_FILENO); 
                close(fd[WRITE_END]);
                
            }    
            else if( i == num - 1){
                //printf("DUP LAST\n");
                //close(fd[WRITE_END]);
                dup2(fd[READ_END], STDIN_FILENO);
                close(fd[READ_END]); 
            }else{
                //printf("DUP IN BETWEEN \n");
                dup2(fd[READ_END], STDIN_FILENO); 
                close(fd[READ_END]); 
                dup2(fd[WRITE_END], STDOUT_FILENO); 
                close(fd[WRITE_END]); 
            }
            printf("What am I execing %s\n", pl -> stage[i].argv[0]);
            execvp(pl -> stage[i].argv[0], pl -> stage[i].argv);
            perror("exec"); 
            exit(-1);  
        } 
        close(fd[READ_END]); 
        close(fd[WRITE_END]);
        tmp = 
    }
    close(fd[READ_END]); 
    close(fd[WRITE_END]);
    for(i = 0; i < num - 1; i++ ){
        if(wait(NULL) == -1){
            perror("wait"); 
        }
    }


    */


/* 
    pid_t pid; 
    int old[2], next[2]; 
    int num, i;
    num = pl -> length;
    pipe(old); 
    i = 0; 
    if( (pid = fork()) == 0){
        close(old[READ_END]); 
        dup2(old[WRITE_END], STDOUT_FILENO); 
        execvp(pl -> stage[i].argv[0], pl -> stage[i].argv);
    }
    for(i = 1; i < num; i++ ){
        if(pipe(next)){
            perror("pipe"); 
            exit(-1); 
        }
        //printf("It should run 3 times\n");
        if ( ( pid = fork()) == 0) {
            //printf("ALSO THREE\n");
            //if( i != 0 ){ 
                //&&&close(fd[WRITE_END]);
                //fd[READ_END] = STDIN_FILENO; 
            //}
        
            //if( i == 0 ){
                //printf("DUP FIRST\n");
                //close(next[READ_END]);
                //dup2(next[WRITE_END], STDOUT_FILENO); 
                //in = fd[READ_END]; 
                //close(fd[WRITE_END]);
                
            //}    
            if( i == num - 1){
                //printf("DUP LAST\n");
                //close(fd[WRITE_END]);
                close(next[WRITE_END]); 
                dup2(next[READ_END], STDIN_FILENO);
                //close(fd[READ_END]); 
            }else{
                //printf("DUP IN BETWEEN \n");
                dup2(next[READ_END], STDIN_FILENO); 
                //close(fd[READ_END]); 
                dup2(next[WRITE_END], STDOUT_FILENO); 
                //close(fd[WRITE_END]); 
            }
            printf("What am I execing %s\n", pl -> stage[i].argv[0]);
            execvp(pl -> stage[i].argv[0], pl -> stage[i].argv);
            perror("exec"); 
            exit(-1);  
        } 
        
        close(old[READ_END]); 
        close(old[WRITE_END]);
        old[0] = next[0]; 
        old[1] = next[1];
        
    }
    close(next[READ_END]); 
    close(next[WRITE_END]);
    for(i = 0; i < num - 1; i++ ){
        if(wait(NULL) == -1){
            perror("wait"); 
        }
    }

    */
