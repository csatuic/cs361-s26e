#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>

int main(int argc, char** argv) {

  struct sockaddr_un serv_addr = {AF_UNIX, "/tmp/socket_name.unix"};

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
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
