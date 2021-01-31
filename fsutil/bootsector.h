
#include <stdio.h>
#include "lsxfs.h"

#define SECTORSIZE_BYTES 128

int extract_bootsectors(lsxfs_t *fs, char *basename);

/* object file header from ld.c */
struct filhdr {
    short fmagic;
    short	tsize;
    short	dsize;
    short	bsize;
    short	ssize;
    short	entry;
    short	pad;
    short	relflg;
} filhdr;
