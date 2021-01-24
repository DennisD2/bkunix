# LSX Unix revival

# Extracting the disks
Original disks are stored as blobs in ```original-disks```.
Their content can be extracted.

Execute script ```extract-disks.sh```
This creates a directory per disk in directory 
```extracted-disks```

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
