#!/bin/bash

taskid=${1:-3}
np=${2:-5}
ncol=${3:-5}
maxmode=2
[ "$taskid" = 6 ] && \
	maxmode=3
case4task3=(touch nontouch)
case4task5=(yes no)
case4task6=(nocut leftcut rightcut)

for ((i=1;i<=$np;++i))
do
	RES=()
	RESV=()
	for ((j=0;j<$ncol;++j))
	do
		curv=`expr $RANDOM % $maxmode`

		if test "$taskid" -eq 6 -a "$j" -ge 3;
		then
			case $j in
				3)
					while [ $curv -eq ${RESV[0]} ];
					do
						curv=`expr $RANDOM % $maxmode`
					done	
					;;
				4)
					while [ $curv -eq ${RESV[2]} ];
					do
						curv=`expr $RANDOM % $maxmode`
					done	
					;;
				*)
			esac
		fi

		RESV[$j]=$curv

		case $taskid in
			3)
				RES[$j]=${case4task3[$curv]}
				;;
			5)
				RES[$j]=${case4task5[$curv]}
				;;
			6)
				RES[$j]=${case4task6[$curv]}
				;;
			*)
				echo "no policy for handling this task."
				exit 1
				;;
		esac
	done

	case $taskid in
		3)
			echo -n "local binchoices$i=("
			;;
		5)
			echo -n "local binchoices$i=("
			;;
		6)
			echo -n "local trichoices$i=("
			;;
		*)
	esac

	size=${#RES[*]}
	let "lsize = size - 1"
	for ((k=0;k<$size;k++));
	do
		echo -n ${RES[$k]}

		[ $k -lt $lsize ] && \
			echo -n " "
	done
	echo ")"
done

exit 0

