#!/bin/bash

if [ $# -lt 2 ];then
	echo "usage: $0 <src_1> <src_2>"
	exit 1
fi

f1=$1
f2=$2

n1=`wc -l ${f1} | awk '{print $1}'`
n2=`wc -l ${f2} | awk '{print $1}'`

let "n1-=1"
let "n2-=1"

n12=`head -n1 ${f1} | tail -n1`
n22=`head -n1 ${f2} | tail -n1`
let "n = n12 + n22"

echo $n

tail -n ${n1} ${f1}
tail -n ${n2} ${f2}

exit 0
