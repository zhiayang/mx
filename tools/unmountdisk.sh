#/bin/bash

if [ `uname` = "Darwin" ]; then
	hdiutil detach -quiet /Volumes/mx

elif [ `uname` = "Linux" ]; then
	umount ../build/mnt
fi
