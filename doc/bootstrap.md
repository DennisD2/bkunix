# Debugging the bootstrap
After a root disk has been created that contains all
parts of the two-staged boot process, I could start
the boot process.


Execution if newroot.dsk file with
```bash
~/pdp11/simh/simh-master/BIN/pdp11 newlsx.ini
```
Short simh command list for running a CPU in single
step mode
* run 0 : runs code starting at some address (here 0)
* step : single step, ```s n``` does n steps
* cont : continue execution  
* examine -m 100-120 : dissassembles code at position 100-120
* ex r0 : shows content of register R0
* ex 177170 : shows content at address
* break 120 : breakpoint at position 120
* show break : shows all breakpoints
* nobreak 120: removes breakpoint at position

Example code disassemblation:
```asm
sim> ex -m 220-240
220:    MOV (R2)+,(R1)+
222:    CMP R1,#60132
226:    BCS 220
230:    JMP (SP)
232:    TST R0
234:    BEQ 244
236:    BIS #20,1016
```
With this little knowledge I could verify that the initial
bootblock is loaded and that it then correctly loads the
secondary boot sectors. Then code jumps to execution of secondary
bootblock, where right at start, the HALT occurs.

Setting breakpoint to first instruction of secondary boot:
```
~/pdp11/simh/simh-master/BIN/pdp11 newlsx.ini
break 200
# start CPU, will give a HALT at 0x02, then continue
run 0
cont
# We will stop at 0x200 after secondary boot sectors were loaded
sim> cont

Breakpoint, PC: 000200 (MOV PC,R2)
```

# Status
Code breaks here:
```asm
Step expired, PC: 000220 (MOV (R2)+,(R1)+)
sim> s

Step expired, PC: 000222 (CMP R1,#60132)
sim> s

Step expired, PC: 000226 (BCS 220)
sim> s

Step expired, PC: 000220 (MOV (R2)+,(R1)+)
sim> s

Step expired, PC: 000222 (CMP R1,#60132)
sim> ex r1
R1:     057006
sim> br 230
sim> cont

Breakpoint, PC: 000230 (JMP (SP)) <---------- a far jump 
sim> ex sp
SP:     057000

Step expired, PC: 057032 (TST R0)
sim> ex -m 57000-57076
57000:  MOV PC,R2
57002:  TST -(R2)
57004:  MOV #57000,SP
57010:  MOV SP,R1
57012:  CMP PC,R1
57014:  BCC 57032
57016:  RESET
57020:  MOV (R2)+,(R1)+
57022:  CMP R1,#60132
57026:  BCS 57020
57030:  JMP (SP)
57032:  TST R0
57034:  BEQ 57044
57036:  BIS #20,57616
57044:  MOV #60116,R1
57050:  MOVB (R1)+,R0
57052:  BEQ 57062
57054:  JSR PC,60040
57060:  BR 57050
57062:  CLR R1
57064:  CLR R0
57066:  MOVB R0,(R1)+
57070:  JSR PC,57764
57074:  CMP R0,#12
sim> s

Step expired, PC: 057034 (BEQ 57044)
sim> s

Step expired, PC: 057044 (MOV #60116,R1)
sim> s

Step expired, PC: 057050 (MOVB (R1)+,R0)
sim> s

Step expired, PC: 057052 (BEQ 57062)
sim> s

Step expired, PC: 057062 (CLR R1)
sim> s

Step expired, PC: 057064 (CLR R0)
sim> s

Step expired, PC: 057066 (MOVB R0,(R1)+)
sim> s

Step expired, PC: 057070 (JSR PC,57764) <------- failing jsr jump
sim> s

Step expired, PC: 057764 (HALT)
sim> s

HALT instruction, PC: 057766 (HALT)

sim> ex -m 57750-57770
57750:  HALT
57752:  HALT
57754:  HALT
57756:  HALT
57760:  HALT
57762:  HALT
57764:  HALT
57766:  HALT <----- jump target
57770:  HALT
```

Because reloading of secondary bootsectors did not work after my changes,
I have to study the first bootsector code in detail.

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
0   	nop			/this is required by the rom
2	    tst	r0		/unit number is in r0 (0,1) - tst on value Zero
4	    beq	0f      /jump forward to label 0 - if value 0
6	    bis	$unit_1,readop	/set instruction to read unit 1 (by or-ing value unit_1 to readop)
    0:
14	    mov	$rxcs,r1	/control and status register to r1
20	    mov	$rxdb,r2	/data and sector/track addr reg to r2
24	    mov	$l_sctr, r4	/set address of secter and track table to r4
    next:
30	    bit	$done,(r1)  /test if done flag is set in rxcs 
34	    beq	next        /loop back to label next until drive is ready
36	    mov	(pc)+,(r1)	/read sector instruction to rxcs, get it from next ".word" This triggers read
    readop:
	    .word	rdrx+go		/read sector instruction, can be modified by unit number
    1:
42	    tstb	(r1)		/transfer request flag - check if rxcs==0
44	    beq	1b              /loop back to label 1 while (r1)==0 (checks for Z==1) 
46	    tstb	(r4)		/last block ? check if (r4)==0
50	    beq	rxboot+0200	/ if yes, jump to program
52	    movb	(r4)+, (r2)	/more to go. setup for next sector (byte)
    2:
54	    tstb	(r1)		/transfer request flag - check if rxcs==0
56	    beq	2b              /loop back to label 2 while (r1)==0 (checks for Z==1) 
60	    movb	(r4)+, (r2)	/track address (byte)
    3:
62	    tstb	(r1)		/read complete ?
    efloop:
64      beq	3b              /not yet
66      bmi	erflag		    /treq on -- error , bit 15 is set 
70      tst	(r1)		    /error flag
72      bmi	erflag
74      mov	bufaddr,r0	    /current buffer address
100     mov	$empty+go,(r1)	/empty rx function to rxcs 
104     br	1f              /jump forward to label 1 
106	    movb	(r2),(r0)+	/empty buffer - is this a byte-wise copy from RX02?
	                    / in r2 the rxdb is located, is this the data byte?
	                    / in r0 bufaddr is located - set initial to 0200
    1:
110     tstb	(r1)		/transfer request flag
112     bmi	efloop		    /br if ready - we read next byte if so
114     beq	1b		        /wait for flag - wait until we can continue reading
116     tst	(r1)		    /error flag
120     bmi	erflag          /Something went wrong
122     mov	r0,bufaddr	    /all fine, next set of 'empty' locations
                            /r0 was incremented in 'efloop' 
126     clr	r0		        /setup unit number in r0 - used for unit number again...
130     cmpb	$rdrx+go,readop	/is unit number zero
136     adc	r0		        /id unit number 0,1 - set r0 to correct value (0 or 1)?
140     br	next		    /read another sector
    erflag:
142     mov	$initrx,(r1)	/initialize heads -- read first sector
146     halt			    /error
150    	br	rxboot		    /restart
    l_sctr:
                    / table of block addresses for boot prog.
152     .byte 4, 1		/ sector 4	track 1
154     .byte 7, 1		/ sector 7	track 1
156     .byte 10, 1		/ sector 10	track 1
160     .byte 21, 0		/ sector 21	track 0
162     .byte 24, 0		/ sector 24	track 0
164     .word 0			/ end of table
    bufaddr:
166     .word 0200		/start of pathname bootstrap
170     .even
```

# Starting the LSX kernel 'lsx'

(Bootsector file 2 is now in ```boot2``` as regular file on disk)

Content of simh after ```lsx``` was loaded, so bootsector2 code worked.

```shell
sim> ex -m 0-600
0:      BR 40
2:      IOT
4:      SEN
6:      SWAB -(R0)
10:     SEN
12:     SWAB -(R1)
14:     SEN
16:     SWAB -(R2)
20:     SEN
22:     SWAB -(R3)
24:     SEN
26:     SWAB -(R4)
30:     SEN
32:     SWAB -(R5)
34:     SEN
36:     SWAB -(SP)
40:     JMP 1560
44:     JMP 140
50:     HALT
52:     HALT
54:     HALT
56:     HALT
60:     JMP (R0)
62:     SWAB -(R0)
64:     JMP (SP)
66:     SWAB -(R0)
70:     HALT
72:     HALT
74:     HALT
76:     HALT
100:    JMP (R4)+
102:    SWAB -(R0)
104:    JMP (R4)+
106:    SWAB -(R0)
110:    JSR R0,326
114:    MOV R2,SP
116:    JSR R0,326
122:    MOV R1,@(SP)+
124:    JSR R0,326
130:    ADC 4067(SP)
134:    JMP @2302(R0)
140:    HALT
142:    HALT
144:    HALT
146:    HALT
150:    HALT
152:    HALT
154:    HALT
156:    HALT
160:    HALT
162:    HALT
164:    HALT
166:    HALT
170:    HALT
172:    HALT
174:    HALT
176:    HALT
200:    HALT
202:    HALT
204:    HALT
206:    HALT
210:    HALT
212:    HALT
214:    HALT
216:    HALT
220:    HALT
222:    HALT
224:    HALT
226:    HALT
230:    HALT
232:    HALT
234:    HALT
236:    HALT
240:    HALT
242:    HALT
244:    HALT
246:    HALT
250:    HALT
252:    HALT
254:    HALT
256:    HALT
260:    HALT
262:    HALT
264:    JMP @(R2)+
266:    SWAB -(R0)
270:    MFPS 177774(SP)
274:    TST 36356
300:    BNE 310
302:    JSR R0,316
306:    CMP -(R1),R0
310:    MOV 36356,(SP)
314:    RTI
316:    TST -(SP)
320:    CLR -(SP)
322:    MTPS (SP)+
324:    BR 330
326:    MFPS -(SP)
330:    MOV R1,-(SP)
332:    MOV SP,R1
334:    MOV R1,-(SP)
336:    ADD #12,(SP)
342:    MOV 4(SP),-(SP)
346:    BIC #177740,(SP)
352:    CMP 12(SP),#40000
360:    BCS 426
362:    INC 36360
366:    MOV 2(SP),R1
372:    MOV #40000,SP
376:    MOV -(R1),-(SP)
402:    MOV -(R1),-(SP)
404:    MOV -(R1),-(SP)
406:    MOV -(R1),-(SP)
410:    MOV -(R1),-(SP)
412:    MOV -(R1),-(SP)
414:    JSR PC,@(R0)+
416:    MOV #340,-(SP)
422:    MTPS (SP)+
424:    BR 434
426:    CLR 36360
432:    JSR PC,@(R0)+
434:    TST (SP)+
436:    MOV (SP)+,R1
440:    CMP 6(SP),#40000
446:    BCS 474
450:    SUB #12,R1
454:    MOV (SP)+,(R1)+
456:    MOV (SP)+,(R1)+
460:    MOV (SP)+,(R1)+
462:    MOV (SP)+,(R1)+
464:    MOV (SP)+,(R1)+
466:    SUB #12,R1
472:    MOV R1,SP
474:    MOV (SP)+,R1
476:    TST (SP)+
500:    MOV (SP)+,R0
502:    RTI
504:    MOV 2(SP),R1
510:    MFPS -(SP)
512:    MOV R2,-(SP)
514:    MOV #340,-(SP)
520:    MTPS (SP)+
522:    MOV 2(R1),R2
526:    BEQ 616
530:    MOVB (R2)+,R0
532:    BIC #177400,R0
536:    MOV R2,2(R1)
542:    DEC (R1)+
544:    BNE 554
546:    CLR (R1)+
550:    CLR (R1)+
552:    BR 572
554:    BIT #7,R2
560:    BNE 610
562:    MOV 177770(R2),(R1)
566:    ADD #2,(R1)
572:    DEC R2
574:    BIC #7,R2
600:    MOV 25770,(R2)
```

Decompile (start) of ```lsx```:
```shell
         File: lsx
         Type: FMAGIC (0407) non-relocatable
Section .text: 10740 bytes
Section .data: 516 bytes
 Section .bss: 4346 bytes
 Symbol table: 0 names (0 bytes)
Entry address: 02000

Disassembly of section .text:
002000 000417                   br      02040 <.text+040>
002002 000004                   iot
002004 000270                   sen
002006 000340                   swab    -(r0)
002010 000270                   sen
002012 000341                   swab    -(r1)
002014 000270                   sen
002016 000342                   swab    -(r2)
002020 000270                   sen
002022 000343                   swab    -(r3)
002024 000270                   sen
002026 000344                   swab    -(r4)
002030 000270                   sen
002032 000345                   swab    -(r5)
002034 000270                   sen
002036 000346                   swab    -(sp)
002040 000167 001514            jmp     03560 <.text+01560>
002044 000167 000070            jmp     02140 <.text+0140>
002050 000000                   halt
002052 000000                   halt
002054 000000                   halt
002056 000000                   halt
002060 000110                   jmp     (r0)
002062 000340                   swab    -(r0)
002064 000116                   jmp     (sp)
002066 000340                   swab    -(r0)
002070 000000                   halt
002072 000000                   halt
002074 000000                   halt
002076 000000                   halt
002100 000124                   jmp     (r4)+
...
```

## Status
lsx unix kernel is loaded and started.
With debugging simh I found that the tty loop is running (waiting for input).
But the tty does not output the shell prompt '#' as the original boot disk does.
It also does not get character input.

By setting a breakpoint to 1352, we can interrupt the input loop when pressing
a key. This works with old disk, but does not work with my newly created disk.

I suppose there is something wrong in the layers kernel -> tty driver code -> 
simh 'hardware' layer. This needs to be analyzed next.

```bash
> ~/pdp11/simh/simh-master/BIN/pdp11 newlsx.ini

PDP-11 simulator V4.0-0 Current        git commit id: d5cc3406
/home/dennis/src/lsxunix/0_pavl_zacharys_garage/extracted-disks/root/newlsx.ini-1> at rx0 newroot.dsk
RX: buffering file in memory
/home/dennis/src/lsxunix/0_pavl_zacharys_garage/extracted-disks/root/newlsx.ini-2> at rx1 usr.dsk
RX: buffering file in memory

rx boot:lsx

[results in endless loop, entered CRTL-e here]

Simulation stopped, PC: 001350 (MTPS (SP)+)
```
