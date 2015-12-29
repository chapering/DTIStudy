#!/bin/bash
: '
this script is set for building the seeding method study; it depends on the
preinstallation of GDCM.
'

BASEDIR=`pwd`
SRCDIR=${BASEDIR}/src
BINDIR=${BASEDIR}/bin
EXECUTABLES="singleitr multiviewitr multiwindowitr"
TASKDIRS="pretask task1 task2 task3 task4 task5 task6 task7 task8 task9"

if [ ! -d ${SRCDIR} ];then
	echo "FATAL: directory ${SRCDIR} NOT found."
	exit -1
fi

: '
build executables for each of the tasks and set them up in task-wise
sub-directories under the $BIN directory
'
mkdir -p ${BINDIR}
for ts in ${TASKDIRS}
do
	echo -e "building for ${ts}..."
	mkdir -p ${BINDIR}/${ts}

	if [ ! -d ${SRCDIR}/${ts} ];then
		echo "FATAL: sub-directory ${SRC}/${ts} NOT found."
		rm -rf ${BINDIR}/*
		exit -1
	fi

	cp ${SRCDIR}/${ts}/*.{h,cpp} ${SRCDIR}
	make -C ${SRCDIR} -w all

	for bin in ${EXECUTABLES}
	do
		if [ ! -s ${SRCDIR}/${bin} ];then
			echo "ERROR: executable ${SRCDIR}/${bin} for ${ts} failed to be built."
			rm -rf ${BINDIR}/*
			exit -1
		fi
		mv -f ${SRCDIR}/${bin} ${BINDIR}/${ts}
	done
	make -C ${SRCDIR} -w cleanall

	cp ${SRCDIR}/${ts}/{tasktext,helptext} ${BINDIR}/${ts}

	echo -e "\t\t------ finished."
done

DATADIR=${BASEDIR}/data
NORMALCASEDATA=normal.data
ABNORMALCASEDATA=agenesis.data
NORMALCASEDATA2=normal2.data
ABNORMALCASEDATA2=agenesis2.data

for ts in task1 task2 task3 task4 task5
do
	cp -f ${DATADIR}/${NORMALCASEDATA} ${BINDIR}/${ts}/
	cp -f ${DATADIR}/fiberidx_${ts}.data ${BINDIR}/${ts}
done

cp -f ${DATADIR}/${NORMALCASEDATA} ${BINDIR}/task6/
cp -f ${DATADIR}/fiberidx_task3.data ${BINDIR}/task6/
cp -f ${DATADIR}/${ABNORMALCASEDATA} ${BINDIR}/task7/
cp -f ${DATADIR}/fiberidx_task3.data ${BINDIR}/task7/

cp -f ${DATADIR}/${NORMALCASEDATA} ${BINDIR}/task8/
cp -f ${DATADIR}/fiberidx_task3.data ${BINDIR}/task8/
cp -f ${DATADIR}/${NORMALCASEDATA} ${BINDIR}/task9/
cp -f ${DATADIR}/${ABNORMALCASEDATA} ${BINDIR}/task9/
cp -f ${DATADIR}/fiberidx_task3.data ${BINDIR}/task9/

echo -e "\n\t---- setup completed ----\t\n"

exit 0

