#!/bin/bash

RANDOM=$$
for fb in cst cg ilf
do
	for ((i=1;i<=5;i++))
	do
		echo "------------- $fb s${i} ----------------"
		src1="s${i}/$fb/fiberidx_left_s${i}.data"
		src2="s${i}/$fb/fiberidx_right_s${i}.data"
		nfb1=`head -n2 $src1 | tail -n1`
		nfb2=`head -n2 $src2 | tail -n1`

		for RESID in "" "pos1"
		do
			mkdir -p s${i}/$fb/${RESID}
			mkdir -p s${i}/$fb/${RESID}
			/usr/bin/mergefiberindex_withdiff.sh $src1 $src2 > s${i}/$fb/${RESID}/fiberidx_nocut_s${i}.data
			echo "in nocut:" `head -n2 $src1 | tail -n1` `head -n2 $src2 | tail -n1`

			lcutp="0.2"
			rcutp="0.3"

			tp=`expr $RANDOM % 401`
			let "tp += 200"
			if [ $nfb1 -lt $nfb2 ];then
				: '
				make sure that the right is $p percent larger then the left
				'
				tright=`echo /dev/null | awk '{printf("%d", '$nfb1'* (1.0 + '$tp'/1000.0) )}'`
				if [ $tright -le $nfb2 ];then
					rcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb2' - '$tright') * 1.0 / '$nfb2')}'`
					/usr/bin/cutpercent.py $src2 ${rcutp} 1> tmp2
					/usr/bin/mergefiberindex_withdiff.sh $src1 tmp2 > s${i}/$fb/${RESID}/fiberidx_leftcut_s${i}.data
					echo "cut right with percentage of $rcutp"
					echo "in leftcut:" `head -n2 $src1 | tail -n1` `head -n2 tmp2 | tail -n1`
				else
					lcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb1' - '$nfb2'*1.0/(1.0+'$tp'/1000.0))*1.0/'$nfb1' )}'`
					/usr/bin/cutpercent.py $src1 ${lcutp} 1> tmp1
					/usr/bin/mergefiberindex_withdiff.sh $src2 tmp1 > s${i}/$fb/${RESID}/fiberidx_leftcut_s${i}.data
					echo "cut left with percentage of $lcutp"
					echo "in leftcut:" `head -n2 tmp1 | tail -n1` `head -n2 $src2 | tail -n1`
				fi

				: '
				make sure that the left is $p percent larger then the right
				'
				tp=`expr $RANDOM % 401`
				let "tp += 200"
				rcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb2' - '$nfb1'*1.0/(1.0+'$tp'/1000.0))*1.0/'$nfb2' )}'`
				/usr/bin/cutpercent.py $src2 ${rcutp} 1> tmp2
				/usr/bin/mergefiberindex_withdiff.sh $src1 tmp2 > s${i}/$fb/${RESID}/fiberidx_rightcut_s${i}.data
				echo "cut right with percentage of $rcutp"
				echo "in rightcut:" `head -n2 $src1  | tail -n1` `head -n2 tmp2 | tail -n1`
			elif [ $nfb1 -gt $nfb2 ];then
				: '
				make sure that the left is $p percent larger then the right
				'
				tleft=`echo /dev/null | awk '{printf("%d", '$nfb2'* (1.0 + '$tp'/1000.0) )}'`
				if [ $tleft -le $nfb1 ];then
					lcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb1' - '$tleft') * 1.0 / '$nfb1')}'`
					/usr/bin/cutpercent.py $src1 ${lcutp} 1> tmp1
					/usr/bin/mergefiberindex_withdiff.sh $src2 tmp1 > s${i}/$fb/${RESID}/fiberidx_rightcut_s${i}.data
					echo "cut left with percentage of $lcutp"
					echo "in rightcut:" `head -n2 tmp1 | tail -n1` `head -n2 $src2 | tail -n1`
				else
					rcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb2' - '$nfb1'*1.0/(1.0+'$tp'/1000.0))*1.0/'$nfb2' )}'`
					/usr/bin/cutpercent.py $src2 ${rcutp} 1> tmp2
					/usr/bin/mergefiberindex_withdiff.sh $src1 tmp2 > s${i}/$fb/${RESID}/fiberidx_rightcut_s${i}.data
					echo "cut right with percentage of $rcutp"
					echo "in rightcut:" `head -n2 $src1 | tail -n1` `head -n2 tmp2 | tail -n1`
				fi

				: '
				make sure that the right is $p percent larger then the left
				'
				tp=`expr $RANDOM % 401`
				let "tp += 200"
				lcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb1' - '$nfb2'*1.0/(1.0+'$tp'/1000.0))*1.0/'$nfb1' )}'`
				/usr/bin/cutpercent.py $src1 ${lcutp} 1> tmp1
				/usr/bin/mergefiberindex_withdiff.sh $src2 tmp1 > s${i}/$fb/${RESID}/fiberidx_leftcut_s${i}.data
				echo "cut left with percentage of $lcutp"
				echo "in leftcut:" `head -n2 tmp1 | tail -n1` `head -n2 $src2 | tail -n1`
			else
				lcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb1' - '$nfb2'*1.0/(1.0+'$tp'/1000.0))*1.0/'$nfb1' )}'`
				/usr/bin/cutpercent.py $src1 ${lcutp} 1> tmp1
				/usr/bin/mergefiberindex_withdiff.sh $src2 tmp1 > s${i}/$fb/${RESID}/fiberidx_leftcut_s${i}.data
				echo "cut left with percentage of $lcutp"
				echo "in leftcut:" `head -n2 tmp1 | tail -n1` `head -n2 $src2 | tail -n1`

				tp=`expr $RANDOM % 401`
				let "tp += 200"
				rcutp=`echo /dev/null | awk '{printf("%2.2f", ('$nfb2' - '$nfb1'*1.0/(1.0+'$tp'/1000.0))*1.0/'$nfb2' )}'`
				/usr/bin/cutpercent.py $src2 ${rcutp} 1> tmp2
				/usr/bin/mergefiberindex_withdiff.sh $src1 tmp2 > s${i}/$fb/${RESID}/fiberidx_rightcut_s${i}.data
				echo "cut right with percentage of $rcutp"
				echo "in rightcut:" `head -n2 $src1 | tail -n1` `head -n2 tmp2 | tail -n1`
			fi
		done
		echo "res $i done."
	done
	echo "bundle $fb done."
done
echo "All done."
rm -f tmp1 tmp2
exit 0

