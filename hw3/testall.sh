#!/bin/bash
rm -f a.out *.JPG
p1=30000
p2=40000
for src in $* ;do
	echo $src
	bash labtest.sh $src $p1 $p2
	let "p1=p1+1"
	let "p2=p2+1"
	sleep 1
done
