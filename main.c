/*
Copyright 2011 Mike Laiosa <mike@laiosa.org>.  Licensed under the GPLv2.
*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char template[] = "/tmp/cronwrap.XXXXXX";
void cleanup(void) {
	unlink(template);
	template[0] = 0;
}

int main(int argc, char **argv) {
	pid_t child;
	int log;
	char **args = malloc(sizeof *args * argc-1);
	if (!args) {
		puts("Out of memory");
		return 1;
	}
	memcpy(args, argv+2, sizeof *args * argc-2);
	args[argc-2] = NULL;

	/* No need for stdin */
	close(0);

	log = mkstemp(template);
	atexit(cleanup);
	child = fork();
	if (child == 0) {
		/*We are the child.*/
		close(1);
		close(2);
		dup2(log, 1);
		dup2(log, 2);
		close(log);
		execvp(argv[1], args);
		printf("Unable to launch %s\n", argv[1]);
		return 1;
	}
	else {
		/*We are the parent*/
		char buf[1024];
		int status;
		size_t bytes_read;
		close(log);
		waitpid(child, &status, 0);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
			exit(0);
		log = open(template, O_RDONLY, 0);
		while ((bytes_read = read(log, buf, sizeof buf)) > 0) {
			write(1, buf, bytes_read);
		}
		close(log);
		exit(status);
	}
	
	return 0;
}
