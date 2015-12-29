#!/bin/bash 
: '
this script, presuming that all executables and data have been readyed in the
configurable directories as follows, boost a run of the whole study of seeding
methods in DTI tractography.
--- this version is proprietarily written for formal seeding study.
'
BASEDIR=`pwd`

BINDIR=${BASEDIR}/bin
TASKDIRS="pretask task1 task2 task3 task4 task5"
EXECUTABLES="singleitr multiviewitr multiwindowitr"

DATADIR=${BASEDIR}/data
NORMALCASEDATADIR=${DATADIR}/normal
#ABNORMALCASEDATADIR=${DATADIR}/agenesis
TUMORDATADIR=${DATADIR}/tumor

LOG=${BASEDIR}/studylog

function checkbins()
{
	if [ ! -d ${BINDIR} ];then
		echo "FATAL: directory ${BINDIR} NOT found, please set up firstly."
		return 1
	fi

	for ts in ${TASKDIRS}
	do
		for bin in ${EXECUTABLES}
		do
			if [ ! -s ${BINDIR}/${ts}/${bin} ];then
				echo "ERROR: executable ${bin} for ${ts} NOT found,"
			    echo "please set up firstly."
				return 1
			fi
		done
	done
	return 0
}

function checkdata()
{
	for bdir in ${NORMALCASEDATADIR}
	#${ABNORMALCASEDATADIR}
	do
		if [ ! -d ${bdir} ];then
			echo "ERROR: data directory ${bdir} NOT found."
			return 1
		fi

		for ((n=1;n<=5;n++));do
			if [ `ls ${bdir}/normal_s${n}.data | wc -l` -le 0 ];then
				echo "ERROR: data of resolution ${n}x${n}x${n} NOT found in the"
				echo "directory ${bdir}"
				return 1
			fi
		done
	done

	if [ ! -s ${TUMORDATADIR}/potato.sm ];then
		echo "ERROR: tumor data NOT found in ${TUMORDATADIR}."
		return 1
	fi

	return 0
}

function taskflag()
{
	echo -e "\n##############################################################" >> $LOG
	echo "                           TASK $1                            " >> $LOG
	echo -e "##############################################################\n" >> $LOG
}

#-----------------------------------------#
#
# pretask
#
#-----------------------------------------#
function pretask()
{
	: '
	this is a trivial, actually virtual, task. It just show the introductory
	message of the tasks to follow
	'
	${BINDIR}/pretask/singleitr \
		-f ${NORMALCASEDATADIR}/*_s5.data \
		-t ${BINDIR}/pretask/tasktext \
		-V 1>> $LOG 2>&1
}

#-----------------------------------------#
#
# task 1
#
#-----------------------------------------#
function task1()
{
	taskflag 1
	setid=2
	if [ $# -ge 1 ];then
		setid=$1
	fi

	#for ((n=1;n<=5;n++))
	for n in 2 3 1 5 4
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n}  setid=${setid}#####" >> $LOG
		${BINDIR}/task1/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task1/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task1/set${setid}/FApt*normalcase_s${n}_?.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/region_skeleton.data \
		-t ${BINDIR}/task1/tasktext \
		-V 1>> $LOG 2>&1

		: '
		if [ $setid -lt 3 ];then
			((setid++))
		else
			setid=1
		fi
		'
	done
}

#-----------------------------------------#
#
# task 2 
#
#-----------------------------------------#
function task2()
{
	taskflag 2
	for n in 1 4
	#for ((n=1;n<=5;n++))
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task2/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task2/region*normal_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task2/fiberidx*normal_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task2/region_skeleton.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/err1/tumorbox_region_normal_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/correct/tumorbox_region_normal_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/err2/tumorbox_region_normal_s${n}.data \
		-k 2 \
		-t ${BINDIR}/task2/tasktext \
		-V 1>> $LOG 2>&1
	done

	for n in 5 3
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task2/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task2/region*normal_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task2/fiberidx*normal_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task2/region_skeleton.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/correct/tumorbox_region_normal_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/err1/tumorbox_region_normal_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/err2/tumorbox_region_normal_s${n}.data \
		-k 1 \
		-t ${BINDIR}/task2/tasktext \
		-V 1>> $LOG 2>&1
	done

	for n in 2
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task2/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task2/region*normal_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task2/fiberidx*normal_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task2/region_skeleton.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/err1/tumorbox_region_normal_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/err2/tumorbox_region_normal_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/correct/tumorbox_region_normal_s${n}.data \
		-k 3 \
		-t ${BINDIR}/task2/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 3 
#
#-----------------------------------------#
function task3()
{
	taskflag 3
	#for ((n=1;n<=5;n++))
	for n in 3 1 2 4 5
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		echo "#####    when the tumor touches fibers    #### ">> $LOG
		${BINDIR}/task3/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task3/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task3/touch/tumorbox_region*normalcase_s${n}_?.data \
		-u \
		${TUMORDATADIR}/potato.sm \
		-s \
		${NORMALCASEDATADIR}/region_for_task3/region_skeleton.data \
		-t ${BINDIR}/task3/tasktext \
		-V 1>> $LOG 2>&1

		echo -e "\n#####    when the tumor is tangential to fibers    ####" >> $LOG
		${BINDIR}/task3/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task3/region*normalcase_s${n}_?.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task3/non-touch/tumorbox_region*normalcase_s${n}_?.data \
		-u \
		${TUMORDATADIR}/potato.sm \
		-s \
		${NORMALCASEDATADIR}/region_for_task3/region_skeleton.data \
		-t ${BINDIR}/task3/tasktext \
		-V 1>> $LOG 2>&1
	done
}

#-----------------------------------------#
#
# task 4 
#
#-----------------------------------------#
function task4()
{
	taskflag 4
	for n in 5 2 4 1 3
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### seeding resolution ${n}x${n}x${n} #####" >> $LOG
		
		if [ $n -eq 0 ];then
			${BINDIR}/task4/singleitr \
			-f \
			${NORMALCASEDATADIR}/flipped/normal_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/flipped/skeletons/normalSkeleton_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/set_for_task4/tensorIdx_s${n}.data \
			-t ${BINDIR}/task4/tasktext \
			-V 1>> $LOG 2>&1
		else
			echo -e "\n##### with Normal case #####" >> $LOG
			echo "##### seeding resolution ${n}x${n}x${n} #####" >> $LOG
			${BINDIR}/task4/singleitr \
			-f \
			${NORMALCASEDATADIR}/normal_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/skeletons/normalSkeleton_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/set_for_task4/tensorIdx_s${n}.data \
			-t ${BINDIR}/task4/tasktext \
			-V 1>> $LOG 2>&1
		fi
	done
}

#-----------------------------------------#
#
# task 5 
#
#-----------------------------------------#
function task5()
{
	taskflag 5
	#for ((n=1;n<=5;n++))
	for n in 4 5 3 2 1
	do
		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### seeding resolution ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task5/singleitr \
		-f \
		${NORMALCASEDATADIR}/flipped/normal_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/flipped/skeletons/normalSkeleton_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/fiberidx_for_task5/fiberidx_s${n}.data \
		-t ${BINDIR}/task5/tasktext \
		-V 1>> $LOG 2>&1
	done
}

function costOfThisTask()
{
	s=$1
	e=$2
	((d=e-s))
	((h=d/3600))
	((m=d%3600/60))
	((s=d%3600%60))
	echo " Time cost of this task: $h hours $m minutes $s seconds." >> $LOG
}

#####################################################################################
###    Task ordering and main flow control
###
#####################################################################################

checkbins
if [ $? -ne 0 ];then
	exit 1
fi

checkdata
if [ $? -ne 0 ];then
	exit 1
fi

>> $LOG

bstart=`date +%s`

curstart=`date +%s`
pretask
curend=`date +%s`
costOfThisTask ${curstart} ${curend}

curstart=`date +%s`
task1
curend=`date +%s`
costOfThisTask ${curstart} ${curend}

curstart=`date +%s`
task2
curend=`date +%s`
costOfThisTask ${curstart} ${curend}

curstart=`date +%s`
task3
curend=`date +%s`
costOfThisTask ${curstart} ${curend}

curstart=`date +%s`
task4
curend=`date +%s`
costOfThisTask ${curstart} ${curend}

curstart=`date +%s`
task5
curend=`date +%s`
costOfThisTask ${curstart} ${curend}

end=`date +%s`
echo -e "\n################## ALL FINISHED #######################" >> $LOG
((d=end-bstart))
((h=d/3600))
((m=d%3600/60))
((s=d%3600%60))
echo " Time cost: $h hours $m minutes $s seconds." >> $LOG

echo "All finished, thank you!"
echo

exit 0

