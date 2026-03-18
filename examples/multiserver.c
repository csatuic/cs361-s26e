#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>

int main(int argc, char** argv) {

  struct sockaddr_in serv_addr = {0};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(8765);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  int flag=1;
  setsockopt(fd,SOL_SOCKET,SO_REUSEPORT,&flag,4);

  if(fd==-1) {
	perror("problem making socket\n");
	exit(1);
  }
  int res = bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if(res) {
	perror("problem binding\n");
	exit(1);
  }

  listen(fd,10);

  while(1) {
    printf("Waiting for client...");
//    fflush(stdout);
    int client = accept(fd,0,0);
    printf("got one!\n");
    
    char buf[100];
    int got;
    while((got=read(client,buf,100))>0) {
        write(1,buf,got);    
    }
  }
}
