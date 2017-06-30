#!/bin/bash
pid=0
id=0
echo "Checking to see if msfrpcd is running ..."
while [ "${pid:-0}" == 0 ]; do
	pid=$(netstat -tpln | grep 0\.0\.0\.0:55553 | awk '{print $7}' | sed 's#/.*##')
	if [ "${pid:-0}" -ne 0 ];
		then
		printf "%s\n" "$pid"
         	echo "The Service was identified in $i seconds."
         	sleep 3
         	python /root/msf-remote-console/Main.py -e -c -r  /root/msf-remote-console/resources/autopwn.rc
	fi
	if [ "$i" == 120 ];
		then
		echo "Cannot determine if service came up."
		break
	fi
	let i=i+1
	sleep 1
done