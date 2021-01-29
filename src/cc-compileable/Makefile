
CC		=	pdp11-pcc
CFLAGS		=	-c -O
AS		=	pdp11-asm

SRCS1  = c00.c c01.c c02.c c03.c c04.c c05.c
OBJS1  = c00.o c01.o c02.o c03.o c04.o c05.o
ASRCS1 = c0t.s
AOBJS1 = c0t.o

SRCS2  = c10.c c11.c c12.c c13.c
OBJS2  = c10.o c11.o c12.o c13.o
AOBJS2 = c1t.s
AOBJST = table.o

SRCS3  = c20.c c21.c
OBJS3  = c20.o c21.o

all: $(OBJS1) $(AOBJS1) $(OBJS2) $(AOBJS2) $(AOBJST) $(OBJS3)

table.o:
	$(AS) table.i
	mv a.out table.o

