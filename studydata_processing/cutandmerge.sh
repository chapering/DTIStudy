#!/bin/bash

for fb in cst cg ilf
do
	for ((i=1;i<=5;i++))
	do
		echo "------------- $fb s${i} ----------------"
		src1="s${i}/$fb/fiberidx_left_s${i}.data"
		src2="s${i}/$fb/fiberidx_right_s${i}.data"
		nfb1=`head -n2 $src1 | tail -n1`
		nfb2=`head -n2 $src2 | tail -n1`

		/usr/bin/mergefiberindex.sh $src1 $src2 > s${i}/$fb/fiberidx_nocut_s${i}.data

		/usr/bin/cutpercent.py $src1 0.2 1> tmp1
		/usr/bin/cutpercent.py $src2 0.2 1> tmp2
		/usr/bin/mergefiberindex.sh $src1 tmp2 > s${i}/$fb/fiberidx_rightcut_s${i}.data
		/usr/bin/mergefiberindex.sh $src2 tmp1 > s${i}/$fb/fiberidx_leftcut_s${i}.data

		mkdir -p s${i}/$fb/pos1
		mkdir -p s${i}/$fb/pos2

		/usr/bin/cutpercent.py $src1 0.3 1> tmp1
		/usr/bin/cutpercent.py $src2 0.3 1> tmp2
		/usr/bin/mergefiberindex.sh $src1 tmp2 > s${i}/$fb/pos1/fiberidx_rightcut_s${i}.data
		/usr/bin/mergefiberindex.sh $src2 tmp1 > s${i}/$fb/pos1/fiberidx_leftcut_s${i}.data

		/usr/bin/cutpercent.py $src1 0.4 1> tmp1
		/usr/bin/cutpercent.py $src2 0.4 1> tmp2
		/usr/bin/mergefiberindex.sh $src1 tmp2 > s${i}/$fb/pos2/fiberidx_rightcut_s${i}.data
		/usr/bin/mergefiberindex.sh $src2 tmp1 > s${i}/$fb/pos2/fiberidx_leftcut_s${i}.data
		echo "done."
	done
done
echo "All done."
exit 0
