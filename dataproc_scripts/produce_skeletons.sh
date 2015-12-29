#!/bin/bash

BASEDIR=`pwd`
SOURCEDATADIR=${BASEDIR}/curves
DISTANCEMATRIXDIR=${BASEDIR}/distance_matrices
COLOROUTPUTDIR=${BASEDIR}/colors
MATLABLAUNCHER=/usr/bin/matlabLauncher.py
CLUSTERSH=./genClusteringColor.sh
MERGESH=/usr/bin/updateColors.sh
#BUNDLES="cg cc ifo ilf"
BUNDLES="ilf"
RESOLUTIONS="1 2 3 4 5"
SORTER=/home/chap/DTI-SVL/hcai/extract-skeleton/sortstlineByColor.py
CLASSIFIER=/home/chap/DTI-SVL/hcai/extract-skeleton/allstlineMedian.m
DESTDIR=${BASEDIR}/skeletons

if [ ! -s ${MATLABLAUNCHER} ]
then
	echo "${MATLABLAUNCHER} NOT found."
	exit 1
fi

BTEMP=${BASEDIR}/tempbundle

mkdir -p ${BTEMP} ${DESTDIR}

### CAD COLORING
#sh ${CLUSTERSH}

for tgfn in `ls *.data ${SOURCEDATADIR}`
do
	### MERGE COLOR AND STREAMLINE GEOMETRIES
	sh ${MERGESH} ${SOURCEDATADIR}/${tgfn} \
			${COLOROUTPUTDIR}/${tgfn%%.*}1_idist_exact.txt \
			${SOURCEDATADIR}/${tgfn%%.*}_withcolor.data

	wait

	### CLUSTERING into BUNDLES
	cd ${BTEMP}
	${SORTER} ${SOURCEDATADIR}/${tgfn%%.*}_withcolor.data 1 1

	cd -
	${MATLABLAUNCHER} ${CLASSIFIER} ${BTEMP} 100 \
			${DESTDIR}/skeleton${tgfn%%.*}.data
	echo "------------------------result-------------------------"
	ls -l ${DESTDIR}/skeleton${tgfn%%.*}.data
	echo "-------------------------------------------------------"

	rm -rf ${BTEMP}/*

	#rm -rf ${SOURCEDATADIR}/*.data ${DISTANCEMATRIXDIR}/* ${COLOROUTPUTDIR}/*
done

echo "Done!"
exit 0
