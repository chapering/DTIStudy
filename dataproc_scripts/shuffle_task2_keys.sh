#!/bin/bash

base=`pwd`
RANDOM=$$
for fb in cst cg cc ifo ilf
do
	for i in 1 2 3 4 5
	do
		j=0
		while ((1))
		do
			j=`expr $RANDOM % 3`
			if [ $j -ne 0 ];then
				break;
			fi
		done

		/usr/bin/exchange_file.sh \
			${base}/s${i}/${fb}/tumorbox_0_region_s${i}.data \
			${base}/s${i}/${fb}/tumorbox_${j}_region_s${i}.data

		/usr/bin/exchange_file.sh \
			${base}/s${i}/${fb}/pos1/tumorbox_0_region_s${i}.data \
			${base}/s${i}/${fb}/pos1/tumorbox_${j}_region_s${i}.data
	done
done

echo "Done!"
exit 0
