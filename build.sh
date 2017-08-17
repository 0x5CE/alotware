#!/bin/sh
#;---------------------------------------------------------;
#;                    	  Alotware		 	   ;
#;		   Build Script for Linux		   ;
#;							   ;
#;              Written by: Muazzam Ali Kazmi		   ;
#;---------------------------------------------------------;

if test "`whoami`" != "root" ; then
	echo "Log in as a root to do this\nType Sudo bash." && exit
fi

dd bs=512 count=70000 if=/dev/zero of=temp.img || exit

if [ ! -e alotware.img ] ; then
	dd bs=37675008 count=1 if=/dev/zero of=alotware.img || exit
fi	

fasm os/boot/bootsect.asm os/boot/bootsect.bin || exit
fasm os/boot/mbr.asm os/boot/mbr.bin || exit

cd os
fasm alotware.asm ALOTWARE.OS || exit
cd ..
dd status=noxfer conv=notrunc if=os/boot/bootsect.bin of=temp.img  || exit

cd apps/
	
for i in *.asm
do
	fasm $i `basename $i .asm`.bin || exit
done
cd ..

mkdir loopdir && mount -o loop -t vfat temp.img loopdir/ || exit

cp apps/* os/*.inc os/*.asm os/*.OS loopdir/ || exit

sleep 1.0 || exit
umount loopdir || exit
dd status=noxfer conv=notrunc if=temp.img of=alotware.img seek=1  || exit
dd status=noxfer conv=notrunc if=os/boot/mbr.bin of=alotware.img || exit 
rm -rf loopdir temp.img

echo "Done!"
