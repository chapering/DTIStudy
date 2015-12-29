#!/bin/bash

CALC=/usr/bin/fiberDensityPerVoxel.py

BASE=${1:-`pwd`/wholebrain/}
TOIBASE=${2:-`pwd`/TOIs/}

function metric_wholebrain()
{
	for ((i=4;i<=5;i++))
	do
		fnout="wholebrain_extra_metrics_s${i}.txt"
		touch ${fnout}

		if [ ! -s ${BASE}/normal_s${i}.data ];then
			echo "${BASE}/normal_s${i}.data not found."
			exit 1
		fi

		: '
		calculate metrics 
		'
		echo "resolution s${i}" >> ${fnout}
		${CALC} ${BASE}/normal_s${i}.data \
			${i} 1>> ${fnout} 2>&1

		echo "Metrics for whole brain in resolution of s${i} ... finished"
	done
}

function metric_TOIs()
{
	for fb in cst cg cc ifo ilf
	do
		for ((i=1;i<=5;i++))
		do
			fnout="${fb}_extra_metrics_s${i}.txt"
			touch ${fnout}

			if [ ! -s ${TOIBASE}/s${i}/${fb}/region_s${i}.data ];then
				echo "${TOIBASE}/s${i}/${fb}/region_s${i}.data not found."
				exit 1
			fi

			: '
			calculate metrics 
			'
			echo "resolution s${i}" >> ${fnout}
			${CALC} ${TOIBASE}/s${i}/${fb}/region_s${i}.data \
				${i} 1>> ${fnout} 2>&1

			echo "Metrics for whole brain in resolution of s${i} ... finished"
		done
	done
}

metric_wholebrain
#metric_TOIs

echo "Done!"
exit 0
