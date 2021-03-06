#!/bin/bash


DISKSIZE=100
echo "=> Creating disk image of $DISKSIZE mb"
# use 'dd' to create.
BLOCKSIZE=$(($DISKSIZE * 1024 * 1024 / 512))
DISKNAME=build/disk.img

# create the image first
dd if=/dev/zero of=$DISKNAME bs=512 count=$BLOCKSIZE &> /dev/null

# create a quick grub floppy.
dd if=grub/stage1 of=grub.img count=1 bs=512 &> /dev/null
dd if=grub/stage2 of=grub.img seek=1 bs=512 &> /dev/null

if [ `uname` = "Darwin" ]; then
	# if not on mac, life will be easier.
	# run fdisk on it
	echo -e "y\nedit 1\n0B\nn\n2047\n\nflag 1\nwrite\nexit\n" | fdisk -e $DISKNAME > /dev/null

	# separate the things.
	dd if=$DISKNAME of=build/mbr.img bs=512 count=2047 &> /dev/null
	dd if=$DISKNAME of=build/fs.img bs=512 skip=2047 &> /dev/null

	# EDIT HERE FOR NON MAC
	# mount the image
	IDENTIFER=`hdiutil attach -nomount build/fs.img`

	# make the vfat image.
	newfs_msdos -v "MX" $IDENTIFER &> /dev/null

	# detach
	hdiutil detach $IDENTIFER

	# recombine
	cat build/mbr.img build/fs.img > $DISKNAME

	# mount the entire image
	hdiutil attach $DISKNAME

	# copy over grub files
	mkdir -p /Volumes/MX/boot/grub
	mkdir -p /Volumes/MX/System/Library/LaunchDaemons
	mkdir -p /Volumes/MX/System/Library/CoreServices
	cp grub/* /Volumes/MX/boot/grub/

	hdiutil detach -quiet $IDENTIFER



elif [ `uname` = "Linux" ]; then
	echo -e "n\np\n1\n2048\n\nt\n0B\na\nw\n" | fdisk $DISKNAME > /dev/null
	dd if=$DISKNAME of=build/mbr.img bs=512 count=2048 &> /dev/null
	dd if=$DISKNAME of=build/fs.img bs=512 skip=2048 &> /dev/null

	mkdir -p mnt/disk
	mkfs.vfat build/fs.img
	sudo mount build/fs.img mnt/disk

	mkdir -p mnt/disk/boot/grub
	mkdir -p mnt/disk/System/Library/LaunchDaemons
	mkdir -p mnt/disk/System/Library/CoreServices

	cp grub/* mnt/disk/boot/grub/

	sudo umount mnt/disk
	cat build/mbr.img build/fs.img > $DISKNAME

	mkdir -p build/mnt/mx
fi


# give the user instructions
echo "=> Installing GRUB"
echo "You should see a shell grub>"
echo "Enter these commands:"
echo "root (hd0,0)"
echo -e "setup (hd0)\n\n"
echo "Press Command-Q to close QEMU once you're done."
qemu-system-x86_64 -boot a -fda grub.img -hda $DISKNAME

echo "=> GRUB setup complete."


echo "=> Cleaning up"
rm grub.img
rm build/fs.img
rm build/mbr.img

if [ `uname` = "Linux" ]; then
	rm -r mnt
fi


echo "=> Ready!"
exit
