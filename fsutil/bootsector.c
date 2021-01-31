#include "bootsector.h"
#include <fcntl.h>
#include <unistd.h>

/*
 * Bootsector extraction code
 */

extern int verbose;

int write_file( char *name, unsigned char *buf, unsigned long num_bytes, struct filhdr *file_header) {
    int written;
    int fd = open(name, O_CREAT|O_RDWR, 0666);
    if (fd < 0) {
        fprintf(stderr, "%s: could not open file, return value %d\n",
                 name, fd);
        return 0;
    }
    if (file_header != NULL) {
        /* write header */
        written = write(fd, file_header, sizeof(struct filhdr));
        if (written != sizeof(struct filhdr)) {
            fprintf(stderr, "%s: could not write file header for file\n", name);
            return 0;
        }
    }
    written = write (fd, buf, num_bytes);
    if (written != num_bytes) {
        fprintf(stderr, "%s: could not write file\n", name);
        return 0;
    }
    close(fd);
    return 1;
}

int read_sector( lsxfs_t *fs, unsigned long offset,
                 unsigned long num_bytes, unsigned char *buf ) {
    if (! lsxfs_seek(fs, offset)) {
        return 0;
    }
    int byte_offset=0;
    while (byte_offset < num_bytes) {
        if (! lsxfs_read8(fs, &(buf[byte_offset]))) {
            return 0;
        }
        /*printf("0%03o\n", buf[byte_offset]);*/
        byte_offset++;
    }
    return 1;
}

int read_sector_raw( lsxfs_t *fs, unsigned int track, unsigned int sector,
                     unsigned long num_bytes, unsigned char *buf ) {
    if (! lsxfs_seek_raw(fs, track, sector)) {
        return 0;
    }
    int byte_offset=0;
    while (byte_offset < num_bytes) {
        if (! lsxfs_read8(fs, &(buf[byte_offset]))) {
            return 0;
        }
        /*printf("0%03o\n", buf[byte_offset]);*/
        byte_offset++;
    }
    return 1;
}

int extract_bootsectors(lsxfs_t *fs, char *basename) {
    unsigned char buf[SECTORSIZE_BYTES];
    if (verbose) {
        printf("Extracting boot sectors\n");
    }

    /* first/single bootsector is at offset 0 */
    /*if (read_sector( fs, 0L, SECTORSIZE_BYTES, buf) == 0) {*/
    if (read_sector_raw( fs, 1, 0, SECTORSIZE_BYTES, buf) == 0) {
        fprintf(stderr, "Error reading sector 0\n");
        return 0;
    }

    if (buf[1] != 000 && buf[0] != 0240) {
        fprintf(stderr, "Does not look like a boot sector No '0000 0240' as first two bytes %x %x\n",
                 buf[0], buf[1]);
    } else {
        printf("Looks like a boot sector\n");
    }

    struct filhdr file_header;
    file_header.fmagic = 0407; /* 04 07 */
    file_header.tsize = 120; /* 120 */
    file_header.bsize = 0;
    file_header.dsize = 0;
    file_header.ssize = 0;
    file_header.entry = 0;
    file_header.pad = 0;
    file_header.relflg = 1; /* 1 */

    printf("Writing file %s\n", basename);
    if ( write_file(basename, buf, SECTORSIZE_BYTES, &file_header) == 0) {
        fprintf(stderr, "Could not write bootsector file %s\n", basename);
        return 0;
    }

    int i;
    unsigned short track[10], sector[10];
    track[0] = 0;
    for (i=0; i<SECTORSIZE_BYTES; i++) {
        /*printf("0%03o\n", buf[i]);*/
        if (buf[i] == (unsigned char)0713 && i>0 && buf[i-1] == 000) {
            printf("Found Opcode where list of secondary boot sectors follow\n");
            i++;
            int j = 0, k= 0;
            while (buf[i+k] != 000 || buf[i+k+1] != 000) {
                /*printf("track: 0%03x\n", buf[i+k]);
                printf("sector: 0%03x\n", buf[i+k+1]);*/
                track[j] = buf[i+k];
                sector[j] = buf[i+k+1];
                j++;
                k += 2;
            }
            track[j] = sector[j] = 0; /* end marker */
            break;
        }
    }
    /*extern unsigned long deskew (unsigned long address);*/
    long expected[] = { 07200, 010000, 010600, 05000, 05600 };
    if (track[0] != 0 || sector[0] != 0 ) {
        char filename[128];
        for (i=0; track[i] != 0 || sector[i] != 0; i++ ) {
            long offset = (track[i] * 26 + sector[i] - 1) * 128;
            printf("Secondary sector, track=%d, sector=%d, offset=%lo, expected=%lo\n", track[i], sector[i],
                   offset, expected[i]);
            filename[0] = '\0';
            sprintf( filename, "%s-%d", basename, i );

            if (offset != expected[i]) {
                printf("Patching value %lo to %lo\n", offset, expected[i] );
                offset = expected[i];
                track[i] = 0;
            }

            printf("Read sector at offset %lo\n", offset);
            /* TODO : explain -1 */
            if (read_sector_raw( fs, track[i], sector[i] - 1, SECTORSIZE_BYTES, buf) == 0) {
                fprintf(stderr, "Error reading sector 0\n");
                return 0;
            }
            struct filhdr *header = NULL;
            if (i == 0) {
                file_header.fmagic = 0407;
                file_header.tsize = 602; /* found by trial and error */
                file_header.bsize = 0;
                file_header.dsize = 0;
                file_header.ssize = 0;
                file_header.entry = 0;
                file_header.pad = 0;
                file_header.relflg = 1;
                header = &file_header;
            }
            printf("Write sector to file %s\n", filename);
            if (write_file( filename, buf, SECTORSIZE_BYTES, header) == 0) {
                fprintf(stderr, "Could not write sector file %s\n", filename);
                return 0;
            }
        }
    }
    printf("All sector files written\n");
    return 1;
}
