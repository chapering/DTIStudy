#!/bin/bash

RESID=""
BASE=`pwd`

if [ $# -ge 2 ];then
	RESID=$2
	if [ "$RESID" = "pos0" ];then
		RESID=""
	fi
fi

if [ $# -ge 1 ];then
	opt=$1
	if test ${opt:0:3} = "pos";then
		RESID=$1
		if [ "$RESID" = "pos0" ];then
			RESID=""
		fi
	else
		BASE=$1
	fi
fi

for fb in cst cg ilf
do
	for ((i=1;i<=5;i++))
	do
		for CUT in nocut leftcut rightcut
		do
			echo -ne "\t$fb\ts${i}\t${CUT}\t\t"
			src="${BASE}/s${i}/$fb/${RESID}/fiberidx_${CUT}_s${i}.data"
			diff=`head -n2 $src | tail -n1 | awk '{print $2}'`
			echo ${diff}
		done
	done
done
exit 0
