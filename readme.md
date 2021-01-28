# LSX Unix revival

*LSX is a port of Sixth Edition Unix by Heinz Lycklama to the LSI-11, a cut-down version of the PDP-11 with a microprocessor CPU. It was feared lost as it was never released outside of Bell Labs.*

LSX UNIX has some more info here: https://www.tuhs.org/cgi-bin/utree.pl?file=LSX

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

# Boot process

Booting is called *Bootstraping*. It is a multi stage process. 

Someone (ROM or CPU, I do not know) first loads first sector 
from a boot device. This is the boot sector at
location sector=1, track=1. The code is loaded to memory
address 0x2000.

From documentation I assume the device was usually something 
like one of two floppy drives. These drives had unit numbers 
0 or 1.

Then the boot code (from ROM or CPU) just starts to execute
the code starting at 0x2000. The code can be found in file 
```rxboot.s```. 

rxboot.s content:
```asm
/
/	rxboot.p -- rx11 skeleton bootstrap program.
/	this program takes only one sector.
/	rxboot.p is stored in boot location
/		sector	1
/		track	1
/	pathname bootstrap is located in location
/		** see l_sectr table
/	this program assumes that the bootstrap rom
/	leaves the unit number (0,1) in register r0.
/	this program in turn leaves the unit number
/	(0,1) in register r0 when it jumps to the
/	pathname bootstrap program.
/
/	modified by ljh for new rx layout 2/10/77
rxcs	= 0177170
rxdb	= rxcs+2
go	= 1
empty	= 2
intlev	= 2
rdrx	= 6
unit_1	= 020
done	= 040
treq	= 0200
initrx	= 040000
halt	= 0
nop	= 0240
	.text
	.globl	rxboot,_rxboot
_rxboot:
rxboot:
	nop			/this is required by the rom
	tst	r0		/unit number is in r0 (0,1)
	beq	0f
	bis	$unit_1,readop	/set instruction to read unit 1
0:
	mov	$rxcs,r1	/control and status register
	mov	$rxdb,r2	/data and sector/track addr reg
	mov	$l_sctr, r4	/ set address of secter and track table
next:
	bit	$done,(r1)
	beq	next
	mov	(pc)+,(r1)	/read sector instruction
readop:
	.word	rdrx+go		/can be modified by unit number
1:
	tstb	(r1)		/transfer request flag
	beq	1b
	tstb	(r4)		/ last block ?
	beq	rxboot+0200	/ jump to program if so
	movb	(r4)+, (r2)	/ more to go. setup for next sector
2:
	tstb	(r1)		/transfer request flag
	beq	2b
	movb	(r4)+, (r2)	/ track address
3:
	tstb	(r1)		/read complete ?
	beq	3b
	bmi	erflag		/treq on -- error
	tst	(r1)		/error flag
	bmi	erflag
	mov	bufaddr,r0	/current buffer address
	mov	$empty+go,(r1)	/empty rx function
	br	1f
efloop:
	movb	(r2),(r0)+	/empty buffer
1:
	tstb	(r1)		/transfer request flag
	bmi	efloop		/br if ready
	beq	1b		/wait for flag
	tst	(r1)		/error flag
	bmi	erflag
	mov	r0,bufaddr	/next set of 'empty' locations
	clr	r0		/setup unit number in r0
	cmpb	$rdrx+go,readop	/is it zero
	adc	r0		/id unit number 0,1
	br	next		/read another sector
erflag:
	mov	$initrx,(r1)	/initialize heads -- read first sector
	halt			/error
	br	rxboot		/restart
l_sctr:
				/ table of block addresses for boot prog.
	.byte 4, 1		/ sector 4	track 1
	.byte 7, 1		/ sector 7	track 1
	.byte 10, 1		/ sector 10	track 1
	.byte 21, 0		/ sector 21	track 0
	.byte 24, 0		/ sector 24	track 0
	.word 0			/ end of table
bufaddr:
	.word 0200		/start of pathname bootstrap
	.even

```
The code above loads a second part of the boot code.
This boot code is located in 5 blocks, their addresses can
be found in the 'sector and track table' at the end of the 
code after label ```l_sctr``` (means *load_sectors* maybe). 
Block addresses are (sector:track) 4:1, 7:1, 10:1, 21:0 and 24:0.

The code of the second step can be found in file ```rxboot2.s```:

```asm
/disk boot

core = 12
rootdir = 1

rp = 0
rk = 0
rf = 0
rx = 1
hp = 0

dotdot = [core*2048]-512

inode = dotdot-1024
mode = inode
addr = inode+8
buf = inode+32

reset = 5
large = 010000

start:
	mov	pc,r2
	tst	-(r2)
	mov	$dotdot,sp
	mov	sp,r1
	cmp	pc,r1
	bhis	2f
	reset
1:
	mov	(r2)+,(r1)+
	cmp	r1,$end+dotdot
	blo	1b
	jmp	(sp)

	/output message to tty
2:
	.if	rx
		tst	r0		/ unit number is in r0 (0,1)
		beq	0f
		bis	$020,readop	/ set instruct to read unit 1
0:
	.endif
	mov	$nm+dotdot,r1
1:
	movb	(r1)+,r0
	beq	1f
	jsr	pc,putc
	br	1b

	/get command from tty
1:
	clr	r1
2:
	clr	r0
3:
	movb	r0,(r1)+
4:
	jsr	pc,getc
	cmp	r0,$'\n'
	beq	1f
	cmp	r0,$'@'
	beq	1b
	cmp	r0,$' '
	beq	2b
	cmp	r0,$'#'
	bne	3b
	dec	r1
	bgt	4b
	br	1b

	/put command on stack in exec format
1:
	mov	sp,r3
	clrb	(r1)+
	clrb	(r1)+
	bic	$1,r1
	sub	r1,sp
	clr	r2
1:
	tst	r1
	beq	1f
	movb	-(r1),-(r3)
	beq	1b
2:
	mov	r3,r4
	movb	-(r1),-(r3)
	bne	2b
	mov	r4,-(sp)
	inc	r2
	br	1b
1:
	mov	r2,-(sp)

	/look up command path name
	.if	hp
		mov	$hpcs1,r0
		mov	$040,8(r0)	/drive clear
		mov	$021,(r0)	/preset
		mov	$010000,26(r0)	/fmt22
	.endif

	decb	-(r4)
	mov	$rootdir,in
1:
	jsr	pc,geti
	mov	r4,r3
	mov	$buf+512,r5
2:
	mov	r3,r4
	mov	r5,r0
	add	$16,r5
3:
	cmp	r0,$buf+512
	blo	4f
	jsr	pc,getblk
		br	start
	sub	$512,r5
4:
	cmp	r3,r4
	bne	5f
	mov	(r0)+,in
	beq	2b
5:
	tstb	(r4)+
	beq	1f
	cmpb	(r4),$'/'
	beq	1b
	cmp	r0,r5
	bhis	5b
	cmpb	(r4),(r0)+
	beq	3b
	br	2b
1:
	jsr	pc,geti
	clr	r3
1:
	jsr	pc,getblk
		br	start
	cmp	(r0),$0407
	bne	2f
	add	$020,r0
2:
	mov	(r0)+,(r3)+
	cmp	r0,$buf+512
	blo	2b
	jsr	pc,getblk
		br	1f
	br	2b
1:
	jsr	pc,*$0

geti:
	mov	in,r1
	add	$31,r1
	mov	r1,-(sp)
	asr	r1
	asr	r1
	asr	r1
	asr	r1
	jsr	pc,rblk
	mov	(sp)+,r1
	bic	$0xfff0,r1
	asl	r1
	asl	r1
	asl	r1
	asl	r1
	asl	r1
	add	r0,r1
	mov	$inode,r0
1:
	mov	(r1)+,(r0)+
	cmp	r0,$addr+16
	blo	1b
	clr	r2
	rts	pc

getblk:
	add	$2,(sp)
	mov	r2,r0
	inc	r2
	bit	$large,$mode
	bne	1f
	asl	r0
	mov	addr(r0),r1
	bne	rblk
2:
	sub	$2,(sp)
	clr	r0
	rts	pc
1:
	mov	r0,-(sp)
	clrb	r0
	swab	r0
	asl	r0
	mov	addr(r0),r1
	beq	2b
	jsr	pc,rblk
	asl	(sp)
	bic	$0xfe01,(sp)
	add	(sp)+,r0
	mov	(r0),r1
	beq	2b

rpda = 0176724
rkda = 0177412
rfda = 0177466
rxda = 0177170
rblk:
	.if	rx
		mov	r5,-(sp)
		mov	r4,-(sp)
		mov	r3,-(sp)
		mov	r2,-(sp)
		mov	$rxda,r4
		mov	$rxda+2,r3
		mov	$buf,r2
		asl	r1
		asl	r1
		mov	r1,-(sp)	/ sectr = blockno * 4
		mov	r1,r5
		add	$4,r5		/ blkno*4 + 4
	L1:	bit	$040,(r4)
		beq	L1
		mov	(pc)+,(r4)	/ *ptcs = READ|GO|UNIT
	readop:	.word	7
		mov	(sp),r1		/ now calculate sector,track
					/ sector = (sectr * 3) % 26 + 1
		asl	r1
		add	(sp),r1
		clr	r0
0:
		sub	$26,r1
		bmi	0f
		inc	r0
		br	0b
0:
		add	$27,r1
	L6:	tstb	(r4)		/while(ptcs->lobyte == 0);
		beq	L6
		movb	r1,(r3)		/ ptdb->lobyte = sector
		mov	r0,r1
					/ track = sectr/26 + 1
		clr	r0
0:
		sub	$3,r1
		bmi	0f
		inc	r0
		br	0b
0:
		inc	r0
		cmp	$77, r0		/ if(sector == 77.) sector = 0
		bne	L9
		clr	r0
	L9:	tstb	(r4)		/ while(ptcs->lobyte == 0);
		beq	L9
		movb	r0,(r3)		/ ptcs->lobyte = track
	L11:	tstb	(r4)		/ while(ptcs->lobyte == 0);
		beq	L11
		mov	$3,(r4)		/ *ptcs = EMPTY|GO
		br	L16		/ do {
	L2:	movb	(r3),(r2)+	/	while(ptcs->lobyte < 0)
	L16:	tstb	(r4)		/	    *ptbf++ = ptdb->lobyte
		blt	L2		/ } while(ptcs->lobyte <= 0)
		tstb	(r4)
		ble	L16
		inc	(sp)		/ increment blkno and see if it is
		cmp	(sp),r5		/ less than blkno*4 + 4
		blt	L1		/ yes--loop again.
		tst	(sp)+
		mov	(sp)+,r2
		mov	(sp)+,r3
		mov	(sp)+,r4
		mov	(sp)+,r5
		mov	$buf, r0
		rts	pc
	.endif

    .if	rx-1

	clr	r0

	.if	rp
		div	$10,r0
		mov	r1,-(sp)
		mov	r0,r1
		clr	r0
		div	$20,r0
		bisb	r1,1(sp)
		mov	$rpda,r1
		mov	(sp)+,(r1)
	.endif

	.if	rk
		div	$12,r0
		ash	$4,r0
		add	r1,r0
		mov	$rkda+2,r1
	.endif

	.if	rf
		ashc	$8,r0
		mov	r0,*$rfda+2
		ashc	$16,r0
		mov	$rfda+2,r1
	.endif

hpcs1   = 0176700
hpda	= 0176706
hpdc	= 0176734
	.if	hp
		div	$22,r0
		mov	r1,-(sp)
		mov	r0,r1
		clr	r0
		div	$19,r0
		mov	r0,*$hpdc
		bisb	r1,1(sp)
		mov	(sp)+,r0
		mov	$hpda+2,r1
	.endif

	mov	r0,-(r1)
	mov	$buf,r0
	mov	r0,-(r1)
	mov	$-256,-(r1)

	.if	rf+rk+rp
		mov	$5,-(r1)
	.endif

	.if	hp
		mov	$071,-(r1)
	.endif

1:
	tstb	(r1)
	bge	1b
	rts	pc

    .endif

tks = 0177560
tkb = 0177562
getc:
	mov	$tks,r0
	inc	(r0)
1:
	tstb	(r0)
	bge	1b
	mov	*$tkb,r0
	bic	$0xff80,r0
	cmp	r0,$0101
	blo	1f
	cmp	r0,$0132
	bhi	1f
	add	$040,r0
1:
	cmp	r0,$015
	bne	putc
	mov	$012,r0

tps = 0177564
tpb = 0177566
putc:
	tstb	*$tps
	bge	putc
	cmp	r0,$012
	bne	1f
	mov	$015,r0
	jsr	pc,putc
	mov	$0212,r0
	jsr	pc,putc
	clr	r0
	jsr	pc,putc
	mov	$012,r0
	rts	pc
1:
	mov	r0,*$tpb
	rts	pc

nm:
	.byte	012
	.if	rp
		.string	"rp boot:"
	.endif

	.if	rk
		.string	"rk boot:"
	.endif

	.if	rf
		.string	"rf boot:"
	.endif

	.if  	hp
		.string	"hp boot:"
	.endif

	.if	rx
		.string	"rx boot:"
	.endif
	.even
in:	.word	rootdir
end:

```

# Status
For unknown reason, the original boot disk is not organized as expected.
I could find the boot code content, but at location 6400 octal = 0xd00 = 3328 decimal
```
0006360 047331 025757 145172 147616 114043 117212 032323 065313
0006400 000240 005700 001403 052767 00002   0 000024 012701 177170
0006420 012702 177172 012704 000152 032711 000040 001775 012711
0006440 000007 105711 001776 105714 001453 112412 105711 001776
0006460 112412 105711 001776 100425 005711 100423 016700 000066
0006500 012711 000003 000401 111220 105711 100775 001775 005711
0006520 100410 010067 000040 005000 122767 000007 177702 005500
0006540 000733 012711 040000 000000 000713 000404 000407 000412
0006560 000025 000030 000000 000200 100607 017150 152123 127051
0006600 000000 000000 000000 000000 000000 000000 000000 000000
*
```

So before that boot block, we have 3328 bytes of "something else".
But what is this?
Hm.
