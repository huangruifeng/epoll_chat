#include"utility.h"

int main(int argc,char**argv)
{
    struct sockaddr_in serverAddr;
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    int listener = socket(AF_INET,SOCK_STREAM,0);
    if(listener < 0)
    {
	exit(-1);
    }
    if(bind(listener,(struct sockaddr*)&serverAddr,sizeof(serverAddr))<0)
    {
	perror("bind");
	exit(-2);
    }
    
    int ret = listen(listener,5);
    if(ret < 0)
    {
	perror("listen");
	exit(-3);
    }
    printf("Start to listen:%s\n",SERVER_IP);

//epoll
    /* 
	create shijianbiao
    */
    int epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0)
    {
	perror("epoll_create");
	exit(-4);
    }
    printf("epoll create,epollfd = %d\n",epfd);

    static struct epoll_event events[EPOLL_SIZE];
    addfd(epfd,listener,true);

   while(1)
    {
	int epoll_events_count = epoll_wait(epfd,events,EPOLL_SIZE,-1);
	if(epoll_events_count < 0)
	{
	    perror("epoll failure");
	    break;
	}
	printf("epoll_events_count = %d\n",epoll_events_count);

	for(int i = 0;i<epoll_events_count;++i)
	{
	    int sockfd = events[i].data.fd;
	    if(sockfd == listener)
	    {
		struct sockaddr_in client_address;
		socklen_t client_addrLength = sizeof(struct sockaddr_in);
		int clientfd = accept(listener,(struct sockaddr*)&client_address,&client_addrLength);
		printf("client connection from:%s:%d,c;ientfd = %d \n",inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port),clientfd);
		addfd(epfd,clientfd,true);

		clients_list.push_back(clientfd);
		printf("Add new clientfd = %d tp epoll\n",clientfd);
		printf("NOw there are %d clients int the chat room\n",(int)clients_list.size());
		
		printf("welcome message\n");
		char message[BUF_SIZE];
		bzero(message,BUF_SIZE);
		sprintf(message,SERVER_WELCOME,clientfd);
		int ret = send(clientfd,message,BUF_SIZE,0);
		if(ret < 0)
		{
		    perror("send error");
		    exit(-6);
		}
	    }
	    else
	    {
		int ret = sendBroadcastmessage(sockfd);
		if(ret < 0)
		{
		    perror("error");exit(-7);
		} 
	    }
	}
    }
    close(listener);
    close(epfd);
    return 0;
}
