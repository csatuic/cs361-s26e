#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<sys/epoll.h>
#include<stdlib.h>

int main(int argc, char** argv) {

  struct sockaddr_in serv_addr = {0};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(8765);

  int serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if(serverfd==-1) {
	perror("problem making socket\n");
	exit(1);
  }
  int one=1;
  setsockopt(serverfd,SOL_SOCKET,SO_REUSEPORT,&one,4);
  int res = bind(serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if(res) {
	perror("problem binding\n");
	exit(1);
  }

  listen(serverfd,10);


  struct epoll_event events[10];
  int epollfd = epoll_create(1);
  struct epoll_event e = {EPOLLIN,.data.fd= serverfd};
  epoll_ctl(epollfd,EPOLL_CTL_ADD,serverfd,&e);

  int event_count;
  while((event_count=epoll_wait(epollfd,events,10,-1))>0) {
    for(int i=0;i<event_count;i++) {
        if(events[i].data.fd==serverfd) {
            int clientfd = accept(serverfd,0,0);
            printf("Got incoming\n");

            struct epoll_event e = {EPOLLIN,.data.fd= clientfd};
            epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&e);
        }
        else {
            char buf[100];
            int got = read(events[i].data.fd,buf,100);
            printf("Got ");
            fflush(stdout);
            write(1,buf,got);
        }
    }
  }

/*  while(1) {
    printf("Waiting for client...");
    fflush(stdout);
    int client = accept(fd,0,0);
    printf("got one!\n");
    
    char buf[100];
    int got;
    while((got=read(client,buf,100))>0) {
        write(1,buf,got);    
    }
  }
    */
}
