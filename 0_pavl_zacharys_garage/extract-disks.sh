#!/bin/bash
# mv or cp, depending on what we want to keep in original disks area
CPCMD=mv

# src directory where all extracted and rearranged files will be copied to
SRC_ROOT=../../../src

# lsx-util must be installed into ../bin, by executing make install in directory fsutil
FSUTIL_EXE=`pwd`/../fsutil/lsx-util

rm -rf extracted-disks
mkdir extracted-disks

# Delete src/* only of you have not edited it manually
#rm -rf src/{devel,etc,sys,bin}

for i in `ls original-disks/*.dsk`; do
    disk=`basename $i .dsk`
    echo "processing $disk ..."
    mkdir -p extracted-disks/$disk
    cd extracted-disks/$disk
    $FSUTIL_EXE -x ../../original-disks/$disk.dsk
    cd ../..
done


# Set up cc
mkdir -p $SRC_ROOT/devel/cc
cd extracted-disks/cc
$CPCMD c0h.c c0[0-5].c c0t.s $SRC_ROOT/devel/cc
$CPCMD c1h.c c1[0-3].c c1t.s table.s table.i $SRC_ROOT/devel/cc
$CPCMD c2h.c c2[0-1].c $SRC_ROOT/devel/cc
$CPCMD cvopt.c script shc shc[0-2] shlsx shlsx5 $SRC_ROOT/devel/cc
cd ../..
cd extracted-disks/cc2
$CPCMD cc.c $SRC_ROOT/devel/cc
cd ../..
rm -f extracted-disks/srcam/cc.c

#pdp11-pcc -c -O c0[0-5].c
#pdp11-asm c0t.s;mv a.out c0t.o
#pdp11-pcc -n c0?.o
#
#pdp11-pcc -c -O c1[0-3].c
#pdp11-asm c1t.s;mv a.out c1t.o
# ---> TODO cvopt table.s table.i
#pdp11-asm table.i;mv a.out table.o
#pdp11-pcc -n c1?.o table.o
#
#cc -c -O c20.c c21.c
#cc -n c2?.o
#

###cc -r -d -O c0?.o
###reloc a.out 40000
###strip a.out
###mv a.out ../rfs/lib/c0
###cc -r -d -O c1?.o table.o
###reloc a.out 40000
###strip a.out
###mv a.out ../rfs/lib/c1
###cc -r -d -O c2?.o
###reloc a.out 40000
###strip a.out
###mv a.out ../rfs/lib/c2

###cc -r -d -O c0?.o
###reloc a.out 50000
###strip a.out
###mv a.out ../rfs/lib/c0.5
###cc -r -d -O c1?.o table.o
###reloc a.out 50000
###strip a.out
###mv a.out ../rfs/lib/c1.5
###cc -r -d -O c2?.o
###reloc a.out 50000
###strip a.out
###mv a.out ../rfs/lib/c2.5


# Set up sys
mkdir -p $SRC_ROOT/sys/dev
cd extracted-disks/usr
$CPCMD alloc.c bio.c buf.h clock.c file.h filsys.h fio.c iget.c inode.h low.s main.c mch.s \
 nami.c param.h proc.h rdwri.c reg.h sh* sig.c slp.c subr.c sys[1-4].c sysent.c systm.h trap.c tty.h user.h \
  $SRC_ROOT/sys
$CPCMD header.dec.s header.lsx.s header.rf.s header.s $SRC_ROOT/sys
$CPCMD emul.s $SRC_ROOT/sys
$CPCMD aedfd.c decfd.c kl.c perfd.c rffd.c sykfd.c tv.c screen.h $SRC_ROOT/sys/dev
cd ../..
# TODO file in bkunix, but not found in extracted disk: boot.S


# Set up etc
mkdir -p $SRC_ROOT/etc
cd extracted-disks/srcam
$CPCMD glob.c init.c mkfs.c mknod.c $SRC_ROOT/etc
cd ../..
# TODO file in bkunix, but not found in extracted disk: fsck.c
# glob.c init.c mkfs.c mknod.c can also be found in src/cc2, but they are equal !
rm -f extracted-disks/cc2/glob.c extracted-disks/cc2/init.c extracted-disks/cc2/mkfs.c \
  extracted-disks/cc2/mknod.c

# Set up as
mkdir -p $SRC_ROOT/devel/as
cd extracted-disks/cc2
$CPCMD as*.s $SRC_ROOT/devel/as
cd ../..
# TODO file in bkunix, but not found in extracted disk: as.h
# all as.* files are duplicated in srcam
rm -f extracted-disks/srcam/as*.s

# Set up db
mkdir -p $SRC_ROOT/devel/db
cd extracted-disks/cc2
$CPCMD db[1-4].s dbinstruct $SRC_ROOT/devel/db
cd ../..
# all db.* files are duplicated in srcam
rm -f extracted-disks/srcam/db[1-4].s extracted-disks/srcam/dbinstruct

# Set up ar, ld, reloc
cd extracted-disks/cc2
$CPCMD ar.c ld.c $SRC_ROOT/devel
cd ../..
cd extracted-disks/srcnz
$CPCMD reloc.c $SRC_ROOT/devel
cd ../..
rm -f extracted-disks/srcam/ar.c extracted-disks/srcam/ld.c

# Set up bin
mkdir -p $SRC_ROOT/bin
cd extracted-disks/srcam
$CPCMD cp.c date.c ed.c ls.c mkdir.s mv.c $SRC_ROOT/bin
# in bkunix, but missing: cal.c cat.c charcode.c clock.c df.c echo.c \
#   halt.c hello.c ln.c mount.c pwd.c testarith.c testprintf.c umount.c wc.c
cd ../..
# parts from cc2 directory
rm -f extracted-disks/cc2/cp.c extracted-disks/cc2/date.c extracted-disks/cc2/ed.c \
  extracted-disks/cc2/ed.c extracted-disks/cc2/ls.c extracted-disks/cc2/mkdir.s extracted-disks/cc2/mv.c
# parts from srcnz directory
cd extracted-disks/srcnz
$CPCMD od.c sh.c shinstruct rm.c rmdir.s stty.c sync.c size.c strip.s $SRC_ROOT/bin
cd ../..

