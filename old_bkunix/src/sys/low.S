/*
 * Copyright 1975 Bell Telephone Laboratories Inc
 * low core
 */

/--------------
    .globl	_decmch

rxcs = 177170
rxdb = 177172

br5 = 340

_decmch:
	mov	r2,-(sp)
	mov	$rxdb,r1
	mov	4(sp),r0
	asl	r0
	add	4(sp),r0
	clr	r2
0:
	sub	$26.,r0
	bmi	0f
	inc	r2
	br	0b
0:
	add	$26.,r0
	inc	r0
	mov	r0,(r1)	/ sector number
0:
	bit	$200,-2(r1)
	beq	0b
	mov	r2,r0
	clr	r2
0:
	sub	$3.,r0
	bmi	0f
	inc	r2
	br	0b
0:
	mov	r2,(r1)	/ track number
	mov	(sp)+,r2
	rts	pc
. = 264^.
	fdintr; br5

/--------------
.globl	_fdintr
fdintr:	jsr	r0,call; _fdintr
