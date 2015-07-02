#/bin/bash

if [ `uname` = "Darwin" ]; then
	# hdiutil unmount -force -quiet -timeout 1 /Volumes/mx
	echo

elif [ `uname` = "Linux" ]; then
	umount build/mnt/mx
fi
