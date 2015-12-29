#!/bin/bash

###############################################################################
:<< disclaimer 
this script is to merge the geometry file in the format of "tgdata" produced by
tubegen with a color text file which holds a consistent color for each
corresponding streamline in the tgdata file, produced based on similarity
measurement in terms of inter-fiber distance

Creation: Mar. 25th 2011
Copyright(C) Chapple Studio 2010~2011
disclaimer
###############################################################################
: << usage
the script must be fed with exactly two files, the first one is the tgdata file
and the second the color text file; and the two files should describe equal
number of fibers which are geometrically symbolized as streamlines
usage

# ------------------ check usage in the first place -----------------------
function usage()
{
	echo -e "\nUsage:\n"
	echo "$0 <tgdata file> <color text> [target]"
	echo
	return 0
}

if [ $# -lt 2 ];
then
	echo "too few argument."
	usage
	exit 1
fi

tgdata=$1
colortxt=$2

if [ ! -s $tgdata -o ! -s $colortxt ];
then
	echo "given file not found."
	exit 1
fi

stlinenum=`cat $tgdata | awk '{if(NF==1)print $0}' | wc -l`
colornum=`cat $colortxt | wc -l`

if [ $((stlinenum - 1)) -ne $colornum ];
then
	echo "files provided are incompatible with each other."
	exit 1
fi

outfile=$tgdata.new
if [ $# -ge 3 ];
then
	outfile=$3
fi
>${outfile}

awk -F' ' 'BEGIN{
		i=1
		while ( (getline < "'$colortxt'") > 0 ) {
			colors[i] = $0
			i ++
			OFS=" "
		}
		i=-1
	}
	{
		if (NF != 6 ) {
			print $0
			i ++
			next
		}

		print $1,$2,$3,colors[i]
	}' $tgdata >> ${outfile}

echo "finished."
exit 0




