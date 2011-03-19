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

static char *output_file = NULL;
static char *tempfile = NULL;
static void cleanup_tempfile(void) {
	if (tempfile)
		unlink(tempfile);
}

struct argument_description {
	const char *name; char **dest;
};

/* Returns the position of the first positional argument. */
static int parse_arguments(const struct argument_description *desc, int argc, char **argv) {
	int i;
	int arg;
	int parsed;
	for (arg = 1; arg < argc; ++arg) {
		/* Everything following -- is positional. */
		if (strcmp(argv[arg], "--") == 0)
			return arg+1;
		parsed = 0;
		/* Go through and see if its one of our arguments. */
		for (i = 0; !parsed && desc[i].name; ++i) {
			const int len = strlen(desc[i].name);
			if (strncmp(argv[arg], desc[i].name, len) == 0) {
				if (argv[arg][len] == 0) {
					++arg;
					if (arg < argc)
						*desc[i].dest = argv[arg];
					else {
						printf("%s requries an argument\n", desc[i].name);
						exit(EXIT_FAILURE);
					}
					parsed = 1;
				}
				else if (argv[arg][len] == '=') {
					*desc[i].dest = argv[arg] + len + 1;
					parsed = 1;
				}
			}
		}
		/* If this wasn't one of our options, then its the first positional argument. */
		if (!parsed)
			return arg;
	}
	return arg;
}

int main(int argc, char **argv) {
	pid_t child;
	int log;
	int first_program_arg;
	static const struct argument_description arg_desc[] = {
		{"--save-output", &output_file},
		{NULL}
	};
	first_program_arg = parse_arguments(arg_desc, argc, argv);
	if (first_program_arg >= argc) {
		printf("At least one argument required\n");
		return 1;
	}

	/* Create a temp file if we need one. */
	if (output_file == NULL) {
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
		output_file = tempfile;
	}
	else {
		/* TODO: There ought to be command line options to allow unlinking
		 * rather than truncating, and to allow appending. Also, how to choose
		 * a mode? Should I do 0666 and let umask handle it? */
		log = open(output_file, O_WRONLY|O_TRUNC|O_CREAT, 0666);
		if (log < 0) {
			printf("Unable to open %s\n", output_file);
			return 1;
		}
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
		execvp(argv[first_program_arg], argv+first_program_arg);
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
		log = open(output_file, O_RDONLY, 0);
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
