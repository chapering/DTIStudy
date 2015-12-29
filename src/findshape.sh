#!/bin/bash

if [ $# -lt 2 ];
then
	echo "too few argument."
	exit 1
fi

tensordata=$1
shape=$2

if [ ! -s $tensordata ];
then
	echo "given file not found."
	exit 1
fi

stlinenum=`cat $tensordata | awk '{if(NR==1 && NF==1)print $0}'`

gawk -F' ' 'BEGIN{
		r=-2
		v=0
	}
	{
		if (NF == 1 ) {
			r ++
			v=0
			next
		}

		out=0
		if ('$shape' == 1 ) {
			v1= $5-$6
			v2 = $6-$7
			v1=v1<0?-v1:v1;
			v2=v2<0?-v2:v2;
			if ( v1 <= 0.0001 && v2 <= 0.0001 ) {
				out=1
			}
		}
		else if ('$shape' == 2 ) {
			all[1] = $5
			all[2] = $6
			all[3] = $7
			asort(all)
			#print all[1],all[2],all[3]
			if ( all[3] > all[1]+all[2] && all[2] - all[1] <= 0.0001 ) {
				out=1
			}
		}
		else if ('$shape' == 3 ) {
			all[1] = $5
			all[2] = $6
			all[3] = $7
			asort(all)
			if ( all[3]-all[2] <= 0.001 && all[2]-all[1] >= 0.001 ) {
				out=1
			}
		}

		if (out == 1) {
			print r,v,$5,$6,$7
		}

		v++

	}' $tensordata

echo "finished."
exit 0




