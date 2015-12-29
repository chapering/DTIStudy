#!/bin/bash

BASEDIR=`pwd`
SOURCEDATADIR=${BASEDIR}/curves
DISTANCEMATRIXDIR=${BASEDIR}/distance_matrices
COLOROUTPUTDIR=${BASEDIR}/colors
MATLABLAUNCHER=/usr/bin/matlabLauncher.py
REGIONDIR=./data/
CLUSTERSH=./genClusteringColor.sh
MERGESH=/usr/bin/updateColors.sh
#BUNDLES="cg cc ifo ilf"
BUNDLES="ilf"
RESOLUTIONS="1 2 3 4 5"
SORTER=/home/chap/DTI-SVL/hcai/extract-skeleton/sortstlineByColor.py
CLASSIFIER=/home/chap/DTI-SVL/hcai/extract-skeleton/allstlineMedian.m

if [ ! -s ${MATLABLAUNCHER} ]
then
	echo "${MATLABLAUNCHER} NOT found."
	exit 1
fi

BTEMP=${BASEDIR}/tempbundle

mkdir -p ${BTEMP}

for tid in 1 2 3 4 5 6
do
	for fb in ${BUNDLES}
	do
		for n in ${RESOLUTIONS}
		do
			cp ${REGIONDIR}/region_for_task${tid}/s${n}/${fb}/region_s${n}.data \
				${SOURCEDATADIR}
		done
		### CAD COLORING
		sh ${CLUSTERSH}

		for n in ${RESOLUTIONS}
		do
			### MERGE COLOR AND STREAMLINE GEOMETRIES
			sh ${MERGESH} ${SOURCEDATADIR}/region_s${n}.data \
					${COLOROUTPUTDIR}/region_s${n}1_idist_exact.txt \
					${SOURCEDATADIR}/region_s${n}_withcolor.data

			wait
			### CLUSTERING into BUNDLES
			cd ${BTEMP}
			${SORTER} ${SOURCEDATADIR}/region_s${n}_withcolor.data 1 1

			cd -
			${MATLABLAUNCHER} ${CLASSIFIER} ${BTEMP} 100 \
					${REGIONDIR}/region_for_task${tid}/s${n}/${fb}/skeleton_region_s${n}.data
			echo "------------------------result-------------------------"
			ls -l ${REGIONDIR}/region_for_task${tid}/s${n}/${fb}/skeleton_region_s${n}.data
			echo "-------------------------------------------------------"

			rm -rf ${BTEMP}/*
		done

		#rm -rf ${SOURCEDATADIR}/*.data ${DISTANCEMATRIXDIR}/* ${COLOROUTPUTDIR}/*
	done
done

echo "Done!"
exit 0
