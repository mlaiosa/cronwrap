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

int main(int argc, char **argv) {
	pid_t child;
	int log;
	/* Make a NULL-terminated copy of argv.  I suspect that argv will itself
	 * always be NULL-terminated, but I can't find any reference proving that.
	 */
	char **args = malloc(sizeof *args * argc);
	if (!args) {
		puts("Out of memory");
		return 1;
	}
	memcpy(args, argv+1, sizeof *args * argc-1);
	args[argc-1] = NULL;

	log = mkstemp(template);

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
		return EXIT_FAILURE;
	}
	else {
		/*We are the parent*/
		char buf[1024];
		int status;
		size_t bytes_read;
		/* Close the FDs we don't need. */
		close(0);
		close(log);
		while (waitpid(child, &status, 0) != child)
			;
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
			exit(EXIT_SUCCESS);
			unlink(template);
		}
		log = open(template, O_RDONLY, 0);
		while ((bytes_read = read(log, buf, sizeof buf)) > 0) {
			write(1, buf, bytes_read);
		}
		close(log);
		unlink(template);
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
		else
			exit(EXIT_FAILURE);
	}

	return 0;
}
