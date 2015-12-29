#!/bin/bash

(( $# < 1 )) && exit 1

for each in $@
do
	echo -n -e "${each}:\t"

	total=`cat ${each} | grep -a -c "Answer"` 

	cat ${each} | \
		grep -E "Answer|TASK" | \
			awk '{if ($NF=="(correct).") print $0}'  | \
				wc -l | awk '{print 100*$0/('$total'*1.0)"%"}'
done

exit 0
