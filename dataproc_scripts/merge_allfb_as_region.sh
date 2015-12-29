#!/bin/bash

if [ $# -lt 2 ];then
	echo "Usage: $0 <src task data dir> <dest task data dir>"
	exit 1
fi

srcblk=$1
dstblk=$2

FIBERTUBE_MERGER=/usr/bin/mergefibertubes.sh
COPY="ln -s -f"
RELDIR="../../../regions/"
DSTDIR=${dstblk}/regions/

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

		fbreg=${srcreg}/s${i}/cst/region_s${i}.data
		skfbreg=${srcreg}/s${i}/cst/skeleton_region_s${i}.data
		dstfbreg=${DSTDIR}/task1_region_s${i}.data
		dstskfbreg=${DSTDIR}/task1_skeleton_region_s${i}.data

		for fb in cc cg ifo ilf
		do
			if [ -s ${fbreg} ];then
				${FIBERTUBE_MERGER} ${fbreg} ${srcreg}/s${i}/${fb}/region_s${i}.data 1> tmp
				mv tmp ${dstfbreg}
				fbreg=${dstfbreg}
			fi

			: '
			skeleton can also be merged
			'
			if [ -s ${skfbreg} ];then
				${FIBERTUBE_MERGER} ${skfbreg} ${srcreg}/s${i}/${fb}/skeleton_region_s${i}.data 1> tmp
				mv tmp ${dstskfbreg}
				skfbreg=${dstskfbreg}
			fi
		done

		: '
		refer to generated region via softlink to save storage
		'
		for fb in cst cc cg ifo ilf
		do
			cd ${dstreg}/s${i}/${fb}/
			${COPY} ${RELDIR}/task1_region_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/task1_skeleton_region_s${i}.data skeleton_region_s${i}.data
			cd -
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

		fbreg=${srcreg}/s${i}/cst/region_s${i}.data
		skfbreg=${srcreg}/s${i}/cst/skeleton_region_s${i}.data
		dstfbreg=${DSTDIR}/task2_region_s${i}.data
		dstskfbreg=${DSTDIR}/task2_skeleton_region_s${i}.data

		for fb in cc cg ifo ilf
		do
			if [ -s ${fbreg} ];then
				${FIBERTUBE_MERGER} ${fbreg} ${srcreg}/s${i}/${fb}/region_s${i}.data 1> tmp
				mv tmp ${dstfbreg}
				fbreg=${dstfbreg}
			fi

			: '
			skeleton can also be merged
			'
			if [ -s ${skfbreg} ];then
				${FIBERTUBE_MERGER} ${skfbreg} ${srcreg}/s${i}/${fb}/skeleton_region_s${i}.data 1> tmp
				mv tmp ${dstskfbreg}
				skfbreg=${dstskfbreg}
			fi
		done

		: '
		refer to generated region via softlink to save storage
		'
		for fb in cst cc cg ifo ilf
		do
			cd ${dstreg}/s${i}/${fb}/
			${COPY} ${RELDIR}/task2_region_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/task2_skeleton_region_s${i}.data skeleton_region_s${i}.data
			cd -
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

		fbreg=${srcreg}/s${i}/cst/region_s${i}.data
		skfbreg=${srcreg}/s${i}/cst/skeleton_region_s${i}.data
		dstfbreg=${DSTDIR}/task3_region_s${i}.data
		dstskfbreg=${DSTDIR}/task3_skeleton_region_s${i}.data

		for fb in cc cg ifo ilf
		do
			if [ -s ${fbreg} ];then
				${FIBERTUBE_MERGER} ${fbreg} ${srcreg}/s${i}/${fb}/region_s${i}.data 1> tmp
				mv tmp ${dstfbreg}
				fbreg=${dstfbreg}
			fi

			: '
			skeleton can also be merged
			'
			if [ -s ${skfbreg} ];then
				${FIBERTUBE_MERGER} ${skfbreg} ${srcreg}/s${i}/${fb}/skeleton_region_s${i}.data 1> tmp
				mv tmp ${dstskfbreg}
				skfbreg=${dstskfbreg}
			fi
		done

		: '
		refer to generated region via softlink to save storage
		'
		for fb in cst cc cg ifo ilf
		do
			cd ${dstreg}/s${i}/${fb}/
			${COPY} ${RELDIR}/task3_region_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/task3_skeleton_region_s${i}.data skeleton_region_s${i}.data
			cd -
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

		fbreg=${srcreg}/s${i}/cst/region_s${i}.data
		skfbreg=${srcreg}/s${i}/cst/skeleton_region_s${i}.data
		dstfbreg=${DSTDIR}/task5_region_s${i}.data
		dstskfbreg=${DSTDIR}/task5_skeleton_region_s${i}.data

		for fb in cc cg ifo ilf
		do
			if [ -s ${fbreg} ];then
				${FIBERTUBE_MERGER} ${fbreg} ${srcreg}/s${i}/${fb}/region_s${i}.data 1> tmp
				mv tmp ${dstfbreg}
				fbreg=${dstfbreg}
			fi

			: '
			skeleton can also be merged
			'
			if [ -s ${skfbreg} ];then
				${FIBERTUBE_MERGER} ${skfbreg} ${srcreg}/s${i}/${fb}/skeleton_region_s${i}.data 1> tmp
				mv tmp ${dstskfbreg}
				skfbreg=${dstskfbreg}
			fi
		done

		: '
		refer to generated region via softlink to save storage
		'
		for fb in cst cc cg ifo ilf
		do
			cd ${dstreg}/s${i}/${fb}/
			${COPY} ${RELDIR}/task5_region_s${i}.data region_s${i}.data
			${COPY} ${RELDIR}/task5_skeleton_region_s${i}.data skeleton_region_s${i}.data
			cd -
		done # for fb
	done # for i
	return 0
}
for taskid in 2 3 5
do
	transfer_task${taskid}
done


echo "All transfering finished."
exit 0


