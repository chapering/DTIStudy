#!/bin/bash

BASEDIR=`pwd`
IDISTBIN=${BASEDIR}/src/distmat/idist
EMBEDDING_SCRIPT=${BASEDIR}/src/coloring/embed_distance_batch.m
SOURCEDATADIR=${BASEDIR}/curves
DISTANCEMATRIXDIR=${BASEDIR}/distance_matrices
COLOROUTPUTDIR=${BASEDIR}/colors
MATLABLAUNCHER=/usr/bin/matlabLauncher.py

if [ ! -x ${IDISTBIN} ]
then
	echo "${IDISTBIN} NOT found."
	exit 1
fi

if [ ! -s ${EMBEDDING_SCRIPT} ]
then
	echo "${EMBEDDING_SCRIPT} NOT found."
	exit 1
fi

if [ ! -s ${MATLABLAUNCHER} ]
then
	echo "${MATLABLAUNCHER} NOT found."
	exit 1
fi

pushd . >/dev/null
cd ${SOURCEDATADIR}
for tgfn in `ls *.data`
do
	: ' 
	1st step: generate distance matrix
	'
	echo "producing distance matrix in ${tgfn%%.*}1_idist_exact.txt ..."
	${IDISTBIN} ${SOURCEDATADIR}/${tgfn} \
				${DISTANCEMATRIXDIR}/${tgfn%%.*}1_idist_exact.txt

	wait
	: '
	2nd step: embedding color
	'
	echo "embedding color for ${tgfn} ..."
	python ${MATLABLAUNCHER} ${EMBEDDING_SCRIPT} ${tgfn%%.*} 1 \
		idist exact ${BASEDIR}
done
popd > /dev/null

exit 0
