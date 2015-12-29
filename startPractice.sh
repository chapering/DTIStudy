pid=0
for ((k=1;k<=5;k++))
do
	if [ ! -s studylog_p${k} ];then
		pid=$k
		break
	fi
done

if [ $pid -eq 0 ];then
	gid=0
	for ((j=1;j<=5;j++))
	do
		if [ ! -d group${j} ];then
			gid=$j
			break
		fi
	done
	if [ $gid -eq 0 ];then
		echo "Failed to continue, quota depleted now!"
		exit 1
	fi
	mkdir -p group${gid}
	mv studylog_p* group${gid}
	pid=1
fi

sh runstudy.sh taskorder $pid 1>/dev/null 2>&1

#rm -f studylog_p${pid}

exit 0
