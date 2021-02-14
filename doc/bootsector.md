# Bootstrap procedure of operating system
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
rxcs	= 0177170 /command and status register of RX02 floppy disk device
rxdb	= rxcs+2  /multi purpose data register
go	= 1           /go bit (of rxcs) bit 0 starts command input
empty	= 2       /empty buffer - value of function bits bit 1-3
intlev	= 2
rdrx	= 6     /read sector instruction
unit_1	= 020   /UNITSEL bit 4
done	= 040   /done bit (of rxcs) bit 5
treq	= 0200  /TR bit 7
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

# Status
From ```lsxfs_inode_get()```, file ```inode.c```, I get this info:

*Inodes are numbered starting from 1.
32 bytes per inode, 16 inodes per block. Skip first and second blocks.*

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

I know that the line with *track=1, sector=21* is wrong. It must be
*track=0, sector=21* because I can see that in the file ```root.dsk```
and in the disassembled code.
(TODO-1: fix this)

Next step is extract these two files

# Content of sectors

Track 1, Sector 1
```
0000000 000240 005700 001403 052767 000020 000024 012701 177170
0000020 012702 177172 012704 000152 032711 000040 001775 012711
0000040 000007 105711 001776 105714 001453 112412 105711 001776
0000060 112412 105711 001776 100425 005711 100423 016700 000066
0000100 012711 000003 000401 111220 105711 100775 001775 005711
0000120 100410 010067 000040 005000 122767 000007 177702 005500
0000140 000733 012711 040000 000000 000713 000404 000407 000412
0000160 000025 000030 000000 000200 100607 017150 152123 127051
```

Track 1, Sector 4
```
0000000 010702 005742 012706 057000 010601 020701 103006 000005
0000020 012221 020127 060132 103774 000116 005700 001403 052767
0000040 000020 000552 012701 060116 112100 001403 004767 000760
0000060 000773 005001 005000 110021 004767 000670 020027 000012
0000100 001414 020027 000100 001765 020027 000040 001763 020027
0000120 000043 001361 005301 003360 000754 010603 105021 105021
0000140 042701 000001 160106 005002 005701 001410 114143 001774
0000160 010304 114143 001375 010446 005202 000766 010246 105344
```

Track 1, Sector 7
```
0000000 012767 000001 000722 004767 000146 010403 012705 056040
0000020 010304 010500 062705 000020 020027 056040 103405 004767
0000040 000206 000656 162705 001000 020304 001003 012067 000650
0000060 001757 105724 001410 121427 000057 001745 020005 103371
0000100 121420 001752 000745 004767 000046 005003 004767 000130
0000120 000627 021027 000407 001002 062700 000020 012023 020027
0000140 056040 103774 004767 000100 000401 000770 004737 000000
0000160 016701 000544 062701 000037 010146 006201 006201 006201
```

Track 1, Sector 10
```
0000000 006201 004767 000140 012601 042701 177760 006301 006301
0000020 006301 006301 006301 060001 012700 055000 012120 020027
0000040 055030 103774 005002 000207 062716 000002 010200 005202
0000060 032767 010000 175312 001010 006300 016001 055010 001023
0000100 162716 000002 005000 000207 010046 105000 000300 006300
0000120 016001 055010 001765 004767 000014 006316 042716 177001
0000140 062600 011001 001755 010546 010446 010346 010246 012704
0000160 177170 012703 177172 012702 055040 006301 006301 010146
```

Track 0, Sector 21
```
0000000 010105 062705 000004 032714 000040 001775 012714 000007
0000020 011601 006301 061601 005000 162701 000032 100402 005200
0000040 000773 062701 000033 105714 001776 110113 010001 005000
0000060 162701 000003 100402 005200 000773 005200 022700 000115
0000100 001001 005000 105714 001776 110013 105714 001776 012714
0000120 000003 000401 111322 105714 002775 105714 003774 005216
0000140 021605 002721 005726 012602 012603 012604 012605 012700
0000160 055040 000207 012700 177560 005210 105710 002376 016700
```

Track 0, Sector 24
```
0000000 117560 042700 177600 020027 000101 103405 020027 000132
0000020 101002 062700 000040 020027 000015 001002 012700 000012
0000040 105767 117520 002375 020027 000012 001016 012700 000015
0000060 004767 177754 012700 000212 004767 177744 005000 004767
0000100 177736 012700 000012 000207 010067 117452 000207 071012
0000120 020170 067542 072157 000072 000001 000000 000000 000000
0000140 066556 073561 071145 074564 064565 070157 071541 063144
0000160 064147 065552 075154 061570 061166 066556 073561 071145
```

All these files can be cat together;
```
cat abc-0 abc-1 abc-2 abc-3 abc-4 >abc2
```
The header needs to be fixed, so that text section is 602 bytes.

This gives the disassembly below. I do not know why the disassembler here 
relocates at 02000:
(TODO-2: Fix this, the code should start at 0).
```asm
         File: abc2
         Type: FMAGIC (0407) non-relocatable
Section .text: 602 bytes
Section .data: 0 bytes
 Section .bss: 0 bytes
 Symbol table: 0 names (0 bytes)
Entry address: 02000

Disassembly of section .text:
002000 010702              	mov	pc, r2
002002 005742              	tst	-(r2)
002004 012706 057000       	mov	$24064, sp
002010 010601              	mov	sp, r1
002012 020701              	cmp	pc, r1
002014 103006              	bcc	02032 <.text+032>
002016 000005              	reset
002020 012221              	mov	(r2)+, (r1)+
002022 020127 060132       	cmp	r1, $24666
002026 103774              	bcs	02020 <.text+020>
002030 000116              	jmp	(sp)
002032 005700              	tst	r0
002034 001403              	beq	02044 <.text+044>
002036 052767 000020 000552	bis	$16, $02616 <.text+0616>
002044 012701 060116       	mov	$24654, r1
002050 112100              	movb	(r1)+, r0
002052 001403              	beq	02062 <.text+062>
002054 004767 000760       	jsr	pc, 03040 <.text+01040>
002060 000773              	br	02050 <.text+050>
002062 005001              	clr	r1
002064 005000              	clr	r0
002066 110021              	movb	r0, (r1)+
002070 004767 000670       	jsr	pc, 02764 <.text+0764>
002074 020027 000012       	cmp	r0, $10
002100 001414              	beq	02132 <.text+0132>
002102 020027 000100       	cmp	r0, $64
002106 001765              	beq	02062 <.text+062>
002110 020027 000040       	cmp	r0, $32
002114 001763              	beq	02064 <.text+064>
002116 020027 000043       	cmp	r0, $35
002122 001361              	bne	02066 <.text+066>
002124 005301              	dec	r1
002126 003360              	bgt	02070 <.text+070>
002130 000754              	br	02062 <.text+062>
002132 010603              	mov	sp, r3
002134 105021              	clrb	(r1)+
002136 105021              	clrb	(r1)+
002140 042701 000001       	bic	$1, r1
002144 160106              	sub	r1, sp
002146 005002              	clr	r2
002150 005701              	tst	r1
002152 001410              	beq	02174 <.text+0174>
002154 114143              	movb	-(r1), -(r3)
002156 001774              	beq	02150 <.text+0150>
002160 010304              	mov	r3, r4
002162 114143              	movb	-(r1), -(r3)
002164 001375              	bne	02160 <.text+0160>
002166 010446              	mov	r4, -(sp)
002170 005202              	inc	r2
002172 000766              	br	02150 <.text+0150>
002174 010246              	mov	r2, -(sp)
002176 105344              	decb	-(r4)
002200 012767 000001 000722	mov	$1, $03130 <.text+01130>
002206 004767 000146       	jsr	pc, 02360 <.text+0360>
002212 010403              	mov	r4, r3
002214 012705 056040       	mov	$23584, r5
002220 010304              	mov	r3, r4
002222 010500              	mov	r5, r0
002224 062705 000020       	add	$16, r5
002230 020027 056040       	cmp	r0, $23584
002234 103405              	bcs	02250 <.text+0250>
002236 004767 000206       	jsr	pc, 02450 <.text+0450>
002242 000656              	br	02000 <.text>
002244 162705 001000       	sub	$512, r5
002250 020304              	cmp	r3, r4
002252 001003              	bne	02262 <.text+0262>
002254 012067 000650       	mov	(r0)+, $03130 <.text+01130>
002260 001757              	beq	02220 <.text+0220>
002262 105724              	tstb	(r4)+
002264 001410              	beq	02306 <.text+0306>
002266 121427 000057       	cmpb	(r4), $47
002272 001745              	beq	02206 <.text+0206>
002274 020005              	cmp	r0, r5
002276 103371              	bcc	02262 <.text+0262>
002300 121420              	cmpb	(r4), (r0)+
002302 001752              	beq	02230 <.text+0230>
002304 000745              	br	02220 <.text+0220>
002306 004767 000046       	jsr	pc, 02360 <.text+0360>
002312 005003              	clr	r3
002314 004767 000130       	jsr	pc, 02450 <.text+0450>
002320 000627              	br	02000 <.text>
002322 021027 000407       	cmp	(r0), $263
002326 001002              	bne	02334 <.text+0334>
002330 062700 000020       	add	$16, r0
002334 012023              	mov	(r0)+, (r3)+
002336 020027 056040       	cmp	r0, $23584
002342 103774              	bcs	02334 <.text+0334>
002344 004767 000100       	jsr	pc, 02450 <.text+0450>
002350 000401              	br	02354 <.text+0354>
002352 000770              	br	02334 <.text+0334>
002354 004737 000000       	jsr	pc, *$0 <.text-02000>
002360 016701 000544       	mov	$03126 <.text+01126>, r1
002364 062701 000037       	add	$31, r1
002370 010146              	mov	r1, -(sp)
002372 006201              	asr	r1
002374 006201              	asr	r1
002376 006201              	asr	r1
002400 006201              	asr	r1
002402 004767 000140       	jsr	pc, 02546 <.text+0546>
002406 012601              	mov	(sp)+, r1
002410 042701 177760       	bic	$-16, r1
002414 006301              	asl	r1
002416 006301              	asl	r1
002420 006301              	asl	r1
002422 006301              	asl	r1
002424 006301              	asl	r1
002426 060001              	add	r0, r1
002430 012700 055000       	mov	$23040, r0
002434 012120              	mov	(r1)+, (r0)+
002436 020027 055030       	cmp	r0, $23064
002442 103774              	bcs	02434 <.text+0434>
002444 005002              	clr	r2
002446 000207              	rts	pc
002450 062716 000002       	add	$2, (sp)
002454 010200              	mov	r2, r0
002456 005202              	inc	r2
002460 032767 010000 175312	bit	$4096, $0 <.text-02000>
002466 001010              	bne	02510 <.text+0510>
002470 006300              	asl	r0
002472 016001 055010       	mov	23048(r0), r1
002476 001023              	bne	02546 <.text+0546>
002500 162716 000002       	sub	$2, (sp)
002504 005000              	clr	r0
002506 000207              	rts	pc
002510 010046              	mov	r0, -(sp)
002512 105000              	clrb	r0
002514 000300              	swab	r0
002516 006300              	asl	r0
002520 016001 055010       	mov	23048(r0), r1
002524 001765              	beq	02500 <.text+0500>
002526 004767 000014       	jsr	pc, 02546 <.text+0546>
002532 006316              	asl	(sp)
002534 042716 177001       	bic	$-511, (sp)
002540 062600              	add	(sp)+, r0
002542 011001              	mov	(r0), r1
002544 001755              	beq	02500 <.text+0500>
002546 010546              	mov	r5, -(sp)
002550 010446              	mov	r4, -(sp)
002552 010346              	mov	r3, -(sp)
002554 010246              	mov	r2, -(sp)
002556 012704 177170       	mov	$-392, r4
002562 012703 177172       	mov	$-390, r3
002566 012702 055040       	mov	$23072, r2
002572 006301              	asl	r1
002574 006301              	asl	r1
002576 010146              	mov	r1, -(sp)
002600 010105              	mov	r1, r5
002602 062705 000004       	add	$4, r5
002606 032714 000040       	bit	$32, (r4)
002612 001775              	beq	02606 <.text+0606>
002614 012714 000007       	mov	$7, (r4)
002620 011601              	mov	(sp), r1
002622 006301              	asl	r1
002624 061601              	add	(sp), r1
002626 005000              	clr	r0
002630 162701 000032       	sub	$26, r1
002634 100402              	bmi	02642 <.text+0642>
002636 005200              	inc	r0
002640 000773              	br	02630 <.text+0630>
002642 062701 000033       	add	$27, r1
002646 105714              	tstb	(r4)
002650 001776              	beq	02646 <.text+0646>
002652 110113              	movb	r1, (r3)
002654 010001              	mov	r0, r1
002656 005000              	clr	r0
002660 162701 000003       	sub	$3, r1
002664 100402              	bmi	02672 <.text+0672>
002666 005200              	inc	r0
002670 000773              	br	02660 <.text+0660>
002672 005200              	inc	r0
002674 022700 000115       	cmp	$77, r0
002700 001001              	bne	02704 <.text+0704>
002702 005000              	clr	r0
002704 105714              	tstb	(r4)
002706 001776              	beq	02704 <.text+0704>
002710 110013              	movb	r0, (r3)
002712 105714              	tstb	(r4)
002714 001776              	beq	02712 <.text+0712>
002716 012714 000003       	mov	$3, (r4)
002722 000401              	br	02726 <.text+0726>
002724 111322              	movb	(r3), (r2)+
002726 105714              	tstb	(r4)
002730 002775              	blt	02724 <.text+0724>
002732 105714              	tstb	(r4)
002734 003774              	ble	02726 <.text+0726>
002736 005216              	inc	(sp)
002740 021605              	cmp	(sp), r5
002742 002721              	blt	02606 <.text+0606>
002744 005726              	tst	(sp)+
002746 012602              	mov	(sp)+, r2
002750 012603              	mov	(sp)+, r3
002752 012604              	mov	(sp)+, r4
002754 012605              	mov	(sp)+, r5
002756 012700 055040       	mov	$23072, r0
002762 000207              	rts	pc
002764 012700 177560       	mov	$-144, r0
002770 005210              	inc	(r0)
002772 105710              	tstb	(r0)
002774 002376              	bge	02772 <.text+0772>
002776 016700 117560       	mov	$0122560 <.bss-060352>, r0
003002 042700 177600       	bic	$-128, r0
003006 020027 000101       	cmp	r0, $65
003012 103405              	bcs	03026 <.text+01026>
003014 020027 000132       	cmp	r0, $90
003020 101002              	bhi	03026 <.text+01026>
003022 062700 000040       	add	$32, r0
003026 020027 000015       	cmp	r0, $13
003032 001002              	bne	03040 <.text+01040>
003034 012700 000012       	mov	$10, r0
003040 105767 117520       	tstb	$0122564 <.bss-060346>
003044 002375              	bge	03040 <.text+01040>
003046 020027 000012       	cmp	r0, $10
003052 001016              	bne	03110 <.text+01110>
003054 012700 000015       	mov	$13, r0
003060 004767 177754       	jsr	pc, 03040 <.text+01040>
003064 012700 000212       	mov	$138, r0
003070 004767 177744       	jsr	pc, 03040 <.text+01040>
003074 005000              	clr	r0
003076 004767 177736       	jsr	pc, 03040 <.text+01040>
003102 012700 000012       	mov	$10, r0
003106 000207              	rts	pc
003110 010067 117452       	mov	r0, $0122566 <.bss-060344>
003114 000207              	rts	pc
003116 071012               div	(r2), r0
003120 020170 067542       	cmp	r1, *28514(r0)
003124 072157              	ash	*-(pc), r1
003126 000072              	.word	58
003130 000001              	wait
```

The found ```rxboot2``` and the extracted file ```abc2``` only differ at 
some bytes:
```shell
cmp -l abc2 ../../fsutil/rxboot2
# byteposition value1 value2
321 367 327
325 312   0
326 372 132
528  35  27
529 160 162
530 237 377
553  15  12
561 367 337
563 120 164
564 237 377
575  15  12
601  67  37
603  52 166
604 237 377
```
# Creating a new root disk
The boot disk contains of the following ingredients:
* empty file system, created with lsxutil with size 256000 bytes
* Bootsector (track 1, sector 1) filled with file ``` rxboot```
  
* Secondary Bootsector with length of 5 sectors, 
  filled from file ```rxboot2``` in tracks-sectors

From the files above, I have then renamed ```abc``` to
```new_rxboot``` and ```abc2``` to ```new_rxboot2```.

With these parts, an empty root filesystem
with all boot sectors can be created with the following command:
```shell
../../fsutil/lsx-util -v -v -v -v -v --new -S --size=256000 --boot=new_rxboot --boot2=new_rxboot2 newroot.dsk
```

```
Boot sector size: 120 + 602 bytes
deskew 0 (00) -> track 1 - sector 0 = 3328 (06400)
seek 0, block 0 - hw 3328 (6400)
deskew 128 (0200) -> track 1 - sector 3 = 3712 (07200)
seek 128, block 0 - hw 3712 (7200)
allocate new block 9 from slot 91
allocate new block 10 from slot 90
Block numbers for secondary boot sectors: 9, 10 (adresses 011000, 012000)

deskew 4608 (011000) -> track 2 - sector 4 = 7168 (016000)
seek 4608, block 9 - hw 7168 (16000)
deskew 4736 (011200) -> track 2 - sector 7 = 7552 (016600)
seek 4736, block 9 - hw 7552 (16600)
deskew 4864 (011400) -> track 2 - sector 10 = 7936 (017400)
seek 4864, block 9 - hw 7936 (17400)
deskew 4992 (011600) -> track 2 - sector 13 = 8320 (020200)
seek 4992, block 9 - hw 8320 (20200)
deskew 5120 (012000) -> track 2 - sector 16 = 8704 (021000)
seek 5120, block 10 - hw 8704 (21000)

deskew 5120 (012000) -> track 2 - sector 16 = 8704 (021000)
seek 5120, block 10 - hw 8704 (21000)
deskew 5248 (012200) -> track 2 - sector 19 = 9088 (021600)
seek 5248, block 10 - hw 9088 (21600)
deskew 5376 (012400) -> track 2 - sector 22 = 9472 (022400)
seek 5376, block 10 - hw 9472 (22400)
deskew 5504 (012600) -> track 2 - sector 25 = 9856 (023200)
seek 5504, block 10 - hw 9856 (23200)
deskew 5632 (013000) -> track 2 - sector 2 = 6912 (015400)
seek 5632, block 11 - hw 6912 (15400)
Boot sectors new_rxboot and new_rxboot2 installed

sim> ex -o 150-164
150:    000713
152:    001004
154:    001007
156:    001012
160:    001015
162:    001020
164:    000000

sim> ex -h 150-164
150:    01CB
152:    0204
154:    0207
156:    020A
160:    020D
162:    0210
164:    0000

0006540 000733 012711 040000 000000 031713 001104 001007 001012
0006560 001015 001020 000000 000200 000000 000000 000000 000000

0006540 01db 15c9 4000 0000 33cb 0244 0207 020a
0006560 020d 0210 0000 0080 0000 0000 0000 0000

```

Now, the 5 secondary bootsectors need to be patched at addresses 0152-162
* From 1-4 to 2-5
* From 1-7 to 2-8
* From 1-10 to 2-B
* From 0-21 to 2-E
* From 0-24 to 2-11 (hex!)


Having done this, the boot disk is *bootable*.
Of course there is no UNIX on the root disk, but the bootstrap
should come to the output line:
```
rx boot:
``````

Continuation of this story can be found [here](bootstrap.md).

TODO: all sector numbers are +1 higher than I had expected.
So my calculation hw address on disk <-> track , sector need to be analyzed and fixed.

# cut of 16 byte header from bootsector files
dd bs=1 skip=16 if=new_rxboot2 of=new_rxboot2.bin