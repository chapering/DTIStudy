
: '
this script, presuming that all executables and data have been readyed in the
configurable directories as follows, boosts a run of the whole study of seeding
methods in DTI tractography.
--- this version is proprietarily written for formal seeding study.
'
BASEDIR=`pwd`

BINDIR=${BASEDIR}/bin
TASKDIRS="pretask task1 task2 task3 task4 task5 task6"
EXECUTABLES="singleitr multiviewitr multiwindowitr"

DATADIR=${BASEDIR}/data
NORMALCASEDATADIR=${DATADIR}/normal
#ABNORMALCASEDATADIR=${DATADIR}/agenesis
TUMORDATADIR=${DATADIR}/tumor
BUNDLES="cst cg cc ifo ilf"

LOG=${BASEDIR}/studylog

ORDERARRAY=()
NUMROW=0 #number of row in the order matrix
NUMCOL=0 #number of column in the order matrix
FBARRARY=(cst cg cc ifo ilf)
CURORDER=""
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

: '
$1 - task order text file
$? - 0 for success and 1 otherwise
'
function readorder()
{
	if [ $# -lt 1 -o ! -s $1 ];then
		echo "In readorder: too few arguments or file does not exist."
		return 1
	fi

	: '
	take the number of fields in the first line as the number of column  of
	the matrix to read into
	'
	local fldnum=0
	local currow=0
	#cat $1 | \
	while read curline
	do
		curfldnum=`echo $curline | awk '{print NF}'`
		if [ $fldnum -eq 0 ];then
			fldnum=$curfldnum
		elif [ $curfldnum -ne $fldnum ];then
			echo "inconsistent row - different number of columns."
			return 1
		fi

		local currol=0
		for num in $curline
		do
			let "index = $currow * $fldnum + $currol"
			ORDERARRAY[$index]=$num
			let "currol += 1"
		done

		let "currow += 1"

	done < $1

	let NUMROW=$currow
	let NUMCOL=$fldnum
	#echo "NUMCOL=$NUMCOL"
	#echo "NUMROW=$NUMROW"

	return 0
}

function print_order()
{
	echo ${ORDERARRAY[*]} | xargs -n $NUMCOL
}

function taskflag()
{
	echo -e "\n##############################################################" >> $LOG
	echo "                           TASK $1                            " >> $LOG
	echo -e "##############################################################\n" >> $LOG
}

function updatetaskprog()
{
	let "TASKNO += 1"
	echo -e "\n\n overall progress : $TASKNO / 40" > ${TASKPROGIDFILE}
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

	local keys=(2 2 2 2 2)
	local di=0
	for n in ${CURORDER}
	do
		updatetaskprog

		CURFB=${FBARRARY[$di]}
		let "di += 1"

		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task1/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/region_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/${CURFB}/tumorbox_1_region_s${n}.data \
		-t ${BINDIR}/task1/tasktext \
		-k ${keys[$pi]} \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi
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
	taskflag 2

	local keys=(2 1 2 2 1)
	local di=0
	for n in ${CURORDER}
	do
		updatetaskprog

		CURFB=${FBARRARY[$di]}
		let "di += 1"

		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task2/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/region_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/fiberidx_*_region_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_1_region_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task2/s${n}/${CURFB}/tumorbox_2_region_s${n}.data \
		-t ${BINDIR}/task2/tasktext \
		-k ${keys[$pi]} \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi
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
	taskflag 3

	local di1=0
	local di2=1
	#for n in ${CURORDER}
	for ((i=0;i<$NUMCOL;i++))
	do
		updatetaskprog

		let "index1 = $pi * $NUMCOL + $i"
		n1=${ORDERARRAY[index1]}

		CURFB=${FBARRARY[$di1]}
		let "di1 += 1"

		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n1}x${n1}x${n1} #####" >> $LOG
		echo "#####    when the tumor touches fibers    #### ">> $LOG
		${BINDIR}/task3/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task3/s${n1}/${CURFB}/region_s${n1}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task3/s${n1}/${CURFB}/touch/tumorbox_0_region_s${n1}.data \
		-u \
		${TUMORDATADIR}/potato.sm \
		-t ${BINDIR}/task3/tasktext \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi

		
		updatetaskprog

		let "index2 = ($NUMCOL+$pi) * $NUMCOL + $i"
		n2=${ORDERARRAY[index2]}

		CURFB=${FBARRARY[$di2]}
		let "di2 += 1"
		if [ ${di2} -ge 5 ];then
			let "di2 = 0"
		fi

		echo -e "\n#####    when the tumor is tangential to fibers    ####" >> $LOG
		${BINDIR}/task3/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task3/s${n2}/${CURFB}/region_s${n2}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task3/s${n2}/${CURFB}/nontouch/tumorbox_0_region_s${n2}.data \
		-u \
		${TUMORDATADIR}/potato.sm \
		-t ${BINDIR}/task3/tasktext \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi
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
	taskflag 4

	local di=0
	for n in ${CURORDER}
	do
		updatetaskprog

		CURFB=${FBARRARY[$di]}
		let "di += 1"

		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task4/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/region_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-j \
		${NORMALCASEDATADIR}/region_for_task4/s${n}/${CURFB}/tumorbox_0_region_s${n}.data \
		-t ${BINDIR}/task4/tasktext \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi
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
	taskflag 5

	local di1=0
	local di2=1

	#for n in ${CURORDER}
	for ((i=0;i<$NUMCOL;i++))
	do
		updatetaskprog

		let "index1 = $pi * $NUMCOL + $i"
		n1=${ORDERARRAY[index1]}

		CURFB=${FBARRARY[$di1]}
		let "di1 += 1"

		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n1}x${n1}x${n1} #####" >> $LOG
		echo "#####    when the answer is yes    #### ">> $LOG
		${BINDIR}/task5/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task5/s${n1}/${CURFB}/region_s${n1}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task5/s${n1}/${CURFB}/yes/fiberidx_*_region_s${n1}.data \
		-t ${BINDIR}/task5/tasktext \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi

		
		updatetaskprog

		let "index2 = ($NUMCOL+$pi) * $NUMCOL + $i"
		n2=${ORDERARRAY[index2]}

		CURFB=${FBARRARY[$di2]}
		let "di2 += 1"
		if [ ${di2} -ge 5 ];then
			let "di2 = 0"
		fi

		echo -e "\n#####    when the answer is no ####" >> $LOG
		${BINDIR}/task5/singleitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task5/s${n2}/${CURFB}/region_s${n2}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task5/s${n2}/${CURFB}/no/fiberidx_*_region_s${n2}.data \
		-t ${BINDIR}/task5/tasktext \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi
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
	taskflag 6
	local di=0
	for n in ${CURORDER}
	do
		updatetaskprog

		CURFB=${FBARRARY[$di]}
		let "di += 1"

		echo -e "\n##### with Normal case #####" >> $LOG
		echo "##### under seeding resolution of ${n}x${n}x${n} #####" >> $LOG
		${BINDIR}/task6/multiviewitr \
		-f \
		${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/region_s${n}.data \
		-f \
		${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/region_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/fiberidx_*_region_s${n}.data \
		-i \
		${NORMALCASEDATADIR}/region_for_task6/s${n}/${CURFB}/fiberidxfewer_region_s${n}.data \
		-s \
		${NORMALCASEDATADIR}/region_for_task1/s${n}/cst/skeleton_region_s${n}.data \
		-t ${BINDIR}/task6/tasktext \
		-p ${TASKPROGIDFILE} \
		-V 1>> $LOG 2>&1

		if [ $? -ne 0 ];then
			return 3
		fi
	done
	return 0
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

function execTasks()
{
	> $LOG

	bstart=`date +%s`

	for curtask in ${TASKDIRS}
	do 
		curstart=`date +%s`
		$curtask	
		ret=$?
		curend=`date +%s`
		costOfThisTask ${curstart} ${curend}

		if [ $ret -ne 0 ];then
			return 3
		fi
	done

	end=`date +%s`
	echo -e "\n################## ALL FINISHED #######################" >> $LOG
	((d=end-bstart))
	((h=d/3600))
	((m=d%3600/60))
	((s=d%3600%60))
	echo " Time cost: $h hours $m minutes $s seconds." >> $LOG
	return 0
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

if [ $# -lt 2 ];then
	echo "Usage: $0 <taskorder> <participant index>"
	exit 1
fi

orderfn=$1
pi=$2
readorder ${orderfn}
if [ $? -ne 0 ];then
	exit 1
fi

#print_order

if [ $pi -gt $NUMCOL ];then
	echo "Participant index out of bound."
	exit 1
fi
LOG="${LOG}_p${pi}"
let "pi -= 1"

for((i=0;i<$NUMCOL;i++))
do
	let "index = $pi * $NUMCOL + $i"
	CURORDER="$CURORDER ${ORDERARRAY[index]}"
done

execTasks
ret=$?

rm -rf ${TASKPROGIDFILE}
if [ $ret -eq 0 ];then
	echo "All finished, thank you!"
else
	echo "Study terminated in advance, thank you all the same."
fi
echo

exit 0

