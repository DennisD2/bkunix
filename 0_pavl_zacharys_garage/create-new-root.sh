#!/bin/bash
BASEDIR=$HOME/src/lsxunix
FSUTIL="$BASEDIR/fsutil/lsx-util"
#FSUTIL="$BASEDIR/fsutil/lsx-util -v -v -v"

cd extracted-disks/root

#OLD ${FSUTIL} -n -s256000 -bsys/boot1 -Bsys/boot2lo root.dsk

rm -f newroot.dsk
cmd="${FSUTIL} -n -S --size=256000 --boot=../new_rxboot --boot2=../new_rxboot2 newroot.dsk"
echo "Executing: $cmd"
$cmd

# Prepare sec. boot sec
dd bs=1 skip=16 if=../new_rxboot2 of=boot2
cmd="${FSUTIL} -a newroot.dsk boot2"
echo "Executing: $cmd"
$cmd

${FSUTIL} -a newroot.dsk lsx usr/ tmp/
${FSUTIL} -a newroot.dsk etc/ etc/clri etc/cset etc/init etc/glob etc/mknod etc/mkfs #etc/fsck
${FSUTIL} -a newroot.dsk bin/ bin/sh bin/ls bin/cp bin/date bin/mkdir \
    bin/mv bin/rm bin/rmdir bin/stty bin/od bin/ed \
    bin/as bin/cc bin/check bin/db bin/ld bin/load bin/ls bin/reloc bin/sh bin/size \
    bin/strip #bin/sync bin/echo bin/cal bin/cat bin/ln bin/wc bin/pwd bin/df bin/mount bin/umount
${FSUTIL} -a newroot.dsk lib/ lib/as2 lib/c0 lib/c1 lib/c2 lib/crt0.o lib/liba.a lib/libc.a
# OLD ${FSUTIL} -a newroot.dsk dev/ dev/tty8!c0:0 dev/fd0!b0:0 dev/fd1!b0:1
${FSUTIL} -c newroot.dsk
${FSUTIL} -v newroot.dsk
od -o newroot.dsk >newroot.odo

echo "Please do not forget to fix block numbers of secondary boot sectors in primary sector table!"
exit 0

rm -rf z
#mkdir -p z
#cd z
#$FSUTIL -x ../newroot.dsk

