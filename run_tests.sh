#!/bin/sh

NUM_PASS=0
NUM_FAIL=0

for f in tests/*; do
	if [ -x $f ] ;then
		if [ ! -e $f.out ] ;then
			NUM_FAIL=`expr $NUM_FAIL + 1`
			echo $f.out missing.
		else
			if ($f 2>&1 | diff -q $f.out - > /dev/null) ;then
				NUM_PASS=`expr $NUM_PASS + 1`
			else
				NUM_FAIL=`expr $NUM_FAIL + 1`
				echo $f Failed.
			fi
		fi
	fi
done
echo Passed: $NUM_PASS
echo Failed: $NUM_FAIL
exit $NUM_FAIL
