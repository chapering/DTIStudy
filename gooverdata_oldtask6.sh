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
CURFB=""

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

#-----------------------------------------#
#
# task 1
#
#-----------------------------------------#
function task1()
{
	local keys=(2 1 2 2 1)
	for di in 0 1 2 3 4 
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			#echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			${BINDIR}/task1/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/tumorbox_1_region_s${n}.data \
			-t ${BINDIR}/task1/tasktext \
			-k ${keys[$di]}

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
	local keys=(2 1 2 2 1)
	for di in 0 1 2 3 4 
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			#echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 
			${BINDIR}/task2/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/fiberidx_*_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_1_region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_2_region_s${n}.data \
			-t ${BINDIR}/task2/tasktext \
			-k ${keys[$di]} 

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
	for di in 0 1 2 3 4 
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			#echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 

			#echo "#####    when the tumor touches fibers    #### "
			${BINDIR}/task3/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/touch/tumorbox_0_region_s${n}.data \
			-u \
			${TUMORDATADIR}/potato.sm \
			-t ${BINDIR}/task3/tasktext 

			#echo -e "\n#####    when the tumor is tangential to fibers    ####" 
			${BINDIR}/task3/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task3/s${n}/${CURFB}/nontouch/tumorbox_0_region_s${n}.data \
			-u \
			${TUMORDATADIR}/potato.sm \
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
	for di in 0 1 2 3 4 
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			#echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 

			${BINDIR}/task4/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/region_s${n}.data \
			-j \
			${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
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
	for di in 0 1 2 3 4 
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			#echo "seeding resolution of ${n}x${n}x${n}, bundle of $CURFB" 

			#echo "#####    when the answer is yes    #### "
			${BINDIR}/task5/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/yes/fiberidx_*_region_s${n}.data \
			-t ${BINDIR}/task5/tasktext 
			
			echo -e "\n#####    when the answer is no ####" 
			${BINDIR}/task5/singleitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task5/s${n}/${CURFB}/no/fiberidx_*_region_s${n}.data \
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
	for di in 0 1 2 3 4 
	do
		CURFB=${FBARRARY[$di]}
		for n in 1 2 3 4 5
		do
			${BINDIR}/task6/multiviewitr \
			-f \
			${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/region_s${n}.data \
			-f \
			${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/fiberidx_*_region_s${n}.data \
			-i \
			${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/fiberidxfewer_region_s${n}.data \
			-t ${BINDIR}/task6/tasktext 

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

execTasks

exit 0

