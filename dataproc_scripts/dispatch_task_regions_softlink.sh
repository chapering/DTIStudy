#!/bin/bash

COPY="ln -s -f"

BASEDIR=`pwd`
if [ $# -ge 1 ];then
	BASEDIR=$1
fi

FBARRARY=(cst cg cc ifo ilf)
RELDIR="../../../regions/"
RELDIR_POS="../../../../regions/"

function dispather()
{
	if [ $# -lt 5 ];then
		return 1
	fi

	regionprefices=$1
	regionprefices_pos1=$2
	skeletonprefices=$3
	skeletonprefices_pos1=$4
	regiondir=$5

	for ((i=1;i<=5;i++))
	do
		for ((j=0;j<5;j++))
		do
			cd ${regiondir}/s${i}/${FBARRARY[$j]}/
			${COPY} ${RELDIR}/${regionprefices[$j]}_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/${skeletonprefices[$j]}_s${i}.data skeleton_region_s${i}.data
			cd -

			for p in pos1
			do
				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/${regionprefices_pos1[$j]}_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/${skeletonprefices_pos1[$j]}_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

function dispatch_region_for_task1()
{
	echo " #################################### FOR TASK 1 ############################### "
	regionprefices=(raw_righths sync1_fa_righths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)
	regionprefices_pos1=(raw_lefths sync1_fa_righths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)

	skeletonprefices=(skeletonraw_righths skeletonrighths normalSkeleton skeletonlefths skeletonrighths)
	skeletonprefices_pos1=(skeletonraw_lefths skeletonrighths normalSkeleton skeletonlefths skeletonrighths)

	regiondir=${BASEDIR}/region_for_task1

	for ((i=1;i<=5;i++))
	do
		for ((j=0;j<5;j++))
		do
			cd ${regiondir}/s${i}/${FBARRARY[$j]}/
			${COPY} ${RELDIR}/${regionprefices[$j]}_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/${skeletonprefices[$j]}_s${i}.data skeleton_region_s${i}.data
			cd -

			for p in pos1
			do
				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/${regionprefices_pos1[$j]}_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/${skeletonprefices_pos1[$j]}_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

function dispatch_region_for_task2()
{
	echo " #################################### FOR TASK 2 ############################### "
	regionprefices=(sync1_fa_lefths sync1_fa_lefths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)
	regionprefices_pos1=(sync1_fa_righths sync1_fa_righths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)

	skeletonprefices=(skeletonlefths skeletonlefths normalSkeleton skeletonlefths skeletonrighths)
	skeletonprefices_pos1=(skeletonrighths skeletonrighths normalSkeleton skeletonlefths skeletonrighths)

	regiondir=${BASEDIR}/region_for_task2

	for ((i=1;i<=5;i++))
	do
		for ((j=0;j<5;j++))
		do
			cd ${regiondir}/s${i}/${FBARRARY[$j]}/
			${COPY} ${RELDIR}/${regionprefices[$j]}_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/${skeletonprefices[$j]}_s${i}.data skeleton_region_s${i}.data
			cd -

			for p in pos1
			do
				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/${regionprefices_pos1[$j]}_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/${skeletonprefices_pos1[$j]}_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

function dispatch_region_for_task3()
{
	echo " #################################### FOR TASK 3 ############################### "
	regionprefices=(sync1_fa_righths sync1_fa_lefths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)
	regionprefices_pos1=(sync1_fa_lefths sync1_fa_righths sync1_fa_normal sync1_fa_lefths sync1_fa_lefths)

	skeletonprefices=(skeletonrighths skeletonlefths normalSkeleton skeletonlefths skeletonrighths)
	skeletonprefices_pos1=(skeletonlefths skeletonrighths normalSkeleton skeletonlefths skeletonlefths)

	regiondir=${BASEDIR}/region_for_task3

	for ((i=1;i<=5;i++))
	do
		for ((j=0;j<5;j++))
		do
			cd ${regiondir}/s${i}/${FBARRARY[$j]}/
			${COPY} ${RELDIR}/${regionprefices[$j]}_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/${skeletonprefices[$j]}_s${i}.data skeleton_region_s${i}.data
			cd -

			for p in pos1
			do
				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/${regionprefices_pos1[$j]}_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/${skeletonprefices_pos1[$j]}_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

function dispatch_region_for_task4()
{
	echo " #################################### FOR TASK 4 ############################### "
	regionprefices=(sync1_fa_lefths sync1_fa_righths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)
	regionprefices_pos1=(sync1_fa_righths sync1_fa_righths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)

	skeletonprefices=(skeletonlefths skeletonrighths normalSkeleton skeletonlefths skeletonrighths)
	skeletonprefices_pos1=(skeletonrighths skeletonrighths normalSkeleton skeletonlefths skeletonrighths)

	regiondir=${BASEDIR}/region_for_task4

	for ((i=1;i<=5;i++))
	do
		for ((j=0;j<5;j++))
		do
			cd ${regiondir}/s${i}/${FBARRARY[$j]}/
			${COPY} ${RELDIR}/${regionprefices[$j]}_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/${skeletonprefices[$j]}_s${i}.data skeleton_region_s${i}.data
			cd -

			for p in pos1
			do
				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/${regionprefices_pos1[$j]}_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/${skeletonprefices_pos1[$j]}_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

function dispatch_region_for_task5()
{
	echo " #################################### FOR TASK 5 ############################### "
	regionprefices=(sync1_fa_righths sync1_fa_righths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)
	regionprefices_pos1=(sync1_fa_lefths sync1_fa_lefths sync1_fa_normal sync1_fa_lefths sync1_fa_righths)

	skeletonprefices=(skeletonrighths skeletonrighths normalSkeleton skeletonlefths skeletonrighths)
	skeletonprefices_pos1=(skeletonlefths skeletonlefths normalSkeleton skeletonlefths skeletonrighths)

	regiondir=${BASEDIR}/region_for_task5

	for ((i=1;i<=5;i++))
	do
		for ((j=0;j<5;j++))
		do
			cd ${regiondir}/s${i}/${FBARRARY[$j]}/
			${COPY} ${RELDIR}/${regionprefices[$j]}_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/${skeletonprefices[$j]}_s${i}.data skeleton_region_s${i}.data
			cd -

			for p in pos1
			do
				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/${regionprefices_pos1[$j]}_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/${skeletonprefices_pos1[$j]}_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

function dispatch_region_for_task6()
{
	echo " #################################### FOR TASK 6 ############################### "
	regionprefices=(sync5_fa_normal sync1_fa_normal sync1_fa_normal sync1_fa_normal sync1_fa_normal)
	regionprefices_pos1=(sync5_fa_normal sync1_fa_normal sync1_fa_normal sync1_fa_normal sync1_fa_normal)
	skeletonprefices=(normalSkeleton normalSkeleton normalSkeleton normalSkeleton normalSkeleton)
	skeletonprefices_pos1=(normalSkeleton normalSkeleton normalSkeleton normalSkeleton normalSkeleton)

	regiondir=${BASEDIR}/region_for_task6

	for ((i=1;i<=5;i++))
	do
		for j in 0 1 4
		do
			cd ${regiondir}/s${i}/${FBARRARY[$j]}/
			${COPY} ${RELDIR}/${regionprefices[$j]}_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/${skeletonprefices[$j]}_s${i}.data skeleton_region_s${i}.data
			cd -

			for p in pos1
			do
				cd ${regiondir}/s${i}/${FBARRARY[$j]}/${p}
				${COPY} ${RELDIR_POS}/${regionprefices_pos1[$j]}_s${i}.data region_s${i}.data
				${COPY} ${RELDIR_POS}/${skeletonprefices_pos1[$j]}_s${i}.data skeleton_region_s${i}.data
				cd -
			done
		done
	done

	return 0
}

############################################################################################################################################
############################################################################################################################################
TASKSET="1 2 3 4 5 6"
if [ $# -ge 2 ];then
	TASKSET=$2
fi

for i in ${TASKSET}
do
	dispatch_region_for_task${i}
done

echo "done!"
exit 0


