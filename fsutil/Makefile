DESTDIR		= ..
CC		= gcc

#CFLAGS		= -g -O -Wall -std=c89
CFLAGS		= -g -Wall -std=c89
OBJS		= superblock.o inode.o create.o check.o file.o block.o bootsector.o
PROGS		= lsx-util
LIBS		=

INSTALLFLAGS =
#  INSTALLFLAGS = -s

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

all:		$(PROGS) rxboot rxboot2

install:	$(PROGS)
		install $(INSTALLFLAGS) $(PROGS) $(DESTDIR)/bin/$(PROGS)

rxboot:		rxboot.s
		pdp11-asm $< -o $@
		pdp11-strip $@
		pdp11-objdump -D $@ > $@.dis

rxboot2:	rxboot2.s
		pdp11-asm $< -o $@
		pdp11-strip $@
		pdp11-objdump -D $@ > $@.dis

clean:
		rm -f *~ *.o *.lst *.dis $(PROGS) floppy.dsk

lsx-util:	fsutil.o $(OBJS)
		$(CC) $(LDFLAGS) -o $@ fsutil.o $(OBJS) $(LIBS)

floppy:		$(PROGS) rxboot rxboot2
		./lsx-util -n -s256000 -brxboot -Brxboot2 -v floppy.dsk
		./lsx-util floppy.dsk
		./lsx-util -c -v floppy.dsk
