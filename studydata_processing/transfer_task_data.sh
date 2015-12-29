#!/bin/bash

if [ $# -lt 2 ];then
	echo "Usage: $0 <src task data dir> <dest task data dir>"
	exit 1
fi

srcblk=$1
dstblk=$2

BOXPOS_MIGRATOR=/usr/bin/normalize_coordsys.py
FIBERIDX_MIGRATOR=/usr/bin/normalize_fiberidx.py

function transfer_task1()
{
	srcreg=${srcblk}/region_for_task1
	if [ ! -d ${srcreg} ];then
		return 1
	fi

	dstreg=${dstblk}/region_for_task1
	mkdir -p ${dstreg}

	for i in 1 2 3 4 5
	do
		if [ ! -d ${srcreg}/s${i} ];then
			continue
		fi

		mkdir -p ${dstreg}/s${i}

		#for fb in cst cc cg ifo ilf
		for fb in cg
		do
			if [ ! -d ${srcreg}/s${i}/${fb} ];then
				continue
			fi

			mkdir -p ${dstreg}/s${i}/${fb}

			: '
			transfering task data parameters need reference to both the source
			and the target geometry data
			'
			if [ ! -s ${srcreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${srcreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			if [ ! -s ${dstreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${dstreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			: '
			transfering task-specific parameters
			'
			for j in 0 1
			do
				if [ ! -s ${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data ];then
					continue
				fi

				${BOXPOS_MIGRATOR} \
				${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data \
				${srcreg}/s${i}/${fb}/region_s${i}.data \
				${dstreg}/s${i}/${fb}/region_s${i}.data \
				1> ${dstreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data 

				echo -e -n "\t ${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data ---> "
				echo -e "\t ${dstreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data"
			done

			for p in pos1 
			do
				if [ ! -d ${srcreg}/s${i}/${fb}/${p} ];then
					continue
				fi

				mkdir -p ${dstreg}/s${i}/${fb}/${p}

				: '
				the parameter sampling in the source region may have crossed
				both sides of the brain, so different set of parameters might need
				different region data for task data Class where a half brain is
				used as the region to load. This is why we need separate region
				data for another set of parameters (stored in pos1/) for task
				data Class 1
				'
				if [ ! -s ${dstreg}/s${i}/${fb}/${p}/region_s${i}.data ];then
					echo "${dstreg}/s${i}/${fb}/${p}/region_s${i}.data NOT found, aborted."
				fi

				for j in 0 1
				do
					if [ ! -s ${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data ];then
						continue
					fi

					${BOXPOS_MIGRATOR} \
					${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data \
					${srcreg}/s${i}/${fb}/region_s${i}.data \
					${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
					1> ${dstreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data 

					echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data ---> "
					echo -e "\t ${dstreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data"
				done # for j
			done # for p
		done # for fb
	done # for i
	return 0
}

function transfer_task2()
{
	srcreg=${srcblk}/region_for_task2
	if [ ! -d ${srcreg} ];then
		return 1
	fi

	dstreg=${dstblk}/region_for_task2
	mkdir -p ${dstreg}

	for i in 1 2 3 4 5
	do
		if [ ! -d ${srcreg}/s${i} ];then
			continue
		fi

		mkdir -p ${dstreg}/s${i}

		for fb in cst cc cg ifo ilf
		do
			if [ ! -d ${srcreg}/s${i}/${fb} ];then
				continue
			fi

			mkdir -p ${dstreg}/s${i}/${fb}

			: '
			transfering task data parameters need reference to both the source
			and the target geometry data
			'
			if [ ! -s ${srcreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${srcreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			if [ ! -s ${dstreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${dstreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			: '
			transfering task-specific parameters
			'
			for j in 0 1 2
			do
				if [ ! -s ${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data ];then
					continue
				fi

				${BOXPOS_MIGRATOR} \
				${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data \
				${srcreg}/s${i}/${fb}/region_s${i}.data \
				${dstreg}/s${i}/${fb}/region_s${i}.data \
				1> ${dstreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data 

				echo -e -n "\t ${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data ---> "
				echo -e "\t ${dstreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data"
			done

			${FIBERIDX_MIGRATOR} \
			${srcreg}/s${i}/${fb}/fiberidx_*_region_s${i}.data \
			${srcreg}/s${i}/${fb}/region_s${i}.data \
			${dstreg}/s${i}/${fb}/region_s${i}.data \
			1> ${dstreg}/s${i}/${fb}/fiberidx_2000002_region_s${i}.data 
			echo -e -n "\t ${srcreg}/s${i}/${fb}/fiberidx_*_region_s${i}.data ---> "
			echo -e "\t ${dstreg}/s${i}/${fb}/fiberidx_2000002_region_s${i}.data"

			for p in pos1
			do
				if [ ! -d ${srcreg}/s${i}/${fb}/${p} ];then
					continue
				fi

				mkdir -p ${dstreg}/s${i}/${fb}/${p}

				: '
				the parameter sampling in the source region may have crossed
				both sides of the brain, so different set of parameters might need
				different region data for task data Class where a half brain is
				used as the region to load. This is why we need separate region
				data for another set of parameters (stored in pos1/) for task
				data Class 1
				'
				if [ ! -s ${dstreg}/s${i}/${fb}/${p}/region_s${i}.data ];then
					echo "${dstreg}/s${i}/${fb}/${p}/region_s${i}.data NOT found, aborted."
				fi

				for j in 0 1 2
				do
					if [ ! -s ${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data ];then
						continue
					fi

					${BOXPOS_MIGRATOR} \
					${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data \
					${srcreg}/s${i}/${fb}/region_s${i}.data \
					${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
					1> ${dstreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data 

					echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data ---> "
					echo -e "\t ${dstreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data"
				done # for j

				${FIBERIDX_MIGRATOR} \
				${srcreg}/s${i}/${fb}/${p}/fiberidx_*_region_s${i}.data \
				${srcreg}/s${i}/${fb}/region_s${i}.data \
				${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
				1> ${dstreg}/s${i}/${fb}/${p}/fiberidx_2000002_region_s${i}.data 
				echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/fiberidx_*_region_s${i}.data ---> "
				echo -e "\t ${dstreg}/s${i}/${fb}/${p}/fiberidx_2000002_region_s${i}.data"

			done # for p
		done # for fb
	done # for i
	return 0
}

function transfer_task3()
{
	srcreg=${srcblk}/region_for_task3
	if [ ! -d ${srcreg} ];then
		return 1
	fi

	dstreg=${dstblk}/region_for_task3
	mkdir -p ${dstreg}

	for i in 1 2 3 4 5
	do
		if [ ! -d ${srcreg}/s${i} ];then
			continue
		fi

		mkdir -p ${dstreg}/s${i}

		for fb in cst cc cg ifo ilf
		do
			if [ ! -d ${srcreg}/s${i}/${fb} ];then
				continue
			fi

			mkdir -p ${dstreg}/s${i}/${fb}

			: '
			transfering task data parameters need reference to both the source
			and the target geometry data
			'
			if [ ! -s ${srcreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${srcreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			if [ ! -s ${dstreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${dstreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			${FIBERIDX_MIGRATOR} \
			${srcreg}/s${i}/${fb}/fiberidx_*_region_s${i}.data \
			${srcreg}/s${i}/${fb}/region_s${i}.data \
			${dstreg}/s${i}/${fb}/region_s${i}.data \
			1> ${dstreg}/s${i}/${fb}/fiberidx_2000003_region_s${i}.data 
			echo -e -n "\t ${srcreg}/s${i}/${fb}/fiberidx_*_region_s${i}.data ---> "
			echo -e "\t ${dstreg}/s${i}/${fb}/fiberidx_2000003_region_s${i}.data"

			for p in pos1 
			do
				if [ ! -d ${srcreg}/s${i}/${fb}/${p} ];then
					continue
				fi

				mkdir -p ${dstreg}/s${i}/${fb}/${p}

				: '
				the parameter sampling in the source region may have crossed
				both sides of the brain, so different set of parameters might need
				different region data for task data Class where a half brain is
				used as the region to load. This is why we need separate region
				data for another set of parameters (stored in pos1/) for task
				data Class 1
				'
				if [ ! -s ${dstreg}/s${i}/${fb}/${p}/region_s${i}.data ];then
					echo "${dstreg}/s${i}/${fb}/${p}/region_s${i}.data NOT found, aborted."
				fi

				${FIBERIDX_MIGRATOR} \
				${srcreg}/s${i}/${fb}/${p}/fiberidx_*_region_s${i}.data \
				${srcreg}/s${i}/${fb}/region_s${i}.data \
				${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
				1> ${dstreg}/s${i}/${fb}/${p}/fiberidx_2000003_region_s${i}.data 
				echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/fiberidx_*_region_s${i}.data ---> "
				echo -e "\t ${dstreg}/s${i}/${fb}/${p}/fiberidx_2000003_region_s${i}.data"

			done # for p
		done # for fb
	done # for i
	return 0
}

function transfer_task4()
{
	srcreg=${srcblk}/region_for_task4
	if [ ! -d ${srcreg} ];then
		return 1
	fi

	dstreg=${dstblk}/region_for_task4
	mkdir -p ${dstreg}

	for i in 1 2 3 4 5
	do
		if [ ! -d ${srcreg}/s${i} ];then
			continue
		fi

		mkdir -p ${dstreg}/s${i}

		for fb in cst cc cg ifo ilf
		do
			if [ ! -d ${srcreg}/s${i}/${fb} ];then
				continue
			fi

			mkdir -p ${dstreg}/s${i}/${fb}

			: '
			transfering task data parameters need reference to both the source
			and the target geometry data
			'
			if [ ! -s ${srcreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${srcreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			if [ ! -s ${dstreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${dstreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			: '
			transfering task-specific parameters
			'
			for j in 0
			do
				if [ ! -s ${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data ];then
					continue
				fi

				${BOXPOS_MIGRATOR} \
				${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data \
				${srcreg}/s${i}/${fb}/region_s${i}.data \
				${dstreg}/s${i}/${fb}/region_s${i}.data \
				1> ${dstreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data 

				echo -e -n "\t ${srcreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data ---> "
				echo -e "\t ${dstreg}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data"
			done

			for p in pos1 
			do
				if [ ! -d ${srcreg}/s${i}/${fb}/${p} ];then
					continue
				fi

				mkdir -p ${dstreg}/s${i}/${fb}/${p}

				: '
				the parameter sampling in the source region may have crossed
				both sides of the brain, so different set of parameters might need
				different region data for task data Class where a half brain is
				used as the region to load. This is why we need separate region
				data for another set of parameters (stored in pos1/) for task
				data Class 1
				'
				if [ ! -s ${dstreg}/s${i}/${fb}/${p}/region_s${i}.data ];then
					echo "${dstreg}/s${i}/${fb}/${p}/region_s${i}.data NOT found, aborted."
				fi

				for j in 0
				do
					if [ ! -s ${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data ];then
						continue
					fi

					${BOXPOS_MIGRATOR} \
					${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data \
					${srcreg}/s${i}/${fb}/region_s${i}.data \
					${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
					1> ${dstreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data 

					echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data ---> "
					echo -e "\t ${dstreg}/s${i}/${fb}/${p}/tumorbox_${j}_region_s${i}.data"
				done # for j

			done # for p
		done # for fb
	done # for i
	return 0
}

function transfer_task5()
{
	srcreg=${srcblk}/region_for_task5
	if [ ! -d ${srcreg} ];then
		return 1
	fi

	dstreg=${dstblk}/region_for_task5
	mkdir -p ${dstreg}

	for i in 1 2 3 4 5
	do
		if [ ! -d ${srcreg}/s${i} ];then
			continue
		fi

		mkdir -p ${dstreg}/s${i}

		for fb in cst cc cg ifo ilf
		do
			if [ ! -d ${srcreg}/s${i}/${fb} ];then
				continue
			fi

			mkdir -p ${dstreg}/s${i}/${fb}

			: '
			transfering task data parameters need reference to both the source
			and the target geometry data
			'
			if [ ! -s ${srcreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${srcreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			if [ ! -s ${dstreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${dstreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			for ans in yes no
			do
				mkdir -p ${dstreg}/s${i}/${fb}/${ans}
				: ' the first line is the task key to this data, should be kept
				while migrating
				'
				head -n1 ${srcreg}/s${i}/${fb}/${ans}/fiberidx_*_region_s${i}.data \
				1> ${dstreg}/s${i}/${fb}/${ans}/fiberidx_2000005_region_s${i}.data 

				${FIBERIDX_MIGRATOR} \
				${srcreg}/s${i}/${fb}/${ans}/fiberidx_*_region_s${i}.data \
				${srcreg}/s${i}/${fb}/region_s${i}.data \
				${dstreg}/s${i}/${fb}/region_s${i}.data \
				1 \
				1>> ${dstreg}/s${i}/${fb}/${ans}/fiberidx_2000005_region_s${i}.data 
				echo -e -n "\t ${srcreg}/s${i}/${fb}/${ans}/fiberidx_*_region_s${i}.data ---> "
				echo -e "\t ${dstreg}/s${i}/${fb}/${ans}/fiberidx_2000005_region_s${i}.data"
			done

			for p in pos1 
			do
				if [ ! -d ${srcreg}/s${i}/${fb}/${p} ];then
					continue
				fi

				mkdir -p ${dstreg}/s${i}/${fb}/${p}

				: '
				the parameter sampling in the source region may have crossed
				both sides of the brain, so different set of parameters might need
				different region data for task data Class where a half brain is
				used as the region to load. This is why we need separate region
				data for another set of parameters (stored in pos1/) for task
				data Class 1
				'
				if [ ! -s ${dstreg}/s${i}/${fb}/${p}/region_s${i}.data ];then
					echo "${dstreg}/s${i}/${fb}/${p}/region_s${i}.data NOT found, aborted."
				fi

				for ans in yes no
				do
					mkdir -p ${dstreg}/s${i}/${fb}/${p}/${ans}

					: ' the first line is the task key to this data, should be kept
					while migrating
					'
					head -n1 ${srcreg}/s${i}/${fb}/${p}/${ans}/fiberidx_*_region_s${i}.data \
					1> ${dstreg}/s${i}/${fb}/${p}/${ans}/fiberidx_2000005_region_s${i}.data 

					${FIBERIDX_MIGRATOR} \
					${srcreg}/s${i}/${fb}/${p}/${ans}/fiberidx_*_region_s${i}.data \
					${srcreg}/s${i}/${fb}/region_s${i}.data \
					${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
					1 \
					1>> ${dstreg}/s${i}/${fb}/${p}/${ans}/fiberidx_2000005_region_s${i}.data 
					echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/${ans}/fiberidx_*_region_s${i}.data ---> "
					echo -e "\t ${dstreg}/s${i}/${fb}/${p}/${ans}/fiberidx_2000005_region_s${i}.data"
				done

			done # for p
		done # for fb
	done # for i
	return 0
}

function transfer_task6()
{
	srcreg=${srcblk}/region_for_task6
	if [ ! -d ${srcreg} ];then
		return 1
	fi

	dstreg=${dstblk}/region_for_task6
	mkdir -p ${dstreg}

	for i in 1 2 3 4 5
	do
		if [ ! -d ${srcreg}/s${i} ];then
			continue
		fi

		mkdir -p ${dstreg}/s${i}

		for fb in cst cg ilf
		do
			if [ ! -d ${srcreg}/s${i}/${fb} ];then
				continue
			fi

			mkdir -p ${dstreg}/s${i}/${fb}

			: '
			transfering task data parameters need reference to both the source
			and the target geometry data
			'
			if [ ! -s ${srcreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${srcreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			if [ ! -s ${dstreg}/s${i}/${fb}/region_s${i}.data ];then
				echo "${dstreg}/s${i}/${fb}/region_s${i}.data NOT found, aborted."
			fi

			for CUT in nocut leftcut rightcut
			do
				: ' the first line is the task key to this data, should be kept
				while migrating to keep consistent with Class II data even
				though it is useless as a matter of fact
				'
				head -n1 ${srcreg}/s${i}/${fb}/fiberidx_${CUT}_s${i}.data \
				1> ${dstreg}/s${i}/${fb}/fiberidx_${CUT}_s${i}.data 

				${FIBERIDX_MIGRATOR} \
				${srcreg}/s${i}/${fb}/fiberidx_${CUT}_s${i}.data \
				${srcreg}/s${i}/${fb}/region_s${i}.data \
				${dstreg}/s${i}/${fb}/region_s${i}.data \
				1 \
				1>> ${dstreg}/s${i}/${fb}/fiberidx_${CUT}_s${i}.data 
				echo -e -n "\t ${srcreg}/s${i}/${fb}/fiberidx_${CUT}_s${i}.data ---> "
				echo -e "\t ${dstreg}/s${i}/${fb}/fiberidx_${CUT}_s${i}.data"
			done

			for p in pos1 
			do
				if [ ! -d ${srcreg}/s${i}/${fb}/${p} ];then
					continue
				fi

				mkdir -p ${dstreg}/s${i}/${fb}/${p}

				: '
				the parameter sampling in the source region may have crossed
				both sides of the brain, so different set of parameters might need
				different region data for task data Class where a half brain is
				used as the region to load. This is why we need separate region
				data for another set of parameters (stored in pos1/) for task
				data Class 1
				'
				if [ ! -s ${dstreg}/s${i}/${fb}/${p}/region_s${i}.data ];then
					echo "${dstreg}/s${i}/${fb}/${p}/region_s${i}.data NOT found, aborted."
				fi

				for CUT in nocut leftcut rightcut
				do
					: ' the first line is the task key to this data, should be kept
					while migrating to keep consistent with Class II data even
					though it is useless as a matter of fact
					'
					head -n1 ${srcreg}/s${i}/${fb}/${p}/fiberidx_${CUT}_s${i}.data \
					1> ${dstreg}/s${i}/${fb}/${p}/fiberidx_${CUT}_s${i}.data 

					${FIBERIDX_MIGRATOR} \
					${srcreg}/s${i}/${fb}/${p}/fiberidx_${CUT}_s${i}.data \
					${srcreg}/s${i}/${fb}/region_s${i}.data \
					${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
					1 \
					1>> ${dstreg}/s${i}/${fb}/${p}/fiberidx_${CUT}_s${i}.data 
					echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/fiberidx_${CUT}_s${i}.data ---> "
					echo -e "\t ${dstreg}/s${i}/${fb}/${p}/fiberidx_${CUT}_s${i}.data"
				done

			done # for p
		done # for fb
	done # for i
	return 0
}

for taskid in 1
do
	transfer_task${taskid}
done


echo "All transfering finished."
exit 0


