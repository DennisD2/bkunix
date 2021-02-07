# Debugging the bootstrap

After a root disk has been created that contains all
parts of the two-staged boot process, I could start
the boot process but it immediately stops with an HALT
instruction.

New root disk
is in file ```newroot.dsk```. For now, it contains only
a empty valid file system and the boot sectors filled.

simh ini ```newlsx.ini``` file for debugging the bootstrap.
```
at rx0 newroot.dsk
set cpu 48k
boot rx0
```

Execution with
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
Step expired, PC: 057032 (TST R0)
sim> ex -m 57030-57076
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

