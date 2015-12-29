#!/bin/bash
SRC=./webdata
DST=./data_on_site
mkdir -p ${DST}/bycls ${DST}/byres

function breakdown()
{
	#for cls in normal_whole normal_allfb normal_partial
	for cls in normal_allfb 
	do
		for res in 1 2 3 4 5
		do
			for tid in 1 2 3 4 5 6
			do
				mkdir -p ${DST}/${cls}/res${res}
				tar zcvf ${DST}/${cls}/res${res}/task${tid}.tar.gz \
					${SRC}/${cls}/region_for_task${tid}/s${res} \
					--dereference	
			done
		done

		: '
		by category 
		'
		tar zcvf ${DST}/bycls/${cls}.tar.gz ${SRC}/${cls} \
					--dereference	
	done
}

function byres()
{
	: '
	by resolution
	'
	for res in 1 2 3 4 5
	do
		tar zcvf ${DST}/byres/res${res}.tar.gz ${SRC}/normal_{whole,allfb,partial}/*/s${res} \
					--dereference	
	done
}

function all()
{
	: '
	all data in one package
	'
	tar zcvf ${DST}/alldata.tar.gz ${SRC} --dereference
}

breakdown

byres

echo "all done."

