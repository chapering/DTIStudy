#!/bin/bash

COPY="cp -f"

function dispatch_region_for_task1()
{
	for ((i=1;i<=5;i++));do ${COPY} raw_righths_s${i}.data ../region_for_task1/s${i}/cst/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonraw_righths_s${i}.data ../region_for_task1/s${i}/cst/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} raw_lefths_s${i}.data ../region_for_task1/s${i}/cst/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonraw_lefths_s${i}.data ../region_for_task1/s${i}/cst/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} righths_s${i}.data ../region_for_task1/s${i}/cg/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task1/s${i}/cg/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} righths_s${i}.data ../region_for_task1/s${i}/cg/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task1/s${i}/cg/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_normal_s${i}.data ../region_for_task1/s${i}/cc/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} normalSkeleton_s${i}.data ../region_for_task1/s${i}/cc/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_normal_s${i}.data ../region_for_task1/s${i}/cc/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} normalSkeleton_s${i}.data ../region_for_task1/s${i}/cc/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} lefths_s${i}.data ../region_for_task1/s${i}/ifo/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonlefths_s${i}.data ../region_for_task1/s${i}/ifo/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} lefths_s${i}.data ../region_for_task1/s${i}/ifo/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonlefths_s${i}.data ../region_for_task1/s${i}/ifo/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} righths_s${i}.data ../region_for_task1/s${i}/ilf/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task1/s${i}/ilf/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} righths_s${i}.data ../region_for_task1/s${i}/ilf/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task1/s${i}/ilf/pos1/skeleton_region_s${i}.data; done

	return 0
}

function dispatch_region_for_task2()
{
	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_lefths_s${i}.data ../region_for_task2/s${i}/cst/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonlefths_s${i}.data ../region_for_task2/s${i}/cst/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_righths_s${i}.data ../region_for_task2/s${i}/cst/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task2/s${i}/cst/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_lefths_s${i}.data ../region_for_task2/s${i}/cg/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonlefths_s${i}.data ../region_for_task2/s${i}/cg/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_righths_s${i}.data ../region_for_task2/s${i}/cg/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task2/s${i}/cg/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_normal_s${i}.data ../region_for_task2/s${i}/cc/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} normalSkeleton_s${i}.data ../region_for_task2/s${i}/cc/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_normal_s${i}.data ../region_for_task2/s${i}/cc/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} normalSkeleton_s${i}.data ../region_for_task2/s${i}/cc/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_lefths_s${i}.data ../region_for_task2/s${i}/ifo/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonlefths_s${i}.data ../region_for_task2/s${i}/ifo/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_lefths_s${i}.data ../region_for_task2/s${i}/ifo/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonlefths_s${i}.data ../region_for_task2/s${i}/ifo/pos1/skeleton_region_s${i}.data; done

	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_righths_s${i}.data ../region_for_task2/s${i}/ilf/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task2/s${i}/ilf/skeleton_region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} sync1_fa_righths_s${i}.data ../region_for_task2/s${i}/ilf/pos1/region_s${i}.data; done
	for ((i=1;i<=5;i++));do ${COPY} skeletonrighths_s${i}.data ../region_for_task2/s${i}/ilf/pos1/skeleton_region_s${i}.data; done

	return 0
}

############################################################################################################################################
############################################################################################################################################
for i in 1 2
do
	dispatch_region_for_task${i}
done


echo "done!"
exit 0


