#!/bin/bash 
BASEDIR=`pwd`

BINDIR=${BASEDIR}/bin
TASKDIRS="task1 task2 task3 task4 task5 task6"
EXECUTABLES="singleitr multiviewitr multiwindowitr"

DATADIR=${BASEDIR}/data
NORMALCASEDATADIR=${DATADIR}/normal
#ABNORMALCASEDATADIR=${DATADIR}/agenesis
TUMORDATADIR=${DATADIR}/tumor
BUNDLES="cst cg cc ifo ilf"

FBARRARY=(cst cg cc ifo ilf)
SUBSET="0 1 2 3 4"
CURFB=""

TASKPROGIDFILE=${BASEDIR}/taskprogidfile
TASKNO=0

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

	if [ ! -s ${TUMORDATADIR}/potato.sm ];then
		echo "ERROR: tumor data NOT found in ${TUMORDATADIR}."
		return 1
	fi

	return 0
}

function updatetaskprog()
{
	let "TASKNO += 1"
	echo -e "\n\n data review progress : $TASKNO / <= 175" > ${TASKPROGIDFILE}
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
		for n in 1 2 3 4 5
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog
			${BINDIR}/task1/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/skeleton_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/tumorbox_1_region_s${n}.data \
			-t ${BINDIR}/task1/tasktext \
			-p ${TASKPROGIDFILE} 

			if [ $? -ne 0 ];then
				return 3
			fi
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
	local keys1=(1 1 2 3 1)
	local keys2=(2 3 2 2 1)
	local keys3=(3 2 1 3 1)
	local keys4=(3 2 3 2 3)
	local keys5=(1 1 1 1 2)
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog
			CURKEY=$(eval echo \${keys$((di+1))[$((n-1))]})

			${BINDIR}/task2/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/fiberidx_*_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_1_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_2_region_s${n}.data \
			-t ${BINDIR}/task2/tasktext \
			-p ${TASKPROGIDFILE} \
			-k ${CURKEY}
			#-k ${keys[$((n-1))]} 

			if [ $? -ne 0 ];then
				return 3
			fi
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
		for n in 1 2 3 4 5
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog
			${BINDIR}/task3/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/fiberidx_*_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-t ${BINDIR}/task3/tasktext 

			if [ $? -ne 0 ];then
				return 3
			fi
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
		for n in 1 2 3 4 5
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog

			${BINDIR}/task4/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/skeleton_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-t ${BINDIR}/task4/tasktext 

			if [ $? -ne 0 ];then
				return 3
			fi
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
		for n in 1 2 3 4 5
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			echo "#####    when the answer is yes    #### "

			updatetaskprog
			${BINDIR}/task5/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/yes/fiberidx_*_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-t ${BINDIR}/task5/tasktext 
			
			echo -e "\n#####    when the answer is no ####" 

			updatetaskprog
			${BINDIR}/task5/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/skeleton_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/no/fiberidx_*_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-t ${BINDIR}/task5/tasktext 

			if [ $? -ne 0 ];then
				return 3
			fi
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

		for n in 1 2 3 4 5
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			for CUT in nocut leftcut rightcut
			do
				echo "##### data class: $CUT ####"
				updatetaskprog

				${BINDIR}/task6/singleitr \
				-f \
				${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/region_s${n}.data \
				-i \
				${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/fiberidx_${CUT}_s${n}.data \
				-s \
				${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/skeleton_region_s${n}.data \
				-p ${TASKPROGIDFILE} \
				-t ${BINDIR}/task6/tasktext 

				if [ $? -ne 0 ];then
					return 3
				fi
			done
		done
	done
	return 0
}

#-----------------------------------------#
#
# task 7 
#
#-----------------------------------------#
function task7()
{
	for di in $SUBSET
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			updatetaskprog

			${BINDIR}/task7/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task7/s${n}/${CURFB}/region_s${n}.data \
			-s \
			${NORMALCASEDATADIR}/region_for_task7/s${n}/${CURFB}/skeleton_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task7/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
			-p ${TASKPROGIDFILE} \
			-t ${BINDIR}/task7/tasktext 

			if [ $? -ne 0 ];then
				return 3
			fi
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

checkbins
if [ $? -ne 0 ];then
	exit 1
fi

checkdata
if [ $? -ne 0 ];then
	exit 1
fi

if [ $# -ge 1 ];then
	task$1
	rm -rf ${TASKPROGIDFILE}
	exit 0
fi

execTasks
rm -rf ${TASKPROGIDFILE}

exit 0

