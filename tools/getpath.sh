#/bin/bash

if [ `uname` = "Darwin" ]; then
	echo "/Volumes/mx"

elif [ `uname` = "Linux" ]; then
	echo "build/mnt/mx"
fi
