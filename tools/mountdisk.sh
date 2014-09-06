#/bin/bash

if [ `uname` = "Darwin" ]; then
	test -d /Volumes/mx || hdiutil attach -quiet build/disk.img
elif [ `uname` = "Linux" ]; then
	kpartx -a -v build/disk.img
	mount -o loop /dev/mapper/loop0p1 build/mnt/mx
fi
