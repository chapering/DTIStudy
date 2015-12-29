#!/bin/bash

curdir=`pwd`
srcdir=${1:-$curdir}
matlabprg=${2:-"./stlineMedian.m"}
destfile=${3:-"stmedian.data"}

> ${destfile}
lncnt=0
for fn in `ls $srcdir/bundle*`
do
	if [ ! -s $fn ];
	then
		echo "File $fn not found, skipped."
		continue
	fi
	matlabLauncher.py  ${matlabprg}  ${fn} 100 mytmp
	cat mytmp >> ${destfile}
	((lncnt++))
done

if [ $lncnt -ge 1 ];then
	echo $lncnt > mytmp
	cat ${destfile} >> mytmp

	mv mytmp ${destfile}
fi

echo "done!"
exit 0
