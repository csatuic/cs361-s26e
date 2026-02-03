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
  serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serv_addr.sin_port = htons(8765);

  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd==-1) {
	perror("problem making socket\n");
	exit(1);
  }
  int res = connect(fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));

  if(res) {
	perror("problem connecting\n");
	exit(1);
  }

  write(fd,"Hello World!\n",13);
}
