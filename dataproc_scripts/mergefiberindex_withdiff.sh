#!/bin/bash

if [ $# -lt 2 ];then
	echo "usage: $0 <src_1> <src_2>"
	exit 1
fi

f1=$1
f2=$2

n1=`head -n2 ${f1} | tail -n1`
n2=`head -n2 ${f2} | tail -n1`

let "n = n1 + n2"

head -n1 ${f1}

if [ $n1 -eq $n2 ];then
	diff="0.00"
elif [ $n1 -gt $n2 ];then
	diff=`echo /dev/null | awk '{printf("%4.4f", ('$n1' - '$n2')*1.0/'$n2' )}'`
else
	diff=`echo /dev/null | awk '{printf("%4.4f", ('$n2' - '$n1')*1.0/'$n1' )}'`
fi
echo $n $diff

tail -n ${n1} ${f1}
tail -n ${n2} ${f2}

exit 0
