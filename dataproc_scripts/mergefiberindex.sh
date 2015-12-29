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
echo $n

tail -n ${n1} ${f1}
tail -n ${n2} ${f2}

exit 0
