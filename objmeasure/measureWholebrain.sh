#!/bin/bash

CALC=/usr/bin/calcObjMeasures.py

BASE=${1:-`pwd`/wholebrain/}
TENSORDIR=${2:-`pwd`}

function metric_wholebrain()
{
	fnout="wholebrain_metrics.txt"
	touch ${fnout}
	for ((i=1;i<=5;i++))
	do
		if [ ! -s ${BASE}/normal_s${i}.data ];then
			echo "${BASE}/normal_s${i}.data not found."
			exit 1
		fi

		: '
		calculate metrics using prepared tensor information
		'
		if [ ! -s ${TENSORDIR}/normal_s${i}.tensorinfo ];then
			echo "${TENSORDIR}/normal_s${i}.tensorinfo not found."
			exit 1
		fi
		echo "resolution s${i}" >> ${fnout}
		${CALC} ${BASE}/normal_s${i}.data \
			${TENSORDIR}/normal_s${i}.tensorinfo >> ${fnout}

		echo "Metrics for whole brain in resolution of s${i} ... finished"
	done
}

metric_wholebrain

echo "Done!"
exit 0
