#!/bin/bash

if [[ ! -f /media/fat/alloy.txt ]]; then
	echo "/media/fat appears to not be mounted."
	exit
fi

cp userspace/user.exe /media/fat/user.exe
sync
