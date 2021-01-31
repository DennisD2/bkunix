#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <getopt.h>
#include "lsxfs.h"

int verbose;
int extract;
int extract_bootsector;
int add;
int newfs;
int check;
int fix;
int flat = 1;
unsigned long bytes;
char *boot_sector;
char *boot_sector2;
char *boot_sector_file_base;

#define alloca(x)  __builtin_alloca(x)

const char *program_version =
	"LSX file system information, version 1.1\n"
	"Copyright (C) 2002 Serge Vakulenko, LSX code reintroduced by Spurtikus.\n"
	"This program is free software; it comes with ABSOLUTELY NO WARRANTY;\n"
	"see the GNU General Public License for more details.";

static const char *program_bug_address = "<vak@cronyx.ru>";

static struct option program_options[] = {
        { "help",	no_argument,		0,	'h' },
        { "version",	no_argument,		0,	'V' },
        { "verbose",	no_argument,		0,	'v' },
        { "add",	no_argument,		    0,	'a' },
        { "extract",	no_argument,		0,	'x' },
        { "check",	no_argument,		0,	'c' },
        { "fix",	no_argument,		    0,	'f' },
        { "new",	no_argument,		    0,	'n' },
        { "size",	required_argument,	0,	's' },
        { "boot",	required_argument,	0,	'b' },
        { "boot2",	required_argument,	0,	'B' },
        { "flat",	no_argument,		0,	'F' },
        { "shuffle",	no_argument,		0,	'S' },
        { "extractboot",	required_argument,0,	'e' },
        { 0 }
};

static void print_help (char *progname)
{
	printf ("%s\n", program_version);
	printf ("This program is free software; it comes with ABSOLUTELY NO WARRANTY;\n"
		"see the GNU General Public License for more details.\n");
	printf ("\n");
	printf ("Usage:\n");
	printf ("    %s filesys.bkd\n", progname);
	printf ("    %s --add filesys.bkd files...\n", progname);
	printf ("    %s --extract filesys.bkd\n", progname);
	printf ("    %s --extractboot bootsector filesys.bkd\n", progname);
	printf ("    %s --check [--fix] filesys.bkd\n", progname);
	printf ("    %s --new --size=bytes filesys.bkd\n", progname);
	printf ("\n");
	printf ("Options:\n");
	printf ("  -a, --add              Add files to filesystem\n");
	printf ("  -x, --extract          Extract all files\n");
	printf ("  -e, --extractboot FILE Extract boot sector(s) to file(s)\n");
	printf ("  -c, --check            Check filesystem, use -c -f to fix\n");
	printf ("  -f, --fix              Fix bugs in filesystem\n");
	printf ("  -n, --new              Create new filesystem, -s required\n");
	printf ("  -s NUM, --size=NUM     Size in bytes for created filesystem\n");
	printf ("  -b FILE, --boot=FILE   Boot sector, -B required if -S\n");
	printf ("  -B FILE, --boot2=FILE  Secondary boot sector, -b required\n");
	printf ("  -F, --flat             Flat mode, no sector remapping (default)\n");
	printf ("  -S, --shuffle          Shuffle mode, remap 128-byte sectors\n");
	printf ("  -v, --verbose          Print verbose information\n");
	printf ("  -V, --version          Print version information and exit\n");
	printf ("  -h, --help             Print this message\n");
	printf ("\n");
	printf ("Report bugs to \"%s\".\n", program_bug_address);
}

void print_inode (lsxfs_inode_t *inode,
	char *dirname, char *filename, FILE *out)
{
	fprintf (out, "%s/%s", dirname, filename);
	switch (inode->mode & INODE_MODE_FMT) {
	case INODE_MODE_FDIR:
		fprintf (out, "/\n");
		break;
	case INODE_MODE_FCHR:
		fprintf (out, " - char %d %d\n",
			inode->addr[0] >> 8, inode->addr[0] & 0xff);
		break;
	case INODE_MODE_FBLK:
		fprintf (out, " - block %d %d\n",
			inode->addr[0] >> 8, inode->addr[0] & 0xff);
		break;
	default:
		fprintf (out, " - %lu bytes\n", inode->size);
		break;
	}
}

void print_indirect_block (lsxfs_t *fs, unsigned int bno, FILE *out)
{
	unsigned short nb;
	unsigned char data [LSXFS_BSIZE];
	int i;

	fprintf (out, " [%d]", bno);
	if (! lsxfs_read_block (fs, bno, data)) {
		fprintf (stderr, "read error at block %d\n", bno);
		return;
	}
	for (i=0; i<LSXFS_BSIZE-2; i+=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb)
			fprintf (out, " %d", nb);
	}
}

void print_double_indirect_block (lsxfs_t *fs, unsigned int bno, FILE *out)
{
	unsigned short nb;
	unsigned char data [LSXFS_BSIZE];
	int i;

	fprintf (out, " [%d]", bno);
	if (! lsxfs_read_block (fs, bno, data)) {
		fprintf (stderr, "read error at block %d\n", bno);
		return;
	}
	for (i=0; i<LSXFS_BSIZE-2; i+=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb)
			print_indirect_block (fs, nb, out);
	}
}

void print_inode_blocks (lsxfs_inode_t *inode, FILE *out)
{
	int i;

	if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FCHR ||
	    (inode->mode & INODE_MODE_FMT) == INODE_MODE_FBLK)
		return;

	fprintf (out, "    ");
	if (inode->mode & INODE_MODE_LARG) {
		for (i=0; i<7; ++i) {
			if (inode->addr[i] == 0)
				continue;
			print_indirect_block (inode->fs, inode->addr[i], out);
		}
		if (inode->addr[7] != 0)
			print_double_indirect_block (inode->fs,
				inode->addr[7], out);
	} else {
		for (i=0; i<8; ++i) {
			if (inode->addr[i] == 0)
				continue;
			fprintf (out, " %d", inode->addr[i]);
		}
	}
	fprintf (out, "\n");
}

void extract_inode (lsxfs_inode_t *inode, char *path)
{
	int fd, n;
	unsigned long offset;
	unsigned char data [512];

	fd = open (path, O_CREAT | O_WRONLY, inode->mode & 0x777);
	if (fd < 0) {
		perror (path);
		return;
	}
	for (offset = 0; offset < inode->size; offset += 512) {
		n = inode->size - offset;
		if (n > 512)
			n = 512;
		if (! lsxfs_inode_read (inode, offset, data, n)) {
			fprintf (stderr, "%s: read error at offset %ld\n",
				path, offset);
			break;
		}
		if (write (fd, data, n) != n) {
			fprintf (stderr, "%s: write error\n", path);
			break;
		}
	}
	close (fd);
}

void extractor (lsxfs_inode_t *dir, lsxfs_inode_t *inode,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	if (verbose)
		print_inode (inode, dirname, filename, out);

	if ((inode->mode & INODE_MODE_FMT) != INODE_MODE_FDIR &&
	    (inode->mode & INODE_MODE_FMT) != 0)
		return;

	path = alloca (strlen (dirname) + strlen (filename) + 2);
	strcpy (path, dirname);
	strcat (path, "/");
	strcat (path, filename);

	if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		if (mkdir (path, 0775) < 0)
			perror (path);
		/* Scan subdirectory. */
		lsxfs_directory_scan (inode, path, extractor, arg);
	} else {
		extract_inode (inode, path);
	}
}

void scanner (lsxfs_inode_t *dir, lsxfs_inode_t *inode,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	print_inode (inode, dirname, filename, out);

	if (verbose > 1) {
		/* Print a list of blocks. */
		print_inode_blocks (inode, out);
		if (verbose > 2) {
			lsxfs_inode_print (inode, out);
			printf ("--------\n");
		}
	}
	if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		/* Scan subdirectory. */
		path = alloca (strlen (dirname) + strlen (filename) + 2);
		strcpy (path, dirname);
		strcat (path, "/");
		strcat (path, filename);
		lsxfs_directory_scan (inode, path, scanner, arg);
	}
}

/*
 * Create a directory.
 */
void add_directory (lsxfs_t *fs, char *name)
{
	lsxfs_inode_t dir, parent;
	char buf [512], *p;

	/* Open parent directory. */
	strcpy (buf, name);
	p = strrchr (buf, '/');
	if (p)
		*p = 0;
	else
		*buf = 0;
	if (! lsxfs_inode_by_name (fs, &parent, buf, 0, 0)) {
		fprintf (stderr, "%s: cannot open directory\n", buf);
		return;
	}

	/* Create directory. */
	if (! lsxfs_inode_by_name (fs, &dir, name, 1,
	    INODE_MODE_FDIR | 0777)) {
		fprintf (stderr, "%s: directory inode create failed\n", name);
		return;
	}
	lsxfs_inode_save (&dir /*, 0*/);

	/* Make link '.' */
	strcpy (buf, name);
	strcat (buf, "/.");
	if (! lsxfs_inode_by_name (fs, &dir, buf, 3, dir.number)) {
		fprintf (stderr, "%s: dot link failed\n", name);
		return;
	}
	++dir.nlink;
	lsxfs_inode_save (&dir /*, 1*/);
/*printf ("*** inode %d: increment link counter to %d\n", dir.number, dir.nlink);*/

	/* Make parent link '..' */
	strcat (buf, ".");
	if (! lsxfs_inode_by_name (fs, &dir, buf, 3, parent.number)) {
		fprintf (stderr, "%s: dotdot link failed\n", name);
		return;
	}
	if (! lsxfs_inode_get (fs, &parent, parent.number)) {
		fprintf (stderr, "inode %d: cannot open parent\n", parent.number);
		return;
	}
	++parent.nlink;
	lsxfs_inode_save (&parent /*, 1*/);
/*printf ("*** inode %d: increment link counter to %d\n", parent.number, parent.nlink);*/
}

/*
 * Create a device node.
 */
void add_device (lsxfs_t *fs, char *name, char *spec)
{
	lsxfs_inode_t dev;
	int majr, minr;
	char type;

	if (sscanf (spec, "%c%d:%d", &type, &majr, &minr) != 3 ||
	    (type != 'c' && type != 'b') ||
	    majr < 0 || majr > 255 || minr < 0 || minr > 255) {
		fprintf (stderr, "%s: invalid device specification\n", spec);
		fprintf (stderr, "expected c<major>:<minor> or b<major>:<minor>\n");
		return;
	}
	if (! lsxfs_inode_by_name (fs, &dev, name, 1, 0666 |
	    ((type == 'b') ? INODE_MODE_FBLK : INODE_MODE_FCHR))) {
		fprintf (stderr, "%s: device inode create failed\n", name);
		return;
	}
	dev.addr[0] = majr << 8 | minr;
	lsxfs_inode_save (&dev /*, 1*/);
}

/*
 * Copy file to filesystem.
 * When name is ended by slash as "name/", directory is created.
 */
void add_file (lsxfs_t *fs, char *name)
{
	lsxfs_inode_t file;
	FILE *fd;
	unsigned char data [512];
	char *p;
	int len;

	if (verbose) {
		printf ("Add: %s\n", name);
	}
	p = strrchr (name, '/');
	if (p && p[1] == 0) {
		*p = 0;
		add_directory (fs, name);
		return;
	}
	p = strrchr (name, '!');
	if (p) {
		*p++ = 0;
		add_device (fs, name, p);
		return;
	}
	fd = fopen (name, "r");
	if (! fd) {
		perror (name);
		return;
	}
	if (! lsxfs_file_create (fs, &file, name, 0777)) {
		fprintf (stderr, "%s: cannot create\n", name);
		return;
	}
	for (;;) {
		len = fread (data, 1, sizeof (data), fd);
        if (verbose > 3) {
            printf("read %d bytes from %s\n", len, name);
        }
		if (len < 0)
			perror (name);
		if (len <= 0)
			break;
		if (! lsxfs_file_write (&file, data, len)) {
			fprintf (stderr, "%s: write error\n", name);
			break;
		}
	}
	lsxfs_file_close (&file);
	fclose (fd);
}

void add_boot (lsxfs_t *fs)
{
	if (flat) {
		if (boot_sector2) {
			fprintf(stderr, "Secondary boot ignored\n");
		}
		if (boot_sector) {
			if (! lsxfs_install_single_boot (fs, boot_sector)) {
				fprintf (stderr, "%s: incorrect boot sector\n",
				boot_sector);
				return;
			}
			printf ("Boot sector %s installed\n", boot_sector);
		}
	} else if (boot_sector && boot_sector2) {
		if (! lsxfs_install_boot (fs, boot_sector,
		    boot_sector2)) {
			fprintf (stderr, "%s: incorrect boot sector\n",
				boot_sector);
			return;
		}
		printf ("Boot sectors %s and %s installed\n",
			boot_sector, boot_sector2);
	}
}


int write_file( char *name, unsigned char *buf, unsigned long num_bytes) {
    int fd = open (name, O_CREAT|O_RDWR);
    if (fd < 0) {
        fprintf (stderr, "%s: could not open file, return value %d\n",
                 name, fd);
        return 0;
    }
    int written = write (fd, buf, num_bytes);
    if (written != num_bytes) {
        fprintf (stderr, "%s: could not write file\n", name);
        return 0;
    }
    close (fd);
    return 1;
}

#define SECTORSIZE_BYTES 128

int read_sector( lsxfs_t *fs, unsigned long offset,
                 unsigned long num_bytes, unsigned char *buf ) {
    if (! lsxfs_seek (fs, offset)) {
        return 0;
    }
    int byte_offset=0;
    while (byte_offset < num_bytes) {
        if (! lsxfs_read8(fs, &(buf[byte_offset]))) {
            return 0;
        }
        printf("0%03o\n", buf[byte_offset]);
        byte_offset++;
    }
    return 1;
}
int extract_bootsectors(lsxfs_t *fs, char *basename) {
    unsigned char buf[SECTORSIZE_BYTES];
    if (verbose) {
        printf ("Extracting boot sectors\n");
    }

    /* first/single bootsector is at offset 0 */
    if (read_sector( fs, 0L, SECTORSIZE_BYTES, buf) == 0) {
        fprintf (stderr, "Error reading sector 0\n");
        return 0;
    }

    if (buf[1] != 000 && buf[0] != 0240) {
        fprintf (stderr, "Does not look like a boot sector No '0000 0240' as first two bytes %x %x\n",
                 buf[0], buf[1]);
    } else {
        printf("Looks like a boot sector\n");
    }
    printf ("Writing file %s\n", basename);
    if ( write_file(basename, buf, SECTORSIZE_BYTES) == 0) {
        fprintf (stderr, "Could not write bootsector file %s\n", basename);
        return 0;
    }

    int i;
    unsigned short track[10], sector[10];
    track[0] = 0;
    for (i=0; i<SECTORSIZE_BYTES; i++) {
        /*printf("0%03o\n", buf[i]);*/
        if (buf[i] == (unsigned char)0713 && i>0 && buf[i-1] == 000) {
            printf ("Found Opcode where list of secondary boot sectors follow\n");
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
        for (i=0; /*track[i] != 0 || sector[i] != 0*/i==0; i++ ) {
            long offset = (track[i] * 26 + sector[i] - 1) * 128;
            printf("Secondary sector, track=%d, sector=%d, offset=%lo, expected=%lo\n", track[i], sector[i],
                   offset, expected[i]);
            filename[0] = '\0';
            sprintf( filename, "%s-%d", basename, i );

            printf("Read sector at offset %lo\n", offset);
            if (read_sector( fs, offset, SECTORSIZE_BYTES, buf) == 0) {
                fprintf (stderr, "Error reading sector 0\n");
                return 0;
            }
            printf("Write sector to file %s\n", filename);
            if (write_file( filename, buf, SECTORSIZE_BYTES) ==0) {
                fprintf (stderr, "Could not write sector file %s\n", filename);
                return 0;
            }
        }
    }
    printf("All sector files written\n");
    return 1;
}


int main (int argc, char **argv)
{
	int i, key;
	lsxfs_t fs;
	lsxfs_inode_t inode;

    for (;;) {
        key = getopt_long (argc, argv, "vaxncfFSs:be:B:Vh",
                           program_options, 0);
        if (key == -1)
            break;
        switch (key) {
            case 'v':
                ++verbose;
                break;
            case 'a':
                ++add;
                break;
            case 'x':
                ++extract;
                break;
            case 'n':
                ++newfs;
                break;
            case 'c':
                ++check;
                break;
            case 'f':
                ++fix;
                break;
            case 'F':
                ++flat;
                break;
            case 'S':
                flat = 0;
                break;
            case 's':
                bytes = strtol (optarg, 0, 0);
                break;
            case 'b':
                boot_sector = optarg;
                break;
            case 'B':
                boot_sector2 = optarg;
                break;
            case 'e':
                boot_sector_file_base = optarg;
                ++extract_bootsector;
                break;
            case 'V':
                printf ("%s\n", program_version);
                return 0;
            case 'h':
                print_help (argv[0]);
                return 0;
            default:
                print_help (argv[0]);
                return -1;
        }
    }
    i = optind;
    if ((! add && i != argc-1) || (add && i >= argc-1) ||
        (extract + newfs + check + add > 1) ||
        (!flat && (! boot_sector ^ ! boot_sector2)) ||
        (newfs && bytes < 5120)) {
        print_help (argv[0]);
        return -1;
    }

	if (newfs) {
		/* Create new filesystem. */
		if (! lsxfs_create (&fs, argv[i], bytes)) {
			fprintf (stderr, "%s: cannot create filesystem\n", argv[i]);
			return -1;
		}
		printf ("Created filesystem %s - %ld bytes\n", argv[i], bytes);
		add_boot (&fs);
		lsxfs_close (&fs);
		return 0;
	}

	if (check) {
		/* Check filesystem for errors, and optionally fix them. */
		if (! lsxfs_open (&fs, argv[i], fix)) {
			fprintf (stderr, "%s: cannot open\n", argv[i]);
			return -1;
		}
		lsxfs_check (&fs);
		lsxfs_close (&fs);
		return 0;
	}

    /* Add or extract or info or boot update. */
    if (! lsxfs_open (&fs, argv[i],
                     (add != 0) || (boot_sector && boot_sector2))) {
        fprintf (stderr, "%s: cannot open\n", argv[i]);
        return -1;
    }

	if (extract) {
		/* Extract all files to current directory. */
		if (! lsxfs_inode_get (&fs, &inode, 1)) {
			fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
			return -1;
		}
		lsxfs_directory_scan (&inode, ".", extractor, (void*) stdout);
		lsxfs_close (&fs);
		return 0;
	}

    if (extract_bootsector) {
        extract_bootsectors(&fs, boot_sector_file_base);
        return 0;
    }

	add_boot (&fs);

	if (add) {
		/* Add files i+1..argc-1 to filesystem. */
		while (++i < argc)
			add_file (&fs, argv[i]);
		lsxfs_sync (&fs, 0);
		lsxfs_close (&fs);
		return 0;
	}

	/* Print the structure of flesystem. */
	lsxfs_print (&fs, stdout);
	if (verbose) {
		printf ("--------\n");
		if (! lsxfs_inode_get (&fs, &inode, 1)) {
			fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
			return -1;
		}
		if (verbose > 1) {
			lsxfs_inode_print (&inode, stdout);
			printf ("--------\n");
			printf ("/\n");
			print_inode_blocks (&inode, stdout);
		}
		lsxfs_directory_scan (&inode, "", scanner, (void*) stdout);
	}
	lsxfs_close (&fs);
	return 0;
}
