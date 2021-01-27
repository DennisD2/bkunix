# LSX Unix revival
  This work is heavily based on the BKUnix effort
of Leonid Broukhis and Serge Vakulenko.
Their original readme is [here](old_bkunix/README).

I took their work (mainly cross compile environment)
and started again with the old, original LSX tape/drive images that were found
in Paul Zacharys Garage. These contained most parts of the original
LSX Unix with sourcecode.

# Preparing lsx-util utility
This utility is required to create and manage files containing
disk drives that can be used by LSX UNIX in connection with
SIMH.

Build the tool:
```shell
cd fsutil
make install
```

# Extracting the disks
All files used in this section can be found below 
directory ```0_pauvl_zacharys_garage```.
Original disks are there stored as blobs in ```original-disks```.
Their content can be extracted.
It is required that the tool ```lsx-util``` is built before
trying the extraction process.

Execute script ```extract-disks.sh```
This creates a directory per disk in directory 
```extracted-disks```

```shell
cd 0_pavl_zacharys_garage
./extract-disks.sh
```

After having extracted the raw disks to file systems,
the script is doing its best to rearrange the content of all 
filesystems in a better organized file system.

This new file system is created in projects root directory ```src```.
All builds of the software will start from that directory.

# Checking content of original disks

## root/lib/crt0.o
With the PDP11 disassembler we can check ```crt0.o```
```
pdp11-disasm ./extracted-disks/root/lib/crt0.o
File: ./extracted-disks/root/lib/crt0.o
Type: FMAGIC (0407) relocatable
Section .text: 26 bytes
Section .data: 0 bytes
Section .bss: 2 bytes
Symbol table: 4 names (48 bytes)
Entry address: 02000
```
```
Disassembly of section .text:
002000 170011                   setd
002002 010600                   mov     sp, r0
002004 011046                   mov     (r0), -(sp)
002006 005720                   tst     (r0)+
002010 010066 000002            mov     r0, 2(sp)
002014 004767 177760            jsr     pc, <_main+02000>
002020 022626                   cmp     (sp)+, (sp)+
002022 010016                   mov     r0, (sp)
002024 004737 000000            jsr     pc, *$<_exit>
002030 104401                   sys     1

Disassembly of section .bss:
002032 <.bss>:                  . = .+2
```

The structure of the object file header can be found in 
```extracted-disks/cc2/ld.c```, structure ```filhdr```:

```
struct	filhdr {
	int	fmagic;
	int	tsize;
	int	dsize;
	int	bsize;
	int	ssize;
	int	entry;
	int	pad;
	int	relflg;
} filhdr;
```
So the real code starts with an offset of 16 bytes in the object file.
First bytes are the 'FMAGIC' with value ```000407``` (octal) follows.
In the hex listing, that value occurs as ```f009```.

Dumping the object file in octal mode with ```od``` shows the bytes 
as listed in the disassembler listing.
For example, in the second line of the od listing we can see the bytes 
```170011``` (octal) corresponding to the assembler line ```setd``` 
above. In the hex listing, that value occurs as ```f009```.

```
# OCTAL dump
od -An -o crt0.o 
 000407 000032 000000 000002 000060 000000 000000 000000
 170011 010600 011046 005720 010066 000002 004767 177760
 022626 010016 004737 000000 104401 000000 000000 000000
 000000 000000 000000 000000 000051 000000 000000 000000
 000030 000000 060563 071166 000065 000000 000044 000032
 062537 064570 000164 000000 000040 000000 066537 064541
 000156 000000 000040 000000 072163 071141 000164 000000
 000002 000000

# HEX dump
od -An -x crt0.o 
 0107 001a 0000 0002 0030 0000 0000 0000
 f009 1180 1226 0bd0 1036 0002 09f7 fff0
 2596 100e 09df 0000 8901 0000 0000 0000
 0000 0000 0000 0000 0029 0000 0000 0000
 0018 0000 6173 7276 0035 0000 0024 001a
 655f 6978 0074 0000 0020 0000 6d5f 6961
 006e 0000 0020 0000 7473 7261 0074 0000
 0002 0000
```

## Content of root/lib/liba.a
This contains mathematical functions including floating point routines.

But also some object files get, put, mesg, switch, ttyn, crypt, savr5

```
get.o
put.o
atan.o
hypot.o
mesg.o
switch.o
ttyn.o
rand.o
crypt.o
ecvt.o
pow.o
ldiv.o
dpadd.o
fp.o
gamma.o
floor.o
fmod.o
sin.o
sqrt.o
exp.o
log.o
savr5.o
```

## Content of root/lib/libc.a
Standard C library.

```
abort.o
abs.o
access.o
alarm.o
alloc.o
atof.o
atoi.o
chdir.o
chmod.o
chown.o
ctime.o
dup.o
execl.o
execv.o
fltpr.o
fork.o
fstat.o
getcsw.o
getgid.o
getpw.o
getuid.o
gtty.o
hmul.o
kill.o
ladd.o
ldfps.o
link.o
locv.o
longops.o
ltod.o
makdir.o
mcount.o
mdate.o
mknod.o
mktemp.o
mon.o
mount.o
nice.o
nlist.o
perror.o
pipe.o
printf.o
ptrace.o
putc.o
qsort.o
reset.o
rin.o
setgid.o
setuid.o
signal.o
sleep.o
snstat.o
stime.o
stty.o
sync.o
time.o
times.o
umount.o
unlink.o
wait.o
close.o
creat.o
csv.o
errlst.o
exit.o
ffltpr.o
getc.o
getchr.o
getpid.o
itol.o
lseek.o
nargs.o
open.o
prof.o
putchr.o
read.o
sbrk.o
seek.o
stat.o
write.o
cerror.o
```

# Summary of examination of existing disks

Source for crt0.o is missing. 

Source for libc.a and liba.a is missing.

Most commands seems to exist. Also the complete build
toolchain seems to be there. There is no 'make', but shell scripts 
show how to compile.

## How to add a libc.a

UNIX system calls are called via a trap, an processor exception

See file ```src/sys/trap.c```.

Inside the trap handling routine, system call id 
and its parameters are extracted from stack and 
the system call is called. This crucial line is there:
```c
trap1(callp->call);
```
and the object callp holds information (system call id, parameters)
which is interpreted inside the trap1() trap handler.

See ```sysent.c``` , array ```sysent``` for the sixty-four
system call ids. ```fork``` has e.g. 0 parameters and id ```2```.
```read``` has two parameters and id ```3```.
```
int	sysent[]
{
	0, &nullsys,			/*  0 = indir */
	0, &rexit,			/*  1 = exit */
	0, &fork,			/*  2 = fork */
	2, &read,			/*  3 = read */
	2, &write,			/*  4 = write */
	2, &open,			/*  5 = open */
	0, &close,			/*  6 = close */
	0, &wait,			/*  7 = wait */
	2, &creat,			/*  8 = creat */
	2, &link,			/*  9 = link */
	1, &unlink,			/* 10 = unlink */
	2, &exec,			/* 11 = exec */
	1, &chdir,			/* 12 = chdir */
	0, &gtime,			/* 13 = time */
	3, &mknod,			/* 14 = mknod */
	2, &chmod,			/* 15 = chmod */
	2, &nullsys,			/* 16 = chown */
	1, &sbreak,			/* 17 = break */
	2, &stat,			/* 18 = stat */
	2, &seek,			/* 19 = seek */
	0, &getpid,			/* 20 = getpid */
	3, &nosys,			/* 21 = mount */
	1, &nosys,			/* 22 = umount */
	0, &nullsys,			/* 23 = setuid */
	0, &getuid,			/* 24 = getuid */
	0, &stime,			/* 25 = stime */
	3, &nullsys,			/* 26 = ptrace */
	0, &alarm,			/* 27 = alarm */
	1, &fstat,			/* 28 = fstat */
	0, &pause,			/* 29 = pause */
	1, &nullsys,			/* 30 = smdate; inoperative */
	1, &stty,			/* 31 = stty */
	1, &gtty,			/* 32 = gtty */
	0, &nosys,			/* 33 = x */
	0, &nullsys,			/* 34 = nice */
	0, &nosys,			/* 35 = sleep */
	0, &sync,			/* 36 = sync */
#ifdef BGOPTION
	0, &kill,			/* 37 = kill */
#endif
#ifndef BGOPTION
	1, &nosys,			/* 37 = kill */
#endif
	0, &nosys,			/* 38 = switch */
	0, &nosys,			/* 39 = x */
	0, &nosys,			/* 40 = x */
	0, &dup,			/* 41 = dup */
	0, &nosys,			/* 42 = pipe */
	1, &nullsys,			/* 43 = times */
	4, &nosys,			/* 44 = prof */
	0, &nosys,			/* 45 = tiu */
	0, &nullsys,			/* 46 = setgid */
	0, &nullsys,			/* 47 = getgid */
	2, &ssig,			/* 48 = sig */
	0, &nosys,			/* 49 = x */
	0, &nosys,			/* 50 = x */
	0, &nosys,			/* 51 = x */
	0, &nosys,			/* 52 = x */
	0, &nosys,			/* 53 = x */
	0, &nosys,			/* 54 = x */
	0, &nosys,			/* 55 = x */
	0, &nosys,			/* 56 = x */
	0, &nosys,			/* 57 = x */
	0, &nosys,			/* 58 = x */
	0, &nosys,			/* 59 = x */
	0, &nosys,			/* 60 = x */
	0, &nosys,			/* 61 = x */
	0, &nosys,			/* 62 = x */
#ifdef BGOPTION
	0, &bground			/* 63 = bground */
#endif
#ifndef BGOPTION
	0, &nosys			/* 63 = x */
#endif
};
```

A libc.a implementation must somehow include that
definition to know how to map system calls to the real OS.

These values can be found e.g. in some libc.a implementation
in header file ```syscall.h```:

```
#define SYS_exit	1	/* Terminate the calling process */
#define SYS_fork	2	/* Create a new process */
#define SYS_read	3	/* Read/write file */
#define SYS_write	4
...
```

# Create a new root disk
First step to do is to try to create a new root disk with the 
original files extracted from the original root disk.
This shows that the ```lsx-util``` works as excpected.

Later in the process we could provide additional content, 
like new commands or a changed Kernel and create the root disk.

```
# root disk
u6-fsutil -n -s256000 -bsys/boot1 -Bsys/boot2lo root.dsk
cp sys/lsx .
u6-fsutil -a root.dsk lsx usr/ tmp/
rm -f lsx
u6-fsutil -a root.dsk etc/ etc/init etc/glob etc/mknod etc/mkfs etc/fsck
u6-fsutil -a root.dsk bin/ bin/sh bin/ls bin/echo bin/cal bin/cp bin/date bin/mkdir bin/sync bin/mv bin/rm bin/rmdir bin/stty bin/od bin/ed bin/cat bin/ln bin/wc bin/pwd bin/df bin/mount bin/umount
u6-fsutil -a root.dsk dev/ dev/tty8!c0:0 dev/fd0!b0:0 dev/fd1!b0:1
u6-fsutil -c root.dsk

# usr disk
u6-fsutil -n -s256000 usr.dsk
u6-fsutil -c usr.dsk
u6-fsutil -v root.dsk

```