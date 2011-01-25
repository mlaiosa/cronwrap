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

#if defined (__GNUC__)
#define UNUSED_VAR __attribute__((unused))
#else
#define UNUSED_VAR
#endif

static char *tempfile = NULL;
static void cleanup_tempfile(void) {
	if (tempfile)
		unlink(tempfile);
}

int main(int argc, char **argv) {
	pid_t child;
	int log;

	/* Create our temp file */
	{
		const char *template = "/cronwrap.XXXXXX";
		const char *tmpdir = getenv("TMPDIR");
		if (tmpdir == NULL) tmpdir = "/tmp";
		tempfile = malloc(strlen(tmpdir) + strlen(template) + 1);
		if (!tempfile) {
			puts("Out of memory");
			return 1;
		}
		strcpy(tempfile, tmpdir);
		strcat(tempfile, template);

		log = mkstemp(tempfile);
		if (log == -1) {
			printf("Unable to create the temporary file %s\n", tempfile);
			tempfile = NULL;
			return 1;
		}
		atexit(cleanup_tempfile);
	}

	child = fork();
	if (child == -1) {
		puts("Unable to fork");
		return 1;
	}
	else if (child == 0) {
		/*We are the child.*/
		tempfile = NULL;
		close(1);
		close(2);
		dup2(log, 1);
		dup2(log, 2);
		close(log);
		execvp(argv[1], argv+1);
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
		}
		log = open(tempfile, O_RDONLY, 0);
		while ((bytes_read = read(log, buf, sizeof buf)) > 0) {
			/* There's nothing we can really do if the write fails,
			   but simply casting the result to void doesn't seem
			   sufficient to overcome GCC's warning. */
			UNUSED_VAR int result = write(1, buf, bytes_read);
		}
		close(log);
		if (WIFEXITED(status))
			exit(WEXITSTATUS(status));
		else
			exit(EXIT_FAILURE);
	}

	return 0;
}
