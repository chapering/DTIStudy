#!/bin/bash

BASEDIR=`pwd`
if [ $# -ge 1 ];then
	BASEDIR=$1
else
	echo -n "will create task data hierarchy from current directory (y/n)? "
	read c
	c=${c:0:1}
	if [ -z "$c" ];then
		c=""
	fi
	if [ "$c" != "y" -a "$c" != "Y" ];then
		exit 1
	fi
fi

FBARRARY=(cst cg cc ifo ilf)

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
			for p in pos1 
			do
				mkdir -p ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
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

