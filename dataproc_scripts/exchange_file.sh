#!/bin/bash

if [ $# -ne 2 ];then
	echo "Usage: $0 <src_1> <src_2>"
	exit 1
fi

mv $1 tmp
mv $2 $1
mv tmp $2

exit 0
