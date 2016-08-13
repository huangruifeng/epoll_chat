
#include<iostream>
#include<list>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

using namespace std;

list<int> clients_list;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define EPOLL_SIZE 5000

#define BUF_SIZE 0xffff

#define SERVER_WELCOME "welcome you join the chat room ! your chat ID is : client #%d"
#define SERVER_MESSAGE "clientID %d say >> %s"

#define EXIT "EXIT"

#define CAUTION "there is only one int the char room!"

int setnoblocking(int sockfd)
{
    fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK);
    return 0;
}


void addfd(int epollfd,int fd ,bool enable_et)
{
/*
    struct epoll_event
    {
	_uint32_t events;
	epoll_data_t data
    }
*/
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if(enable_et)
    {
	ev.events = EPOLLIN | EPOLLET;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
    setnoblocking(fd);
    printf("fd added to epoll!\n\n");
}

int sendBroadcastmessage(int clientfd)
{
    char buf[BUF_SIZE],message[BUF_SIZE];
    bzero(buf,BUF_SIZE);
    bzero(message,BUF_SIZE);

    printf("read from client(clientID = %d)\n",clientfd);
    int len = recv(clientfd,buf,BUF_SIZE,0);

    if(len == 0)
    {
	close(clientfd);
	clients_list.remove(clientfd);
	printf("ClientId = %d closed.\n now there are %d client in the char room\n",\
clientfd,(int)clients_list.size());
    } 
    else//roadcast message
    {
	if(clients_list.size()==1)
	{
	    send(clientfd,CAUTION,strlen(CAUTION),0);
	    return len;
	}

	sprintf(message,SERVER_MESSAGE,clientfd,buf);
   	list<int>::iterator it;
	for(it = clients_list.begin();it != clients_list.end();++it)
	{
	    if(*it != clientfd)
	    {
		if(send(*it,message,BUF_SIZE,0)<0)
		{
		    perror("error");
		    exit(-1);
		}
	    }
	}
     }
return len;
}
