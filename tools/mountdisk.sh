#/bin/bash


if [ `uname` = "Darwin" ]; then
	test -d /Volumes/mx || hdiutil attach -quiet build/disk.img
elif [ `uname` = "Linux" ]; then
	mount -o loop,offset=1048576 build/disk.img build/mnt/mx
fi
