#!/bin/bash 
BASEDIR=`pwd`

BINDIR=${BASEDIR}/bin
TASKDIRS="task1 task2 task3 task4 task5 task6"
EXECUTABLES="singleitr multiviewitr multiwindowitr"

DATADIR=${BASEDIR}/data
NORMALCASEDATADIR=${DATADIR}/normal_partial
BUNDLES="cst cg cc ifo ilf"

FBARRARY=(cst cg cc ifo ilf)
SUBSET="0 1 2 3 4"
CURFB=""

SUBRES="1 2 3 4 5"

TASKPROGIDFILE=${BASEDIR}/taskprogidfile
TASKNO=0

TUBERADIUS="0.2"

IMAGEROOT=${BASEDIR}/allsnapshots/

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

		: ' whole brain data not used anymore
		for ((n=1;n<=5;n++));do
			if [ `ls ${bdir}/normal_s${n}.data | wc -l` -le 0 ];then
				echo "ERROR: data of resolution ${n}x${n}x${n} NOT found in the"
				echo "directory ${bdir}"
				return 1
			fi
		done
		'

		# check task-specific data
		for ts in ${TASKDIRS}
		do
			if [ $ts = "pretask" ];then
				continue
			fi

			taskdatadir="region_for_${ts}"
			for ((n=1;n<=5;n++));do
				for fb in ${BUNDLES}
				do
					# cc and ifo not used in task6
					if [ $ts = "task6" ];then
						if [ $fb = "cc" -o $fb = "ifo" ];then
							continue
						fi
					fi

					if [ `ls -R ${bdir}/${taskdatadir}/s${n}/${fb}/ | grep data | wc -l` -lt 2 ];then
						echo "ERROR: data missed for ${ts}/s${n}/${fb}."
						return 1
					fi
				done
			done
		done
	done

	return 0
}

function updatetaskprog()
{
	let "TASKNO += 1"
	echo -e "\n\n       $TASKNO / 200" > ${TASKPROGIDFILE}
}

#-----------------------------------------#
#
# task 1
#
#-----------------------------------------#
function task1()
{
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		flip=0
		if [ "$CURFB" == "cst" ];then
			flip=1
		fi
		for n in ${SUBRES}
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog
			${BINDIR}/task1/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/${RESID}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/${RESID}/skeleton_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/${RESID}/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/${RESID}/tumorbox_1_region_s${n}.data \
			-t ${BINDIR}/task1/tasktext \
			-r ${TUBERADIUS} \
			-p ${TASKPROGIDFILE} \
			-i ${flip}
			if [ $? -ne 0 ];then
				return 3
			fi

			mkdir -p ${IMAGEROOT}/res${n}/
			convert -resize 300x300 /tmp/cur.png ${IMAGEROOT}/res${n}/task1_${CURFB}_${pos}.png
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 2 
#
#-----------------------------------------#
function task2()
{
	local keys1=(2 2 3 3 2)
	local keys2=(1 2 2 1 2)
	local keys3=(3 2 1 3 1)
	local keys4=(3 2 3 2 3)
	local keys5=(2 2 3 2 3)
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		for n in ${SUBRES}
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog
			CURKEY=$(eval echo \${keys$((di+1))[$((n-1))]})

			${BINDIR}/task2/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/${RESID}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/${RESID}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/${RESID}/fiberidx_*_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/${RESID}/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/${RESID}/tumorbox_1_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/${RESID}/tumorbox_2_region_s${n}.data \
			-t ${BINDIR}/task2/tasktext \
			-r ${TUBERADIUS} \
			-p ${TASKPROGIDFILE} \
			-k ${CURKEY}
			#-k ${keys[$((n-1))]} 
			if [ $? -ne 0 ];then
				return 3
			fi

			mkdir -p ${IMAGEROOT}/res${n}/
			convert -resize 300x300 /tmp/cur.png ${IMAGEROOT}/res${n}/task2_${CURFB}_${pos}.png
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 3 
#
#-----------------------------------------#
function task3()
{
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		for n in ${SUBRES}
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog
			${BINDIR}/task3/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/${RESID}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/${RESID}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/${RESID}/fiberidx_*_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-t ${BINDIR}/task3/tasktext 
			if [ $? -ne 0 ];then
				return 3
			fi

			mkdir -p ${IMAGEROOT}/res${n}/
			convert -resize 300x300 /tmp/cur.png ${IMAGEROOT}/res${n}/task3_${CURFB}_${pos}.png
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 4 
#
#-----------------------------------------#
function task4()
{
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		for n in ${SUBRES}
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog

			${BINDIR}/task4/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/${RESID}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/${RESID}/skeleton_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/${RESID}/tumorbox_0_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-t ${BINDIR}/task4/tasktext 
			if [ $? -ne 0 ];then
				return 3
			fi

			mkdir -p ${IMAGEROOT}/res${n}/
			convert -resize 300x300 /tmp/cur.png ${IMAGEROOT}/res${n}/task4_${CURFB}_${pos}.png
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 5 
#
#-----------------------------------------#
function task5()
{
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		for n in ${SUBRES}
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			echo "#####    when the answer is yes    #### "

			updatetaskprog
			${BINDIR}/task5/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/${RESID}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/${RESID}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/${RESID}/yes/fiberidx_*_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-t ${BINDIR}/task5/tasktext 
			if [ $? -ne 0 ];then
				return 3
			fi

			mkdir -p ${IMAGEROOT}/res${n}/
			convert -resize 300x300 /tmp/cur.png ${IMAGEROOT}/res${n}/task5_${CURFB}_${pos}_yes.png
			
			echo -e "\n#####    when the answer is no ####" 

			updatetaskprog
			${BINDIR}/task5/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/${RESID}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/${RESID}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/${RESID}/no/fiberidx_compound_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-r ${TUBERADIUS} \
			-t ${BINDIR}/task5/tasktext 
			if [ $? -ne 0 ];then
				return 3
			fi

			mkdir -p ${IMAGEROOT}/res${n}/
			convert -resize 300x300 /tmp/cur.png ${IMAGEROOT}/res${n}/task5_${CURFB}_${pos}_no.png
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 6
#
#-----------------------------------------#
function task6()
{
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		# cc and ifo not used in task6
		if [ $CURFB = "cc" -o $CURFB = "ifo" ];then
			continue
		fi

		for n in ${SUBRES}
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			for CUT in nocut leftcut rightcut
			do
				echo "##### data class: $CUT ####"
				updatetaskprog

				${BINDIR}/task6/singleitr \
				-f \
				${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/${RESID}/region_s${n}.data \
				-s \
				${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/${RESID}/skeleton_region_s${n}.data \
				-i \
				${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/${RESID}/fiberidx_${CUT}_s${n}.data \
				-p ${TASKPROGIDFILE} \
				-r ${TUBERADIUS} \
				-t ${BINDIR}/task6/tasktext 

				mkdir -p ${IMAGEROOT}/res${n}/
				convert -resize 300x300 /tmp/cur.png ${IMAGEROOT}/res${n}/task6_${CURFB}_${pos}_${CUT}.png

				if [ $? -ne 0 ];then
					return 3
				fi
			done
		done
	done
	return 0
}

function execTasks()
{
	for curtask in ${TASKDIRS}
	do 
		$curtask	
		if [ $? -ne 0 ];then
			return 3
		fi
	done
	return 0
}

#####################################################################################
###    main flow control
###
#####################################################################################
if [ $# -lt 1 ];then
	echo "Usage: $0 <datacls: normal_whole|normal_partial|normal_allfb> [taskid: 1~6] [pos0|pos1] [did: 1~5] [res: 1~5]"
	exit 1
fi
NORMALCASEDATADIR=${DATADIR}/$1
IMAGEROOT=${IMAGEROOT}/$1

checkbins
if [ $? -ne 0 ];then
	exit 1
fi

checkdata
if [ $? -ne 0 ];then
	exit 1
fi

RESID=""
if [ $# -ge 3 ];then
	RESID=$3
	if [ "$RESID" = "pos0" ];then
		RESID=""
	fi

	if [ $# -ge 4 ];then
		SUBSET=$4

		if [ $# -ge 5 ];then
			SUBRES=$5
		fi
	fi
fi

if [ $# -ge 2 ];then
	opt=$2
	if test ${opt:0:3} = "pos";then
		RESID=$2
		if [ "$RESID" = "pos0" ];then
			RESID=""
		fi
	else
		task$2
		rm -rf ${TASKPROGIDFILE}
		exit 0
	fi
fi

pos=pos0
if [ "$RESID" = "pos1" ];then
	pos=pos1
fi

execTasks
rm -rf ${TASKPROGIDFILE}

exit 0

