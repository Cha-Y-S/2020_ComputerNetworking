#!/bin/bash

# input
rm -f a.out *.JPG
src=$1;
p1=$2;
p2=$3;

echo $1
echo $2
echo $3

id1=hid01$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 5 | head -n 1)
id2=hid02$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 5 | head -n 1)
echo "" > $id1.sh
h1sleep=0
slt=$(($RANDOM%5+5))
let "h1sleep=h1sleep+$slt+1"
echo "sleep" $slt >> $id1.sh
cnt=$(($RANDOM%6+5)) 
echo $cnt
for ((i=1;i<=cnt;i++)); do
	strlen=$(($RANDOM%30+5)) 
	newstr=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $strlen | head -n 1 | sed 's/S/ /g' | sed 's/K/ /g')
	echo "echo" $newstr >> $id1.sh
	slt=$(($RANDOM%4))
	sltms=$slt.$(($RANDOM%1000))
	let "h1sleep=h1sleep+$slt+1"
	echo $h1sleep $slt $sltms
	echo "sleep" $sltms >> $id1.sh
	echo $newstr >> $id2.txt
done

echo "" > $id2.sh
h2sleep=0
slt=$(($RANDOM%3+1))
let "h2sleep=h2sleep+$slt+1"
echo "sleep" $slt.$(($RANDOM%1000)) >> $id2.sh
echo "echo @talk localhost $p1" >> $id2.sh
slt=$(($RANDOM%5+1)) 
let "h2sleep=h2sleep+$slt+1"
echo "sleep" $slt >> $id2.sh
for ((i=1;i<=cnt;i++)); do
	strlen=$(($RANDOM%30+5)) 
	newstr=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $strlen | head -n 1 | sed 's/S/ /g' | sed 's/K/ /g')
	echo "echo" $newstr >> $id2.sh
	slt=$(($RANDOM%4))
	sltms=$slt.$(($RANDOM%1000))
	let "h2sleep=h2sleep+$slt+1"
	echo $h2sleep $slt $sltms
	echo "sleep" $sltms >> $id2.sh
	echo $newstr >> $id1.txt
done
if [[ $h1sleep < $h2sleep ]];
then
	let "h1sleep=h2sleep-h1sleep+5"
	h2sleep=1
else
	let "h2sleep=h1sleep-h2sleep+2"
	h1sleep=5
fi

echo "sleep " $h2sleep >> $id2.sh
echo "echo @quit" >> $id2.sh

echo "sleep " $h1sleep >> $id1.sh
echo "echo @quit" >> $id1.sh


if [[ "$src" =~ ".cpp" ]];
then
	g++ -pthread $src
	bash $id1.sh | timeout 60 ./a.out $p1 $id1 > aa.txt &
	bash $id2.sh | timeout 60 ./a.out $p2 $id2 > bb.txt &
elif [[ $src =~ ".c" ]];
then
	gcc -pthread $src 
	bash $id1.sh | timeout 60 ./a.out $p1 $id1  > aa.txt&
	bash $id2.sh | timeout 60 ./a.out $p2 $id2  > bb.txt&
elif [[ $src =~ ".py" ]];
then
	bash $id1.sh | timeout 60 python3 $src $p1 $id1  > aa.txt&
	bash $id2.sh | timeout 60 python3 $src $p2 $id2 > bb.txt &
else
	echo "not supported" $src
fi

wait
sleep 1
echo "Programs ended"

succcnt=0
grep "$id2 : " aa.txt | sed "s/$id2 : //g" > aa1.txt
diff aa1.txt $id1.txt
if [[ $? == 0 ]];
then
	let 'succcnt=succcnt+1'
fi

grep "connection " aa.txt
if [[ $? == 0 ]];
then
	let 'succcnt=succcnt+2'
fi

grep "Connection Closed " aa.txt
if [[ $? == 0 ]];
then
	let 'succcnt=succcnt+4'
fi

grep "$id1 : " bb.txt | sed "s/$id1 : //g" > bb1.txt
diff bb1.txt $id2.txt
if [[ $? == 0 ]];
then
	let 'succcnt=succcnt+8'
fi

# show the result
echo "Result:" $succcnt $src

rm -f hid0*.sh hid0*.txt aa1.txt bb1.txt a.out


