#include <arpa/inet.h> 
#include <talk.h> 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <poll.h> 

#define LOCAL 0
#define REMOTE 1
#define MAX 1024
#define BACKLOG 10 
#define ZERO 0 
#define COLLECTION 1
#define WAIT 100
#define OK_MSG "ok"
#define DENY_MSG "no"
#define EXIT "Connection closed.  ^C to terminate."
void chatserver(int sockfd, int N_flag); 
void chatclient(int clinet_fd, int N_flag);
void client_action(int v_flag, int a_flag, int N_flag, char *hostname, char *port){
    int client_fd, port_id, len; 
    struct sockaddr_in sa; 
    struct hostent *hostent; 
    char permission[2]; 
    char *username; 
    permission[0] = '\0'; 
    /*Look up peer address*/
    if ((hostent = gethostbyname(hostname)) == NULL){
        perror("could not get host"); 
        exit(1); 
    } 
    /*Create the socket */
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("failed to create socket"); 
        exit(EXIT_FAILURE);  
    }
    /*set variables for sockaddr_in */
    sa.sin_family = AF_INET;
    port_id = atoi(port);
    sa.sin_port = htons(port_id); 
    sa.sin_addr.s_addr = *(uint32_t*)hostent -> h_addr_list[0];
    /*verbosity*/
    if(v_flag){
        printf("Options:\n"); 
        printf("int       opt_verbose = %d\n", v_flag); 
        printf("talkmode  opt_mode    = client\n"); 
        printf("int       opt_port    = %d\n", atoi(port)); 
        printf("char     *opt_host    = %s\n", hostname); 
        printf("int       opt_accept  = %d\n", a_flag); 
        printf("int       opt_windows = %d\n", N_flag); 
    }
    /*connect to the server*/
    if ((connect(client_fd, (struct sockaddr*)&sa, sizeof(sa)))==-1){
        perror("failed to connect");
        exit(EXIT_FAILURE); 
    }  
    printf("Waiting for response from %s\n", hostname);
    /*get and pass user name*/
    username = getlogin(); 
    if (username == NULL){
        perror("can not get username"); 
        exit(1); 
    } 
    if((send(client_fd, username, sizeof(username), 0)) == -1){
        perror("send fail"); 
    } 
    /*receive chat permission from server */
    len = recv(client_fd, permission, sizeof(permission), 0);
    if (len == -1){
       perror("recv fail"); 
       exit(1); 
    }
    /*compare it with "ok" */  
    if (!strncmp(permission, OK_MSG, len)){ 
        /*start chatting */
        chatclient(client_fd, N_flag); 
    }else{
        printf("%s declined connection.\n", hostname);
        close(client_fd); 
        exit(0); 
    }
    /*close remaining socket*/
    close(client_fd); 
}
void server_action(int v_flag, int a_flag, int N_flag, char *port){
    char input; 
    int server_fd, newsock_fd;
    int port_id; 
    struct sockaddr_in address, newsockinfo, peerinfo;
    char localaddr[INET_ADDRSTRLEN], peeraddr[INET_ADDRSTRLEN],
         hbuf[NI_MAXHOST], username[MAX];   
    socklen_t len; 
    /*Create the server socket*/
    if( (server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("failed to create socket"); 
        exit(EXIT_FAILURE); 
    }   
    /* Initialize sockaddr_in address*/
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    port_id = atoi(port); 
    address.sin_port = htons(port_id); 
    /*bind this socket to the address */
    if (bind(server_fd, (struct sockaddr *)&address, 
    sizeof(address)) == -1){
        perror("fail to bind address");
        exit(EXIT_FAILURE);     
    }
    /*verbosity*/
    if(v_flag){
        printf("Options:\n");
        printf("int       opt_verbose = %d\n", v_flag); 
        printf("talkmode  opt_mode    = servser\n"); 
        printf("int       opt_port    = %d\n", atoi(port)); 
        printf("char     *opt_host    = (none)\n"); 
        printf("int       opt_accept  = %d\n", a_flag); 
        printf("int       opt_windows = %d\n", N_flag); 
        printf("VERB: Waiting for connection...");
        fflush(stdout);  
    }
    /*Wait for a connection with listen */ 
    if(listen(server_fd, BACKLOG) == -1){
        perror("failed to listen"); 
        exit(EXIT_FAILURE); 
    }
    len = sizeof(newsock_fd); 
    /*Accept a connection with accpet */
    if( (newsock_fd = accept(server_fd, (struct sockaddr *)&peerinfo,
            &len)) == -1){
        perror("failed to accept"); 
        exit(EXIT_FAILURE);
    } 
    /*pop info to newsockinfo */
    len = sizeof(newsockinfo);
    if((getsockname(newsock_fd, (struct sockaddr *)&newsockinfo, &len))
           == -1){
        perror("getsockname fails"); 
        exit(1); 
    }  
    /*get IP address for printing */
    inet_ntop(AF_INET, &newsockinfo.sin_addr.s_addr, 
                localaddr,sizeof(localaddr));  
    inet_ntop(AF_INET, &peerinfo.sin_addr.s_addr, 
                peeraddr, sizeof(peeraddr));
    newsockinfo.sin_family = AF_INET;
    /*verbosity: second part*/
    if(v_flag){
        printf("New connection from: %s:%d\n",
                peeraddr, ntohs( peerinfo.sin_port)); 
    } 
    /*check accpet flag */
    if(a_flag){
        if(send(newsock_fd, OK_MSG, strlen(OK_MSG), 0)==-1){
            perror("send fails"); 
        }
        /*receive extra username*/
        recv(newsock_fd, username, sizeof(username), 0); 
    } 
    else{
        /* get hostname*/
        if(getnameinfo((struct sockaddr*)&newsockinfo, sizeof(struct sockaddr_in),
                   hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD)!= 0){
            perror("could not resolve host name"); 
        }
        /*get username */
        if(recv(newsock_fd, username, sizeof(username), 0) == -1){
            perror("recv fails"); 
        } 
        printf("Mytalk request from %s@%s.  Accept(y/n)?", username,
                hbuf); 
        scanf("%c", &input);
        /*decline permission */
        if(input != 'y'){
            send(newsock_fd, DENY_MSG, strlen(DENY_MSG),0); 
            exit(0); 
        }else{
            send(newsock_fd, OK_MSG, strlen(OK_MSG), 0); 
        }
    }
    /*start chatting and close remaining sockets */
    chatserver(newsock_fd, N_flag);    
    close(server_fd); 
    close(newsock_fd); 
}   

void chatserver(int sockfd, int N_flag){
    int len, done = 0;
    ssize_t mlen; 
    struct pollfd fds[COLLECTION + 1]; 
    char buff [MAX];
    /*if N flag is on, do not turn on window */
    if(!N_flag){
        start_windowing(); 
    }
    buff[0] = '\0';
    /*initialize pollfd structure for two-way communiation*/ 
    fds[LOCAL].fd = STDIN_FILENO;
    fds[LOCAL].events = POLLIN; 
    fds[LOCAL].revents = 0; 
    fds[REMOTE] = fds[LOCAL]; 
    fds[REMOTE].fd = sockfd; 
    while(!done){
        /*check which fd is ready to read*/
        poll(fds, 2, -1);  
        if(fds[LOCAL].revents & POLLIN){
            /*update input buffer*/
            update_input_buffer(); 
            /*if has whole line */
            if(has_whole_line()){
                len = read_from_input(buff, MAX);
                /*if receive EOF */
                if(len == 0 && has_hit_eof()){
                    send(sockfd, EXIT, sizeof(EXIT) -1,0);
                    if(!N_flag){
                        stop_windowing();
                    } 
                    close(sockfd); 
                    return; 
                /*If receive EOF after some text */
                }else if(has_hit_eof()){
                    buff[strlen(buff)-1] = '\n'; 
                    send(sockfd, buff, len, 0);
                    memset(buff, 0, sizeof(buff));
                    send(sockfd, EXIT, sizeof(EXIT) -1, 0);
                    if(!N_flag){
                        stop_windowing();
                    }
                    close(sockfd); 
                    return; 
                /*chatting.. */
                }else{
                    send(sockfd, buff, len, 0);
                    memset(buff, 0, sizeof(buff));
                }
            } 
        }
        /*write to windowing*/
        if(fds[REMOTE].revents & POLLIN){
            mlen = recv(fds[1].fd, buff, sizeof(buff), 0);
            /*If the connection is lost on the other end*/
            if(mlen == 0 ){
               /*Wirte EXIT msg*/
               write_to_output(EXIT, sizeof(EXIT)-1);
               close(sockfd);
               /*wait a hundred second for user to terminate*/
               sleep(WAIT); 
               if(!N_flag){
                  stop_windowing(); 
               } 
               return;  
            }else{
                write_to_output(buff, mlen); 
            } 
            memset(buff,0, sizeof(buff)); 
        }
    }
    /*N-flag on, no windowing to stop*/
    if(!N_flag){
        stop_windowing(); 
    }
    close(sockfd);
}
/*Same things as chatserver */
void chatclient(int client_fd, int N_flag){
int len, mlen, done; 
char buff [MAX]; 
struct pollfd fds [COLLECTION + 1];
done = 0; 
fds[LOCAL].fd = STDIN_FILENO;
fds[LOCAL].events = POLLIN;
fds[LOCAL].revents = 0; 
fds[REMOTE] = fds[LOCAL];
fds[REMOTE].fd = client_fd;
if(!N_flag){
    start_windowing(); 
}
/*Same thing as chatserver*/
while (!done){
    poll(fds, 2, -1);
    /*For reading input*/
    if(fds[LOCAL].revents & POLLIN){
        update_input_buffer(); 
        if(has_whole_line()){
            len = read_from_input(buff, MAX); 
            if (len == 0 && has_hit_eof()){
                send(client_fd, EXIT, sizeof(EXIT)- 1, 0);
                memset(buff, 0, sizeof(buff)); 
                if(!N_flag){
                    stop_windowing();
                } 
                close(client_fd); 
                return;
            }else if(has_hit_eof()){
                buff[strlen(buff)-1] = '\n'; 
                send(client_fd, buff, len, 0); 
                memset(buff, 0, sizeof(buff)); 
                send(client_fd, EXIT, sizeof(EXIT)-1, 0); 
                if(!N_flag){
                    stop_windowing(); 
                }
                close(client_fd); 
                return; 
            }else{
                send(client_fd, buff, len, 0);
                memset(buff, 0, sizeof(buff)); 
            }
        }
    }
    /*For printing output */
    if(fds[REMOTE].revents & POLLIN){
        mlen = recv(fds[1].fd, buff, sizeof(buff), 0);
        if(mlen == 0){
            write_to_output(EXIT, sizeof(EXIT)-1); 
            close(client_fd); 
            sleep(WAIT); 
            if(!N_flag){
                stop_windowing();
            }
            return; 
        }else{
            write_to_output(buff, mlen);
        }
        memset(buff, 0, sizeof(buff)); 
    }
}
if(!N_flag){
    stop_windowing(); 
}
close(client_fd); 
}

int main(int argc, char *argv[]){
int option, flags = 0; 
int v_flag = 0; 
int a_flag = 0; 
int N_flag = 0;
int client_mode = 0;
char *hostname=""; 
char *port ="";
/*argument too less */
if(argc < 2){
    perror("usage: mytalk [-v] [-a] [-N] [hostname] port");
    exit(3); 
}
/*getopt and count total flags */
while((option = getopt(argc, argv, "vaN")) != -1){
   switch(option){
   case 'v': 
	    v_flag = 1;
		flags++;
		break; 
	case 'a': 
        a_flag = 1;
		flags++; 
        break;  
	case 'N':
	 	N_flag = 1; 
		flags++; 
        break;  
	case '?':
		break;
    default: 
        abort();  
   }
}
/*based on flag count, determine server/client*/
if(argc - flags > 2){
    hostname = argv[optind]; 
	client_mode = 1; 
    port = argv[++optind]; 
}else{
    port = argv[optind];
}
/*client_mode flag */
if(client_mode){
    client_action(v_flag, a_flag, N_flag, hostname, port); 
}else{
    server_action(v_flag, a_flag, N_flag, port); 
}
return 0; 
}



/* 
printf("flags total: %d\n", flags); 
printf("v flags: %d\n", v_flag); 
printf("a flags: %d\n", a_flag); 
printf("N flags: %d\n", N_flag); 
printf("Client mode? %d\n", client_mode);
printf("hostname: %s\n", hostname); 
printf("port name: %s\n", port);
 */
