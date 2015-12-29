#!/bin/bash

BASE=${1:-`pwd`}
RESID=""

CUTRATE="0.8"
if [ "$RESID" = "pos1" ];then
	CUTRATE="0.7"
fi

for fb in cst cg ilf
do
	for ((i=1;i<=5;i++))
	do
		src1="${BASE}/s${i}/$fb/fiberidx_left_s${i}.data"
		src2="${BASE}/s${i}/$fb/fiberidx_right_s${i}.data"
		nfb1=`head -n2 $src1 | tail -n1`
		nfb2=`head -n2 $src2 | tail -n1`

		for CUT in nocut leftcut rightcut
		do
			lnfb=$nfb1
			rnfb=$nfb2

			if [ "$CUT" = "leftcut" ];then
				lnfb=`echo /dev/null | awk '{printf("%d", '$lnfb'*'$CUTRATE')}'`
			elif [ "$CUT" = "rightcut" ];then
				rnfb=`echo /dev/null | awk '{printf("%d", '$rnfb'*'$CUTRATE')}'`
			fi

			echo -ne "\t$fb\ts${i}\t${CUT}\t"
			src="${BASE}/s${i}/$fb/${RESID}/fiberidx_${CUT}_s${i}.data"
			diff=`head -n2 $src | tail -n1 | awk '{print $2}'`
			echo -e "${diff}\t${lnfb}\t${rnfb}"
		done
	done
done
exit 0

