#!/bin/sh


for f in tests/*; do
	if [ -x $f ] ;then
		if [ ! -e $f.out ] ;then
			echo $f.out missing.
		else
			($f | diff -q $f.out - > /dev/null) || echo $f failed.
		fi
	fi
done
