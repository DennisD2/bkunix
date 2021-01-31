
#include <stdio.h>
#include "lsxfs.h"

#define SECTORSIZE_BYTES 128

int extract_bootsectors(lsxfs_t *fs, char *basename);