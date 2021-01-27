#!/bin/bash
FSUTIL="../../../fsutil.lsx/lsx-util -v -v -v"
rm -f newroot.dsk
${FSUTIL} -n -s256000 newroot.dsk
${FSUTIL} -a newroot.dsk lsx usr/ tmp/
${FSUTIL} -a newroot.dsk etc/ etc/clri etc/cset etc/init etc/glob etc/mknod etc/mkfs #etc/fsck
${FSUTIL} -a newroot.dsk bin/ bin/sh bin/ls bin/cp bin/date bin/mkdir \
    bin/mv bin/rm bin/rmdir bin/stty bin/od bin/ed  \
    bin/as bin/cc bin/check bin/db bin/ld bin/load bin/ls bin/reloc bin/sh bin/size \
    bin/strip #bin/sync bin/echo bin/cal bin/cat bin/ln bin/wc bin/pwd bin/df bin/mount bin/umount
#${FSUTIL} -a newroot.dsk dev/ dev/tty8!c0:0 dev/fd0!b0:0 dev/fd1!b0:1
${FSUTIL} -c newroot.dsk
${FSUTIL} -v newroot.dsk

rm -rf z
mkdir z
cd z
../$FSUTIL -x ../newroot.dsk

