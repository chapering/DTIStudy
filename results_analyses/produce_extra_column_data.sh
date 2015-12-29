#!/bin/bash

TASKSET=${1:-"1 2 3 4 5 6"}
CRUISER="sh gooverblock_withprompt.sh "
CLASSES="normal_whole normal_allfb normal_partial"
CUTTER="sh data/cuttoconstantSNR.sh"

function ExtraColumns_task1()
{
	i=1
	for cls in ${CLASSES}
	do
		for RESID in pos0 pos1
		do
			echo -n "producing FA difference for task 1 in class ${cls} at ${RESID} ..."
			${CRUISER} ${cls} 1 ${RESID} | \
			grep "FAInfo:" | awk '{print $6,$2,$3,$4,$5}' > faDiffs_block${i}
			let "i+=1"
			echo " Done."
		done
	done
}

function ExtraColumns_task2()
{
	i=1
	for cls in ${CLASSES}
	do
		for RESID in pos0 pos1
		do
			echo -n "producing marked fiber info task 2 in class ${cls} at ${RESID} ..."
			${CRUISER} ${cls} 2 ${RESID} | \
			grep "nmfbers:" | awk '{print $2}' > task2_nmfb_block${i}
			let "i+=1"
			echo " Done."
		done
	done
}

function ExtraColumns_task3()
{
	i=1
	for cls in ${CLASSES}
	do
		for RESID in pos0 pos1
		do
			echo -n "producing marked fiber info task 3 in class ${cls} at ${RESID} ..."
			${CRUISER} ${cls} 3 ${RESID} | \
			grep "nmfbers:" | awk '{print $2}' > task3_nmfb_block${i}
			let "i+=1"
			echo " Done."
		done
	done
}

function ExtraColumns_task4()
{
	i=1
	for cls in ${CLASSES}
	do
		for RESID in pos0 pos1
		do
			echo -n "producing lesion info task 4 in class ${cls} at ${RESID} ..."
			${CRUISER} ${cls} 4 ${RESID} | \
			grep "LesionInfo:" | awk '{print $2,$3}' > task4_lesion_block${i}
			let "i+=1"
			echo " Done."
		done
	done
}

function ExtraColumns_task5()
{
	i=1
	for cls in ${CLASSES}
	do
		for RESID in pos0 pos1
		do
			echo -n "producing member-alien fiber info for task 5 in class ${cls} at ${RESID} ..."
			${CUTTER} data/${cls}/region_for_task5 ${RESID} | \
			grep "after cutting" | awk '{print $(NF-1),$NF}' > task5_SN_block${i}
			let "i+=1"
			echo " Done."
		done
	done
}

function ExtraColumns_task6()
{
	i=1
	for cls in ${CLASSES}
	do
		for RESID in pos0 pos1
		do
			let "i+=1"
		done
	done
}

for tid in ${TASKSET}
do
	ExtraColumns_task${tid}
done

echo "Finished."
exit 0

