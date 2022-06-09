#!/bin/bash
edir="$YALNIX_FRAMEWORK/etc"
if [ $# == 4 ] 
then
    $edir/c/collect2 --build-id -m elf_i386 --hash-style=gnu -static -o $1 -u exit -u __brk -u __sbrk -u __mmap -u __default_morecore $edir/m/crt1.o $edir/m/crti.o $edirp/crtbeginT.o -L $2/lib -L$edir/p -L/lib/ -L/usr/lib -T $2/etc/user.x  $3 $4 -lyuser  --start-group -lgcc -lgcc_eh  --end-group $edir/p/crtend.o $edir/m/crtn.o
else if [ $# == 3 ] 
then
    $edir/c/collect2 --build-id -m elf_i386 --hash-style=gnu -static -o $1 -u exit -u __brk -u __sbrk -u __mmap -u __default_morecore $edir/m/crt1.o $edir/m/crti.o $edir/p/crtbeginT.o -L $2/lib -L$edir/p -L/lib/ -L/usr/lib -T $2/etc/user.x  $3 -lyuser  --start-group -lgcc -lgcc_eh  --end-group $edir/p/crtend.o $edir/m/crtn.o
else
    echo "yuserbuild <executable name> <yalnix path> <objname>"
fi
fi
