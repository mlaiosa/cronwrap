CC=gcc
CFLAGS=-Os -Wall -Werror
#CFLAGS=-g -Wall -Werror

all: cronwrap tester
clean:
	rm -f cronwrap tester
test: all
	./run_tests.sh

cronwrap: main.c
	$(CC) $(CFLAGS) -o cronwrap main.c

tester: tester.c
	$(CC) $(CFLAGS) -o tester tester.c
