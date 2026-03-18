#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

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
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
    perror("error setting SO_REUSEADDR");
    exit(1);
  }

  int res = bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if(res) {
	perror("problem binding\n");
	exit(1);
  }

  listen(fd,10); 
  while (1) {
        int clientfd = accept(fd, NULL, NULL);
        if (clientfd == -1) {
            perror("accept");
            continue;
        }

        // fdopen wraps file descriptor in a FILE* for libc buffered I/O functions
        FILE* client = fdopen(clientfd,"r+");
        char *limerick[5] = {0};
        int stored_lines = 0;

        char *line = NULL;
        size_t len = 0;
        int nread = 0;
        while ((nread=getline(&line, &len, client)) != -1) {
            // Remove trailing newline for command matching
            if (nread > 0 && (line[nread - 2] == '\r')) line[nread - 2] = '\0';
            if (nread > 0 && (line[nread - 1] == '\n')) line[nread - 1] = '\0';

            if (strcmp(line, "PRESENT") == 0) {
                fprintf(client, "GO AHEAD\n");
                fflush(client);
                // Read exactly 5 lines for the limerick
                stored_lines = 0;
                for (int i = 0; i < 5; i++) {
                    nread=getline(&line, &len, client);
                    if (nread == -1)
                        goto cleanup_client;

                    // Store a copy (including the newline)
                    limerick[i] = strdup(line);
                    if (!limerick[i])
                        goto cleanup_client;
                    stored_lines++;
                }
            }
            else if (strcmp(line, "AWAIT") == 0) {
                if (stored_lines == 0) {
                    fprintf(client, "No limerick stored.\n");
                } else {
                    for (int i = 0; i < stored_lines; i++) {
                        fprintf(client, "%s", limerick[i]);
                    }
                }
            }
            else {
                fprintf(client, "Unknown command\n");
            }
        }

    cleanup_client:
        // Free stored limerick lines
        for (int i = 0; i < 5; i++) {
            if(limerick[i]) 
                free(limerick[i]);
            limerick[i] = NULL;
        }
        free(line);
        close(clientfd);
    }

    close(fd);
    return 0;
}
