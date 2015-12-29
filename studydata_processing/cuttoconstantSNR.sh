#!/bin/bash

BASEDIR=${1:-`pwd`}
RESSET=${2:-"pos0 pos1"}

: ' the target SNR '
PCNT="0.5"

for fb in cst cg cc ifo ilf
do
	for ((i=1;i<=5;i++))
	do
		for RESID in ${RESSET}
		do
			if [ "$RESID" == "pos0" ];then
				RESID=""
			fi

			if [ ! -d ${BASEDIR}/s${i}/$fb/${RESID}/no/ ];then
				continue
			fi

			echo "------------- $fb s${i} ${RESID} ----------------"
			
			src1="${BASEDIR}/s${i}/$fb/${RESID}/no/fiberidx_member_region_s${i}.data"
			src2="${BASEDIR}/s${i}/$fb/${RESID}/no/fiberidx_alien_region_s${i}.data"
			nfb1=`head -n2 $src1 | tail -n1`
			nfb2=`head -n2 $src2 | tail -n1`
			#echo "original: $nfb1 $nfb2"

			vnfb1=`echo /dev/null | awk '{printf("%d", '$nfb2'*1.0 / '$PCNT')}'`
			vnfb2=`echo /dev/null | awk '{printf("%d", '$nfb1'*1.0 * '$PCNT')}'`
			#echo "desired: $vnfb1 $vnfb2"

			if [ $vnfb1 -le $nfb1 ];then
				cutp1=`echo /dev/null | awk '{printf("%.4f", ('$nfb1' - '$vnfb1')*1.0 / '$nfb1')}'`
				/usr/bin/cutpercent.py $src1 ${cutp1} 1> tmp1
				/usr/bin/mergefiberindex.sh $src2 tmp1 > ${BASEDIR}/s${i}/$fb/${RESID}/no/fiberidx_compound_region_s${i}.data
				echo "after cutting member group by $cutp1:" `head -n2 tmp1 | tail -n1` `head -n2 $src2 | tail -n1`

			elif [ $vnfb2 -le $nfb2 ];then
				cutp2=`echo /dev/null | awk '{printf("%.4f", ('$nfb2' - '$vnfb2')*1.0 / '$nfb2')}'`
				/usr/bin/cutpercent.py $src2 ${cutp2} 1> tmp2
				/usr/bin/mergefiberindex.sh $src1 tmp2 > ${BASEDIR}/s${i}/$fb/${RESID}/no/fiberidx_compound_region_s${i}.data
				echo "after cutting alien group by $cutp2:" `head -n2 $src1 | tail -n1` `head -n2 tmp2 | tail -n1`

			else
				echo "FATAL: don't know how to cut, failed."

			fi
		done
	done
done
echo "All done."
rm -f tmp1 tmp2
exit 0

