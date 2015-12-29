#!/bin/bash

COPY="ln -s -f"

BASEDIR=`pwd`
if [ $# -ge 1 ];then
	BASEDIR=$1
fi

FBARRARY=(cst cg cc ifo ilf)
RELDIR_POS="../"

function mend_secondaryblock_softlinks()
{
	if [ $# -lt 1 ];then
		return 1
	fi

	echo " #################################### FOR TASK $taskid ############################### "

	taskid=$1
	regiondir=${BASEDIR}/region_for_task${taskid}

	for ((i=1;i<=5;i++))
	do
		for ((j=0;j<5;j++))
		do
			for p in pos1 pos2
			do
				if [ ! -d ${regiondir}/s${i}/${FBARRARY[$j]}/${p} ];then
					continue
				fi

				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/region_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/skeleton_region_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

############################################################################################################################################
############################################################################################################################################
for taskid in 1 2 3 4 5 6
do
	mend_secondaryblock_softlinks $taskid
done

echo "done!"
exit 0


