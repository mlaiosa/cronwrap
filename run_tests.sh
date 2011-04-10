#!/bin/sh

NUM_PASS=0
NUM_FAIL=0

INCOMPLETE=""
ERRORS=""

for f in tests/*; do
	if [ -x $f ] ;then
		INCOMPLETE="-$INCOMPLETE"
	fi
done
INCOMPLETE="${INCOMPLETE}\r"
printf -- "$INCOMPLETE"

for f in tests/*; do
	if [ -x $f ] ;then
		printf \|
		if [ ! -e $f.out ] ;then
			NUM_FAIL="$NUM_FAIL + 1"
			ERRORS="$ERRORS\n$f.out missing."
		else
			if ($f 2>&1 | diff -q $f.out - > /dev/null) ;then
				NUM_PASS="$NUM_PASS + 1"
			else
				NUM_FAIL="$NUM_FAIL + 1"
				ERRORS="$ERRORS\n$f Failed."
			fi
		fi
	fi
done

printf "$ERRORS\n"
NUM_PASS=`expr $NUM_PASS`
NUM_FAIL=`expr $NUM_FAIL`
echo Passed: $NUM_PASS
echo Failed: $NUM_FAIL
exit $NUM_FAIL
