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
  int client = accept(fd,0,0);
  
  char buf[100];
  int got = read(client,buf,100);
  write(1,buf,got);
}
