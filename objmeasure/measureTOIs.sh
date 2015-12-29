#!/bin/bash

CALC=/usr/bin/calcObjMeasures.py
SCALER=/usr/bin/stlinesynctrans.py

BASE=${1:-`pwd`}
TENSORDIR=${2:-`pwd`}
SCALEREFDIR=${3:-/home/chap/DTI-SVL/onSeedingStudy/density_study/data/normal_whole/regions/}

for ((i=1;i<=5;i++));do 
	for fb in cst cc cg ilf ifo;
	do  
		cp ../data/normal_allfb/region_for_task1/s${i}/${fb}/region_s${i}.data \
			TOIs/s${i}/${fb}/region_s${i}.data; 
	done
done

function metric_cst()
{
	for fb in cst
	do
		fnout="${fb}_metrics.txt"
		touch ${fnout}
		for ((i=1;i<=5;i++))
		do
			if [ ! -s ${BASE}/s${i}/${fb}/region_s${i}.data ];then
				echo "${BASE}/s${i}/${fb}/region_s${i}.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s1.data ];then
				echo "${SCALEREFDIR}/fa_normal_s1.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s${i}.data ];then
				echo "${SCALEREFDIR}/fa_normal_s${i}.data not found."
				exit 1
			fi
			: '
			scale model geometries in the TOI to its original metric
			'
			${SCALER} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${SCALEREFDIR}/fa_normal_s1.data \
				${SCALEREFDIR}/fa_normal_s${i}.data 1> tmp.data
			mv tmp.data ${BASE}/s${i}/${fb}/region_s${i}.data

			: '
			calculate metrics using prepared tensor information
			'
			if [ ! -s ${TENSORDIR}/normal_s${i}.tensorinfo ];then
				echo "${TENSORDIR}/normal_s${i}.tensorinfo not found."
				exit 1
			fi
			echo "resolution s${i}" >> ${fnout}
			${CALC} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${TENSORDIR}/normal_s${i}.tensorinfo >> ${fnout}

			echo "Metrics for TOI : ${fb} in resolution of s${i} ... finished"
		done
	done
}

function metric_cg()
{
	for fb in cg 
	do
		fnout="${fb}_metrics.txt"
		touch ${fnout}
		for ((i=1;i<=5;i++))
		do
			if [ ! -s ${BASE}/s${i}/${fb}/region_s${i}.data ];then
				echo "${BASE}/s${i}/${fb}/region_s${i}.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s1.data ];then
				echo "${SCALEREFDIR}/fa_normal_s1.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s${i}.data ];then
				echo "${SCALEREFDIR}/fa_normal_s${i}.data not found."
				exit 1
			fi
			: '
			scale model geometries in the TOI to its original metric
			'
			${SCALER} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${SCALEREFDIR}/raw_normal_s1.data \
				${SCALEREFDIR}/raw_normal_s${i}.data 1> tmp.data
			mv tmp.data ${BASE}/s${i}/${fb}/region_s${i}.data

			: '
			calculate metrics using prepared tensor information
			'
			if [ ! -s ${TENSORDIR}/normal_s${i}.tensorinfo.flipped ];then
				echo "${TENSORDIR}/normal_s${i}.tensorinfo not found."
				exit 1
			fi
			echo "resolution s${i}" >> ${fnout}
			${CALC} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${TENSORDIR}/normal_s${i}.tensorinfo.flipped >> ${fnout}

			echo "Metrics for TOI : ${fb} in resolution of s${i} ... finished"
		done
	done
}

function metric_cc()
{
	for fb in cc 
	do
		fnout="${fb}_metrics.txt"
		touch ${fnout}
		for ((i=1;i<=5;i++))
		do
			if [ ! -s ${BASE}/s${i}/${fb}/region_s${i}.data ];then
				echo "${BASE}/s${i}/${fb}/region_s${i}.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s1.data ];then
				echo "${SCALEREFDIR}/fa_normal_s1.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s${i}.data ];then
				echo "${SCALEREFDIR}/fa_normal_s${i}.data not found."
				exit 1
			fi
			: '
			scale model geometries in the TOI to its original metric
			'
			${SCALER} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${SCALEREFDIR}/raw_normal_s5.data \
				${SCALEREFDIR}/raw_normal_s${i}.data 1> tmp.data
			mv tmp.data ${BASE}/s${i}/${fb}/region_s${i}.data

			: '
			calculate metrics using prepared tensor information
			'
			if [ ! -s ${TENSORDIR}/normal_s${i}.tensorinfo.flipped ];then
				echo "${TENSORDIR}/normal_s${i}.tensorinfo not found."
				exit 1
			fi
			echo "resolution s${i}" >> ${fnout}
			${CALC} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${TENSORDIR}/normal_s${i}.tensorinfo.flipped >> ${fnout}

			echo "Metrics for TOI : ${fb} in resolution of s${i} ... finished"
		done
	done
}

function metric_ilf()
{
	for fb in ilf
	do
		fnout="${fb}_metrics.txt"
		touch ${fnout}
		for ((i=1;i<=5;i++))
		do
			if [ ! -s ${BASE}/s${i}/${fb}/region_s${i}.data ];then
				echo "${BASE}/s${i}/${fb}/region_s${i}.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s1.data ];then
				echo "${SCALEREFDIR}/fa_normal_s1.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s${i}.data ];then
				echo "${SCALEREFDIR}/fa_normal_s${i}.data not found."
				exit 1
			fi
			: '
			scale model geometries in the TOI to its original metric
			'
			${SCALER} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${SCALEREFDIR}/raw_normal_s1.data \
				${SCALEREFDIR}/raw_normal_s${i}.data 1> tmp.data
			mv tmp.data ${BASE}/s${i}/${fb}/region_s${i}.data

			: '
			calculate metrics using prepared tensor information
			'
			if [ ! -s ${TENSORDIR}/normal_s${i}.tensorinfo.flipped ];then
				echo "${TENSORDIR}/normal_s${i}.tensorinfo not found."
				exit 1
			fi
			echo "resolution s${i}" >> ${fnout}
			${CALC} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${TENSORDIR}/normal_s${i}.tensorinfo.flipped >> ${fnout}

			echo "Metrics for TOI : ${fb} in resolution of s${i} ... finished"
		done
	done
}

function metric_ifo()
{
	for fb in ifo
	do
		fnout="${fb}_metrics.txt"
		touch ${fnout}
		for ((i=1;i<=5;i++))
		do
			if [ ! -s ${BASE}/s${i}/${fb}/region_s${i}.data ];then
				echo "${BASE}/s${i}/${fb}/region_s${i}.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s1.data ];then
				echo "${SCALEREFDIR}/fa_normal_s1.data not found."
				exit 1
			fi

			if [ ! -s ${SCALEREFDIR}/fa_normal_s${i}.data ];then
				echo "${SCALEREFDIR}/fa_normal_s${i}.data not found."
				exit 1
			fi
			: '
			scale model geometries in the TOI to its original metric
			'
			${SCALER} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${SCALEREFDIR}/raw_normal_s5.data \
				${SCALEREFDIR}/raw_normal_s${i}.data 1> tmp.data
			mv tmp.data ${BASE}/s${i}/${fb}/region_s${i}.data

			: '
			calculate metrics using prepared tensor information
			'
			if [ ! -s ${TENSORDIR}/normal_s${i}.tensorinfo.flipped ];then
				echo "${TENSORDIR}/normal_s${i}.tensorinfo not found."
				exit 1
			fi
			echo "resolution s${i}" >> ${fnout}
			${CALC} ${BASE}/s${i}/${fb}/region_s${i}.data \
				${TENSORDIR}/normal_s${i}.tensorinfo.flipped >> ${fnout}

			echo "Metrics for TOI : ${fb} in resolution of s${i} ... finished"
		done
	done
}

for fb in cst cg cc ifo ilf
do
	metric_${fb}
done

echo "Done!"
exit 0
