#!/bin/bash

# input
rm -f a.out *.JPG
src=$1;

echo $1
echo $2

if [[ "$src" =~ ".cpp" ]];
then
	g++ $src
	./a.out $2 &
elif [[ $src =~ ".c" ]];
then
	g++ $src 
	./a.out $2 &
elif [[ $src =~ ".py" ]];
then
	python3 $src $2 &
else
	echo "not supported" $src
fi

sleep 1

wget -O aaa http://localhost:$2/biga.html

killall a.out python3

diff biga.html aaa

# show the result
if [[ $? == 0 ]];
then
	echo "Success: " $src
else
	echo "Fail: " $src
fi


