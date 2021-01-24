#
# Make LSI-UNIX system.
#
AS		= pdp11-asm
LD		= pdp11-ld

STRIP		= pdp11-strip
SIZE		= pdp11-size
ASRCS1		= as11.s as12.s as13.s as14.s as15.s \
              as16.s as17.s as18.s as19.s
ASRCS2		= as21.s as22.s as23.s as24.s as25.s \
            as26.s as27.s as28.s as29.s
OBJS1		= as11.o as12.o as13.o as14.o as15.o \
              as16.o as17.o as18.o as19.o
OBJS2		= as21.o as22.o as23.o as24.o as25.o \
              as26.o as27.o as28.o as29.o

all:		pdp11-asm pdp11-asm2

clean:
		rm -f *~ *.o pdp11-asm pdp11-asm2

.s.o:   $(OBJS)
	$(AS) $(LDFLAGS) -o $@ $<

pdp11-asm:		$(OBJS1)
		$(STRIP) $@
		@echo "Warning: For 10K LSX system, size must be less than 046000"
		@echo "         for 8k lsx system size must be less than 036500"
		$(SIZE) $@
		$(LD) -o pdp11-asm $(OBJS1)

pdp11-asm2:		$(OBJS2)
		$(STRIP) $@
		@echo "Warning: For 10K LSX system, size must be less than 046000"
		@echo "         for 8k lsx system size must be less than 036500"
		$(SIZE) $@
		$(LD) -o pdp11-asm2 $(OBJS2)
