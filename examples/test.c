main() {
 	char *t[]={"echo","quux",0};
        execve("/usr/bin/echo",t,0);
	perror("echo failed\n");
}
