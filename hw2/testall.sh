#!/bin/bash
rm -f a.out *.JPG
port=30000
for src in $* ;do
	echo $src
	bash labtest.sh $src $port
	let "port=port+1"
	sleep 1
done
