#include <stdio.h>
#include <stdlib.h>

/* Prints all the arguments, and returns success if there's an even number of
 * them. */

int main(int argc, char **argv) {
	int i;
	for (i = 1; i < argc; ++i) {
		puts(argv[i]);
	}
	/* exit success or failure depending on the number of arguments */
	exit((argc-1) % 2);
}
