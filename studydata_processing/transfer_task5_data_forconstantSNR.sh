#!/bin/bash

if [ $# -lt 2 ];then
	echo "Usage: $0 <src task data dir> <dest task data dir>"
	exit 1
fi

srcblk=$1
dstblk=$2

BOXPOS_MIGRATOR=/usr/bin/normalize_coordsys.py
FIBERIDX_MIGRATOR=/usr/bin/normalize_fiberidx.py

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

			for ans in yes 
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

			for ans in no
			do
				mkdir -p ${dstreg}/s${i}/${fb}/${ans}
				: ' the first line is the task key to this data, should be kept
				while migrating
				'
				for grp in member alien
				do
					head -n1 ${srcreg}/s${i}/${fb}/${ans}/fiberidx_${grp}_region_s${i}.data \
					1> ${dstreg}/s${i}/${fb}/${ans}/fiberidx_${grp}_region_s${i}.data 

					${FIBERIDX_MIGRATOR} \
					${srcreg}/s${i}/${fb}/${ans}/fiberidx_${grp}_region_s${i}.data \
					${srcreg}/s${i}/${fb}/region_s${i}.data \
					${dstreg}/s${i}/${fb}/region_s${i}.data \
					1 \
					1>> ${dstreg}/s${i}/${fb}/${ans}/fiberidx_${grp}_region_s${i}.data 
					echo -e -n "\t ${srcreg}/s${i}/${fb}/${ans}/fiberidx_${grp}_region_s${i}.data ---> "
					echo -e "\t ${dstreg}/s${i}/${fb}/${ans}/fiberidx_${grp}_region_s${i}.data"
				done
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

				for ans in yes
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

				for ans in no
				do
					mkdir -p ${dstreg}/s${i}/${fb}/${ans}
					: ' the first line is the task key to this data, should be kept
					while migrating
					'
					for grp in member alien
					do
						head -n1 ${srcreg}/s${i}/${fb}/${p}/${ans}/fiberidx_${grp}_region_s${i}.data \
						1> ${dstreg}/s${i}/${fb}/${p}/${ans}/fiberidx_${grp}_region_s${i}.data 

						${FIBERIDX_MIGRATOR} \
						${srcreg}/s${i}/${fb}/${p}/${ans}/fiberidx_${grp}_region_s${i}.data \
						${srcreg}/s${i}/${fb}/region_s${i}.data \
						${dstreg}/s${i}/${fb}/${p}/region_s${i}.data \
						1 \
						1>> ${dstreg}/s${i}/${fb}/${p}/${ans}/fiberidx_${grp}_region_s${i}.data 
						echo -e -n "\t ${srcreg}/s${i}/${fb}/${p}/${ans}/fiberidx_${grp}_region_s${i}.data ---> "
						echo -e "\t ${dstreg}/s${i}/${fb}/${p}/${ans}/fiberidx_${grp}_region_s${i}.data"
					done
				done

			done # for p
		done # for fb
	done # for i
	return 0
}

for taskid in 5
do
	transfer_task${taskid}
done

echo "All transfering finished."
exit 0

