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

In the fsutil directory, there is an object file ```rxboot``` file already compiled, content is identical (?):
```asm
dennis@dennis-pc:~/src/lsxunix/fsutil> pdp11-disasm rxboot   
         File: rxboot
         Type: FMAGIC (0407) non-relocatable
Section .text: 120 bytes
Section .data: 0 bytes
 Section .bss: 0 bytes
 Symbol table: 0 names (0 bytes)
Entry address: 0

Disassembly of section .text:
000000 <.text>:
     0: 000240                  nop
     2: 005700                  tst     r0
     4: 001403                  beq     12 <.text+014>
     6: 052767 000020 000024    bis     $16, $040 <.text+040>
    14: 012701 177170           mov     $-392, r1
    20: 012702 177172           mov     $-390, r2
    24: 012704 000152           mov     $106, r4
    30: 032711 000040           bit     $32, (r1)
    34: 001775                  beq     030 <.text+030>
    36: 012711 000007           mov     $7, (r1)
    42: 105711                  tstb    (r1)
    44: 001776                  beq     042 <.text+042>
    46: 105714                  tstb    (r4)
    50: 001453                  beq     0200 <.bss+010>
    52: 112412                  movb    (r4)+, (r2)
    54: 105711                  tstb    (r1)
    56: 001776                  beq     054 <.text+054>
    60: 112412                  movb    (r4)+, (r2)
    62: 105711                  tstb    (r1)
    64: 001776                  beq     062 <.text+062>
    66: 100425                  bmi     0142 <.text+0142>
    70: 005711                  tst     (r1)
    72: 100423                  bmi     0142 <.text+0142>
    74: 016700 000066           mov     $0164 <.text+0164>, r0
   100: 012711 000003           mov     $3, (r1)
   104: 000401                  br      0110 <.text+0110>
   106: 111220                  movb    (r2), (r0)+
   110: 105711                  tstb    (r1)
   112: 100775                  bmi     0106 <.text+0106>
   114: 001775                  beq     0110 <.text+0110>
   116: 005711                  tst     (r1)
   120: 100410                  bmi     0142 <.text+0142>
   122: 010067 000040           mov     r0, $0166 <.text+0166>
   126: 005000                  clr     r0
   130: 122767 000007 177702    cmpb    $7, $040 <.text+040>
   136: 005500                  adc     r0
   140: 000733                  br      030 <.text+030>
   142: 012711 040000           mov     $16384, (r1)
   146: 000000                  halt
   150: 000713                  br      0 <.text>
   152: 000404                  br      0164 <.text+0164>
   154: 000407                  br      0174 <.bss+4>
   156: 000412                  br      0204 <.bss+014>
   160: 000025                  .word   21
   162: 000030                  .word   24
   164: 000000                  halt
   166: 000200                  rts     r0
```
The code above loads a second part of the boot code.
This boot code is located in 5 blocks, their addresses can
be found in the 'sector and track table' at the end of the 
code after label ```l_sctr``` (means *load_sectors* maybe). 
Block addresses are (track:sector) 1:4, 1:7, 1:10, 0:21 and 0:24.

In the 70ies, these cool long-bearded guys named a sector 'secter' 
as can be seen from the listing above.

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

In the fsutil directory, there is an object file ```rxboot2``` file already compiled, 
content is NOT identical but very close:
Blocks are at 07200, 010000, 010600, 05000, 05600.

```asm
dennis@dennis-pc:~/src/lsxunix/fsutil> pdp11-disasm rxboot2
         File: rxboot2
         Type: FMAGIC (0407) non-relocatable
Section .text: 602 bytes
Section .data: 0 bytes
 Section .bss: 0 bytes
 Symbol table: 0 names (0 bytes)
Entry address: 0

Disassembly of section .text:
000000 <.text>:
     0: 010702                  mov     pc, r2
     2: 005742                  tst     -(r2)
     4: 012706 057000           mov     $24064, sp
    10: 010601                  mov     sp, r1
    12: 020701                  cmp     pc, r1
    14: 103006                  bcc     032 <.text+032>
    16: 000005                  reset
    20: 012221                  mov     (r2)+, (r1)+
    22: 020127 060132           cmp     r1, $24666
    26: 103774                  bcs     16 <.text+020>
    30: 000116                  jmp     (sp)
    32: 005700                  tst     r0
    34: 001403                  beq     044 <.text+044>
    36: 052767 000020 000552    bis     $16, $0616 <.text+0616>
    44: 012701 060116           mov     $24654, r1
    50: 112100                  movb    (r1)+, r0
    52: 001403                  beq     062 <.text+062>
    54: 004767 000760           jsr     pc, 01040 <.text+01040>
    60: 000773                  br      050 <.text+050>
    62: 005001                  clr     r1
    64: 005000                  clr     r0
    66: 110021                  movb    r0, (r1)+
    70: 004767 000670           jsr     pc, 0764 <.text+0764>
    74: 020027 000012           cmp     r0, $10
   100: 001414                  beq     0132 <.text+0132>
   102: 020027 000100           cmp     r0, $64
   106: 001765                  beq     062 <.text+062>
   110: 020027 000040           cmp     r0, $32
   114: 001763                  beq     064 <.text+064>
   116: 020027 000043           cmp     r0, $35
   122: 001361                  bne     066 <.text+066>
   124: 005301                  dec     r1
   126: 003360                  bgt     070 <.text+070>
   130: 000754                  br      062 <.text+062>
   132: 010603                  mov     sp, r3
   134: 105021                  clrb    (r1)+
   136: 105021                  clrb    (r1)+
   140: 042701 000001           bic     $1, r1
   144: 160106                  sub     r1, sp
   146: 005002                  clr     r2
   150: 005701                  tst     r1
   152: 001410                  beq     0174 <.text+0174>
   154: 114143                  movb    -(r1), -(r3)
   156: 001774                  beq     0150 <.text+0150>
   160: 010304                  mov     r3, r4
   162: 114143                  movb    -(r1), -(r3)
   164: 001375                  bne     0160 <.text+0160>
   166: 010446                  mov     r4, -(sp)
   170: 005202                  inc     r2
   172: 000766                  br      0150 <.text+0150>
   174: 010246                  mov     r2, -(sp)
   176: 105344                  decb    -(r4)
   200: 012767 000001 000722    mov     $1, $01130 <.text+01130>
   206: 004767 000146           jsr     pc, 0360 <.text+0360>
   212: 010403                  mov     r4, r3
   214: 012705 056040           mov     $23584, r5
   220: 010304                  mov     r3, r4
   222: 010500                  mov     r5, r0
   224: 062705 000020           add     $16, r5
   230: 020027 056040           cmp     r0, $23584
   234: 103405                  bcs     0250 <.text+0250>
   236: 004767 000206           jsr     pc, 0450 <.text+0450>
   242: 000656                  br      0 <.text>
   244: 162705 001000           sub     $512, r5
   250: 020304                  cmp     r3, r4
   252: 001003                  bne     0262 <.text+0262>
   254: 012067 000650           mov     (r0)+, $01130 <.text+01130>
   260: 001757                  beq     0220 <.text+0220>
   262: 105724                  tstb    (r4)+
   264: 001410                  beq     0306 <.text+0306>
   266: 121427 000057           cmpb    (r4), $47
   272: 001745                  beq     0206 <.text+0206>
   274: 020005                  cmp     r0, r5
   276: 103371                  bcc     0262 <.text+0262>
   300: 121420                  cmpb    (r4), (r0)+
   302: 001752                  beq     0230 <.text+0230>
   304: 000745                  br      0220 <.text+0220>
   306: 004767 000046           jsr     pc, 0360 <.text+0360>
   312: 005003                  clr     r3
   314: 004767 000130           jsr     pc, 0450 <.text+0450>
   320: 000627                  br      0 <.text>
   322: 021027 000407           cmp     (r0), $263
   326: 001002                  bne     0334 <.text+0334>
   330: 062700 000020           add     $16, r0
   334: 012023                  mov     (r0)+, (r3)+
   336: 020027 056040           cmp     r0, $23584
   342: 103774                  bcs     0334 <.text+0334>
   344: 004767 000100           jsr     pc, 0450 <.text+0450>
   350: 000401                  br      0354 <.text+0354>
   352: 000770                  br      0334 <.text+0334>
   354: 004737 000000           jsr     pc, *$0 <.text>
   360: 016701 000544           mov     $01126 <.text+01126>, r1
   364: 062701 000037           add     $31, r1
   370: 010146                  mov     r1, -(sp)
   372: 006201                  asr     r1
   374: 006201                  asr     r1
   376: 006201                  asr     r1
   400: 006201                  asr     r1
   402: 004767 000140           jsr     pc, 0546 <.text+0546>
   406: 012601                  mov     (sp)+, r1
   410: 042701 177760           bic     $-16, r1
   414: 006301                  asl     r1
   416: 006301                  asl     r1
   420: 006301                  asl     r1
   422: 006301                  asl     r1
   424: 006301                  asl     r1
   426: 060001                  add     r0, r1
   430: 012700 055000           mov     $23040, r0
   434: 012120                  mov     (r1)+, (r0)+
   436: 020027 055030           cmp     r0, $23064
   442: 103774                  bcs     0434 <.text+0434>
   444: 005002                  clr     r2
   446: 000207                  rts     pc
   450: 062716 000002           add     $2, (sp)
   454: 010200                  mov     r2, r0
   456: 005202                  inc     r2
   460: 032727 010000 055000    bit     $4096, $23040
   466: 001010                  bne     0510 <.text+0510>
   470: 006300                  asl     r0
   472: 016001 055010           mov     23048(r0), r1
   476: 001023                  bne     0546 <.text+0546>
   500: 162716 000002           sub     $2, (sp)
   504: 005000                  clr     r0
   506: 000207                  rts     pc
   510: 010046                  mov     r0, -(sp)
   512: 105000                  clrb    r0
   514: 000300                  swab    r0
   516: 006300                  asl     r0
   520: 016001 055010           mov     23048(r0), r1
   524: 001765                  beq     0500 <.text+0500>
   526: 004767 000014           jsr     pc, 0546 <.text+0546>
   532: 006316                  asl     (sp)
   534: 042716 177001           bic     $-511, (sp)
   540: 062600                  add     (sp)+, r0
   542: 011001                  mov     (r0), r1
   544: 001755                  beq     0500 <.text+0500>
   546: 010546                  mov     r5, -(sp)
   550: 010446                  mov     r4, -(sp)
   552: 010346                  mov     r3, -(sp)
   554: 010246                  mov     r2, -(sp)
   556: 012704 177170           mov     $-392, r4
   562: 012703 177172           mov     $-390, r3
   566: 012702 055040           mov     $23072, r2
   572: 006301                  asl     r1
   574: 006301                  asl     r1
   576: 010146                  mov     r1, -(sp)
   600: 010105                  mov     r1, r5
   602: 062705 000004           add     $4, r5
   606: 032714 000040           bit     $32, (r4)
   612: 001775                  beq     0606 <.text+0606>
   614: 012714 000007           mov     $7, (r4)
   620: 011601                  mov     (sp), r1
   622: 006301                  asl     r1
   624: 061601                  add     (sp), r1
   626: 005000                  clr     r0
   630: 162701 000032           sub     $26, r1
   634: 100402                  bmi     0642 <.text+0642>
   636: 005200                  inc     r0
   640: 000773                  br      0630 <.text+0630>
   642: 062701 000033           add     $27, r1
   646: 105714                  tstb    (r4)
   650: 001776                  beq     0646 <.text+0646>
   652: 110113                  movb    r1, (r3)
   654: 010001                  mov     r0, r1
   656: 005000                  clr     r0
   660: 162701 000003           sub     $3, r1
   664: 100402                  bmi     0672 <.text+0672>
   666: 005200                  inc     r0
   670: 000773                  br      0660 <.text+0660>
   672: 005200                  inc     r0
   674: 022700 000115           cmp     $77, r0
   700: 001001                  bne     0704 <.text+0704>
   702: 005000                  clr     r0
   704: 105714                  tstb    (r4)
   706: 001776                  beq     0704 <.text+0704>
   710: 110013                  movb    r0, (r3)
   712: 105714                  tstb    (r4)
   714: 001776                  beq     0712 <.text+0712>
   716: 012714 000003           mov     $3, (r4)
   722: 000401                  br      0726 <.text+0726>
   724: 111322                  movb    (r3), (r2)+
   726: 105714                  tstb    (r4)
   730: 002775                  blt     0724 <.text+0724>
   732: 105714                  tstb    (r4)
   734: 003774                  ble     0726 <.text+0726>
   736: 005216                  inc     (sp)
   740: 021605                  cmp     (sp), r5
   742: 002721                  blt     0606 <.text+0606>
   744: 005726                  tst     (sp)+
   746: 012602                  mov     (sp)+, r2
   750: 012603                  mov     (sp)+, r3
   752: 012604                  mov     (sp)+, r4
   754: 012605                  mov     (sp)+, r5
   756: 012700 055040           mov     $23072, r0
   762: 000207                  rts     pc
   764: 012700 177560           mov     $-144, r0
   770: 005210                  inc     (r0)
   772: 105710                  tstb    (r0)
   774: 002376                  bge     0772 <.text+0772>
   776: 013700 177562           mov     *$65394, r0
  1002: 042700 177600           bic     $-128, r0
  1006: 020027 000101           cmp     r0, $65
  1012: 103405                  bcs     01026 <.text+01026>
  1014: 020027 000132           cmp     r0, $90
  1020: 101002                  bhi     01026 <.text+01026>
  1022: 062700 000040           add     $32, r0
  1026: 020027 000012           cmp     r0, $10
  1032: 001002                  bne     01040 <.text+01040>
  1034: 012700 000012           mov     $10, r0
  1040: 105737 177564           tstb    *$65396
  1044: 002375                  bge     01040 <.text+01040>
  1046: 020027 000012           cmp     r0, $10
  1052: 001016                  bne     01110 <.text+01110>
  1054: 012700 000012           mov     $10, r0
  1060: 004767 177754           jsr     pc, 01040 <.text+01040>
  1064: 012700 000212           mov     $138, r0
  1070: 004767 177744           jsr     pc, 01040 <.text+01040>
  1074: 005000                  clr     r0
  1076: 004767 177736           jsr     pc, 01040 <.text+01040>
  1102: 012700 000012           mov     $10, r0
  1106: 000207                  rts     pc
  1110: 010037 177566           mov     r0, *$65398
  1114: 000207                  rts     pc
  1116: 071012                  div     (r2), r0
  1120: 020170 067542           cmp     r1, *28514(r0)
  1124: 072157                  ash     *-(pc), r1
  1126: 000072                  .word   58
  1130: 000001                  wait
```

The 000072 000001 can be found here in root.dsk:
```
0005720  020170  067542  072157  000072  000001  000000  000000  000000
          x  sp   b   o   o   t   : nul soh nul nul nul nul nul nul nul
```

# Status
From ```lsxfs_inode_get()```, file ```inode.c```, I get this info:

*Inodes are numbered starting from 1. 
32 bytes per inode, 16 inodes per block.
Skip first and second blocks.*

Because a block has size 512, the first 1024 bytes are skipped.
After some debugging and thinking, I understood that all these Inodes
needs space at the beginning of the disk image and 
the first payload block starts with - guess it - 3328

Here is a verbose output showing this (with -v -v -v -v), There you
      have it, check the 3328 for block 0:
```asm
../../fsutil/lsx-util -n -s256000 -b../../fsutil/rxboot n1 -v -v -v -v
...
Boot sector size: 120 bytes
seek 0, block 0 - hw 3328
seek 128, block 0 - hw 3712
seek 256, block 0 - hw 4096
```

After knowing this, I could verify that the first bootstrap part (rxboot.o) is
at correct place.

The second bootstrap file also could be found, for that I added some code to lsx-util
tool to find (and later extract) the boot sectors
```
dennis@dennis-pc:~/src/lsxunix/0_pavl_zacharys_garage/original-disks> ~/src/lsxunix/fsutil/lsx-util -v -e abc root.dsk
Extracting boot sectors
Extracted boot sector, writing file abc
Looks like a boot sector
Found Opcode where list of secondary boot sectors follow
Secondary sector, track=1, sector=4, offset=7200, expected=7200
Secondary sector, track=1, sector=7, offset=10000, expected=10000
Secondary sector, track=1, sector=10, offset=10600, expected=10600
Secondary sector, track=1, sector=21, offset=13400, expected=5000
Secondary sector, track=0, sector=24, offset=5600, expected=5600
```

Next step is extract these two files and then to recreate a bootable root disk.