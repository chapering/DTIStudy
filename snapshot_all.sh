#!/bin/bash

for cls in normal_whole normal_allfb normal_partial
do
	for pos in pos0 pos1
	do
		sh ./goover_snapshots.sh $cls $pos
	done
done

exit 0
