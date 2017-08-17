#!/bin/sh
#;---------------------------------------------------------;
#;                    	  Alotware		 	   ;
#;             Written by: Muazzam Ali Kazmi		   ;
#;                                                         ;
#;Script for Linux to install Alotware on USB or hard disk.;
#; 	        Replace 'sdb' with the device	   	   ;
#;---------------------------------------------------------;


if test "`whoami`" != "root" ; then
	echo "Log in as a root to do this"
	echo "Type Sudo bash."
	exit
fi

dd if=alotware.img of=/dev/sdb  bs=37675008 count=1 || exit

echo "Installed!"
