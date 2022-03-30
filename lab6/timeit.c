#include <stdio.h> 
#include <stdlib.h> 
#include <signal.h> 
#include <sys/time.h> 
#include <unistd.h> 

/*usage for error handling*/
void usage(const char *name){
	fprintf(stderr, "usage: %s", name); 
	exit(1); 
}
/*signal handler for sigaction*/
void handler(int signum){
	static int n = 1; 
	if( n++ % 2){
	   printf("Tick..."); 
	}else{
	   printf("Tock\n"); 
	} 
	/*flush out stdio stream*/
	fflush(stdout); 

}
int main(int argc, char *argv[]) {
	int num; 
	struct sigaction sa; 
	struct itimerval it; 
	char *end;
	//sigset_t mask;  
	if (argc != 2 ){
	   usage(argv[0]); 
	} 
	num = strtol(argv[1], &end, 10); 
	/*If argument is less than 0 or not a number*/
	if (num < 0 || *end ){
	   usage(argv[0]); 	
	}
	sa.sa_handler = handler;
	/*Initialize signal mask*/ 
	sigemptyset(&sa.sa_mask); 
	sa.sa_flags = 0; 
	/*activate the timer*/
	if( sigaction(SIGALRM, &sa, NULL) == -1){
	   perror("Sigaction failed");
	   exit(EXIT_FAILURE); 
	} 
	//sigemptyset(&mask); 
	/*adds SIGALRM signal to mask */
	//sigaddset(&mask, SIGALRM); 
	/*indicate the mask signal should be blocked*/
	//if(sigprocmask(SIG_BLOCK, &mask, &sa.sa_mask) == -1){
	//     perror("sigprocmask failed"); 
	//   exit(EXIT_FAILURE); 
	//}
	/*Delete IGALRM from mask */
	//sigdelset(&sa.sa_mask, SIGALRM); 
	it.it_interval.tv_sec = 0;
	/*half a second*/ 
	it.it_interval.tv_usec = 500000; 
	it.it_value = it.it_interval;
	/* Set ITIMER_REAL to timer it*/ 
 	if (setitimer(ITIMER_REAL, &it, NULL) == -1){	
	   perror("setitimer failed"); 
	   exit(EXIT_FAILURE); 
	}	
	/* tick tpck times*/
	num *= 2; 
	while ( num-- ){
	   /* waut for signal to be delivered*/
	   sigsuspend(&sa.sa_mask); 
	}
	printf("Time's up!\n"); 
return 0; 
}
