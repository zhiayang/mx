#/bin/bash

if [ `uname` = "Darwin" ]; then
	echo "/Volumes/mx"

elif [ `uname` = "Linux" ]; then
	echo "`pwd`/build/mnt/mx"
fi
