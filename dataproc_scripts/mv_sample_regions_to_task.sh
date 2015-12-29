#!/bin/bash

for i in 2 3 4 5
do
	for fb in cst cg ilf
	do
		: '
		./singleitr -f \
		/home/chap/DTI-SVL/onSeedingStudy/density_study/data//normal/region_for_task6/s${i}/${fb}/region_s${i}.data \
		-i \
		/home/chap/DTI-SVL/onSeedingStudy/density_study/data/normal/region_for_task6/s${i}/${fb}/fiberidx_nocut_s${i}.data 
		'

		mv \
		/home/chap/DTI-SVL/onSeedingStudy/density_study/data/normal/region_for_task6/s${i}/${fb}/region_13* \
	  	/home/chap/DTI-SVL/onSeedingStudy/density_study/data/normal/region_for_task7/s${i}/${fb}/region_s${i}.data
	done
done
