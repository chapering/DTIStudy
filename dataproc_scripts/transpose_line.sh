#!/bin/bash

## transpose rows orderly, the input file should consist of lines containing 
## equal number of columns, otherwise the result will be distorted

infile=${1:-" "}
delim=${2:-" "}

maxr=1000
maxc=100

if [ ! $infile = ' ' ]
then
	maxr=`wc -l $infile | cut -d' ' -f 1`
	maxc=`wc -L $infile | cut -d' ' -f 1`
	#printf "Totally %d lines, the longest line has %d columns\n" $maxr $maxc
fi

cat ${infile} | \
awk -F ["$delim"] ' \
	BEGIN{
		for( i = 1; i <= '$maxr'; ++i ) {
			for( j = 1; j <= '$maxc'; ++j ){
				content[i, j] = " ";
			}
			colnum[i] = 0;
		}
		rcnt = 1;
		tc = 0;
	}

	{
		for( i = 1; i <= NF; i++ ) {
			content[rcnt, i] = $i;
		}

		colnum[rcnt] = NF;
		if (NF > tc) {
			tc = NF;
		}

		rcnt ++;
	} 

	END{
		#print "maximal words = " tc
		for( j = 1; j <= tc; ++j ){
			for( i = 1; i <= '$maxr'; ++i ) {
				if ( j <= colnum[i] ) {
					printf("%s", content[i, j]);
				}
				else {
					printf("%s", " ");
				}
						
				if (i < '$maxr') {
					printf("%s", " ");
				}
			}
			printf("\n");
		}
	}'

#echo "tranposition finished."
exit 0



