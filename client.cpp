#include"utility.h"

int main(int argc,char**argv)
{
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
	perror("socket");
	exit(1);
    }
    if(connect(sock,(struct sockaddr*)&serverAddr,sizeof(serverAddr))< 0)
    {
	perror("connect");
	exit(2);
    }

    int pipe_fd[2];
    if(pipe(pipe_fd)<0)
    {
	perror("pipe error");exit(3);
    }
    
    int epfd =epoll_create(EPOLL_SIZE);
    if(epfd < 0)
    {
	perror("epoll_Create");
	exit(4);
    }

    static struct epoll_event events[2];
    addfd(epfd,sock,true);
    addfd(epfd,pipe_fd[0],true);

    bool isClientwork =true;

    char message[BUF_SIZE];

    int pid = fork();
    if(pid == 0)
    {
	close(pipe_fd[0]);
	printf("please input 'exit'to exit chat room\n");
	while(isClientwork)
	{
	    bzero(&message,BUF_SIZE);
	    fgets(message,BUF_SIZE,stdin);
	    if(strncasecmp(message,EXIT,strlen(EXIT))==0)
	    {
		isClientwork = 0;
	    }
	    else{
		if(write(pipe_fd[1],message,strlen(message)-1)<0)
		{
		    perror("fork");exit(4);
		}
	    }
	}
    }else
    {
	close(pipe_fd[1]);

	while(isClientwork)
	{
	    int epoll_events_count = epoll_wait(epfd,events,2,-1);
	    for(int i = 0;i<epoll_events_count;++i)
	    {
		bzero(&message,BUF_SIZE);
		if(events[i].data.fd == sock)
		{
		    int ret = recv(sock,message,BUF_SIZE,0);
		    if(ret == 0)
		    {
			printf("Server closed connection:%d\n",sock);
			close(sock);
			isClientwork = 0;
		    }
		    else printf("%s\n",message);
		}
		else
		{
		    int ret = read(events[i].data.fd,message,BUF_SIZE);
		    if(ret == 0)isClientwork = 0;
		    else
		    {
			send(sock,message,BUF_SIZE,0);
		    }
		}
 	    }
	}
    }
    if(pid){
	close(pipe_fd[0]);
	close(sock);
    }else
    {
	close(pipe_fd[1]);
    }
    return 0;
}
