#!/bin/bash

echo -e "bundle\tres\tleft\tright\tdiff_percent"
echo "--------------------------------------------"
for fb in cst cg ilf
do
	for ((i=1;i<=5;i++))
	do
		src1="s${i}/$fb/fiberidx_left_s${i}.data"
		src2="s${i}/$fb/fiberidx_right_s${i}.data"
		nfb1=`head -n2 $src1 | tail -n1`
		nfb2=`head -n2 $src2 | tail -n1`

		if [ $nfb1 -eq $nfb2 ];then
			diff="0.00%"
		elif [ $nfb1 -gt $nfb2 ];then
			diff=`echo /dev/null | awk '{printf("%4.2f%%", ('$nfb1' - '$nfb2')*1.0/'$nfb2' *100)}'`
		else
			diff=`echo /dev/null | awk '{printf("%4.2f%%", ('$nfb2' - '$nfb1')*1.0/'$nfb1' *100)}'`
		fi

		echo -e "$fb\ts${i}\t$nfb1\t$nfb2\t$diff"
		echo "--------------------------------------------"
	done
	echo "--------------------------------------------"
done
