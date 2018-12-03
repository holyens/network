/**
 * cmd: gcc -o httpserver httpserver.c -lpthread -std=gnu99
 * run: ./main [-p 1883][-m 6][-d .]
 * p:port
 * m:thread_max_num
 * d:rootdir
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <semaphore.h>
#define MTU 1490
const char *HttpStatus200 = "HTTP/1.1 200 OK";
const char *HttpStatus404 = "HTTP/1.1 404 Not Found";

#define ContentTypeNum 7
const char *ContextType[][2]={{".html","text/html"},
							{".js","application/x-javascript"},
							{".gif","image/gif"},
							{".ico","image/x-icon"},
							{".jpeg","image/jpeg"},
							{".jpg","image/jpeg"},
							{".png","image/png"}};
enum{html,js,gif,ico,jpeg,jpg,png};
int sockfd;
int threadCounter=0;
sem_t sem;
void *SubThread(void *sfd)
{
	int connfd = *((int*)sfd);
	char buff[MTU];
	struct sockaddr_in cliAddr;	
	threadCounter++;
	/*Get client IP&PORT*/
	int cliAddrLen = sizeof(cliAddr);
	getpeername(connfd, (struct sockaddr*)&cliAddr, &cliAddrLen);
	//printf("new client:%s:%d\n",inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port));
	//while(1){
		int n = read(connfd, buff, MTU);
		if (n<=0){	/*read failure*/
			if(n<0)
				perror("\tSubThread recv");
			//break;
		}
		else{	/*read success*/			
			char file[128];  
			/*Get request filename*/
			sscanf(buff,"%*s %s %*s",file);
			printf("%s:%u %d GET %s\n", inet_ntoa(cliAddr.sin_addr), ntohs(cliAddr.sin_port),n, file);
			if(file[strlen(file)-1]=='/')	/*Set default file when requesting directory*/
				strcat(file,"index.html");
			int usedbuflen = 0;
			int fd = open(file,O_RDONLY);
			if(fd<0){	/*File does not exist or is forbidden to access*/
				usedbuflen += sprintf(buff+usedbuflen,"%s\r\nContent-Length: %d\r\n\r\n",HttpStatus404,0);
				write(connfd,buff,usedbuflen);
				usedbuflen = 0;
				perror("\tSubThread openfile");
			}else{
				usedbuflen += sprintf(buff+usedbuflen,"%s\r\n",HttpStatus200);
				struct stat filestat;
				stat(file, &filestat);
				int length = filestat.st_size;	/*Get file length(bytes)*/
				/*Parsing the file by file extension*/
				char *ext = strrchr(file, '.');
				for(int i=0;i<ContentTypeNum;i++){
					if(strncmp(ext,ContextType[i][0],8)==0)
						usedbuflen += sprintf(buff+usedbuflen,"Content-Type: %s\r\n",ContextType[i][1]);
				}
				usedbuflen += sprintf(buff+usedbuflen,"Content-Length: %d\r\n\r\n",length);
				/*Read and send file content*/
				do{
					int rdfilelen = read(fd,buff+usedbuflen,MTU-usedbuflen);
					if(rdfilelen<=0){
						break;
					}else{
						usedbuflen += rdfilelen;
						write(connfd,buff,usedbuflen);
						usedbuflen = 0;
					}					
				}while(1);
			}
			close(fd);			
		}
		close(connfd);	
	//}
	threadCounter--;
	sem_post(&sem);
	//printf("%s:%d client exit\n",inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port));
	return NULL;
}
void StopHandler(int s)
{
	printf("\nTerminated by the user\n");
	printf("The remaining %d threads before the end\n",threadCounter);
	close(sockfd);
	sem_destroy(&sem);
	exit(0);
}
int main(int argc, char *argv[])
{
	unsigned short int serverPort = 80;
	struct sockaddr_in addr;
	int addrLen;
	int thread_max = 6;
	/*Parse parameters*/
	char rootdir[128] = ".";
	int optch;
    while((optch = getopt(argc, argv, "p:m:d:")) != -1)
    {
        switch(optch)
        {
            case 'p':
                serverPort = atoi(optarg);
				if(serverPort==0){
					fprintf(stderr,"Parameter -p error!\n");
					return -1;
				}
                break;
            case 'm':
                thread_max = atoi(optarg);
				if(thread_max==0){
					fprintf(stderr,"Parameter -m error!\n");
					return -1;
				}
				break;
			case 'd':
				strncpy(rootdir,optarg,128);
				if(chdir(rootdir)<0){
					fprintf(stderr,"Parameter -d error:%s!\n",rootdir);
					return -1;
				}
                break;
        }
    }
	/*Limited the range of directory*/
	getcwd(rootdir,128);
	if(chroot(rootdir)<0 || chdir("/")<0){
		perror("\tmain set rootdir");
		return -1;
	}
	/*set signal*/
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, StopHandler);
	/*create socket*/
	if((sockfd=socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("\tmain create socket");
		return -1;
	}
	/*set TCP listen*/
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverPort);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		perror("\tmain bind");
		return -1;
	}
    if(listen(sockfd, 5)<0){
        perror("\tmain listen\n");
        return -1;
    }
	printf("listen in %s:%d thread_max=%d rootdir=\"%s\"\n",\
				inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),thread_max,rootdir);
	sem_init(&sem, 0, thread_max);
	while (1){
		int connfd;
		pthread_t tid;
		sem_wait(&sem);	/*Wait until the number of threads falls below the max limit*/
		connfd = accept(sockfd, (struct sockaddr*)&addr, &addrLen);
		if(connfd<0){
			perror("\tmain accept");
			break;
		}
		else{
			int ret = pthread_create(&tid, NULL, SubThread, &connfd);
			if(ret){
				perror("\tmain pthread_create");
				break;
			}
			pthread_detach(tid);	//detech thread
		}
	}
	close(sockfd);
	sem_destroy(&sem);
	printf("main exit\n");
    return -1;
}
