#/bin/bash

if [ `uname` = "Darwin" ]; then
	test -d /Volumes/mx || hdiutil attach -quiet build/disk.img
elif [ `uname` = "Linux" ]; then
	mkdir ../build/mnt/mx
	mount ../build/disk.img ../build/mnt/mx
fi
