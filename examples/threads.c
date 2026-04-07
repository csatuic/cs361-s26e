#include<pthread.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

char global;

// must hold buflock when accessing buf
pthread_mutex_t buflock;
char buf[100];

void* handle_client(void* arg) {
    int fd = (int)arg;
    char localbuf[100];
    while(1) {
    int got = read(fd,localbuf,100);    
    pthread_mutex_lock(&buflock);
    strncpy(buf,localbuf,got);
    sleep(5);
    write(fd,buf,got);
    pthread_mutex_unlock(&buflock);
    }
    return 0;
}

int main(int argc, char** argv) {
    pthread_mutex_init(&buflock,0);
  struct sockaddr_in serv_addr = {0};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(8766);

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
    pthread_t thread;

    while(1) {  
      int client = accept(fd,0,0);
      pthread_create(&thread,0,handle_client,(void*)client);
    }
    
    // accept() 

    printf("hi from parent\n");
    void* retval;
    pthread_join(thread, &retval);
    printf("thread finished\n");    
//    sleep(1);
}