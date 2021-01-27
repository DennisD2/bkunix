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
int add;
int newfs;
int check;
int fix;
int flat = 1;
unsigned long bytes;
char *boot_sector;
char *boot_sector2;

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
        { "add",	no_argument,		0,	'a' },
        { "extract",	no_argument,		0,	'x' },
        { "check",	no_argument,		0,	'c' },
        { "fix",	no_argument,		0,	'f' },
        { "new",	no_argument,		0,	'n' },
        { "size",	required_argument,	0,	's' },
        { "boot",	required_argument,	0,	'b' },
        { "boot2",	required_argument,	0,	'B' },
        { "flat",	no_argument,		0,	'F' },
        { "shuffle",	no_argument,		0,	'S' },
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
	printf ("    %s --check [--fix] filesys.bkd\n", progname);
	printf ("    %s --new --size=bytes filesys.bkd\n", progname);
	printf ("\n");
	printf ("Options:\n");
	printf ("  -a, --add          Add files to filesystem\n");
	printf ("  -x, --extract      Extract all files\n");
	printf ("  -c, --check        Check filesystem, use -c -f to fix\n");
	printf ("  -f, --fix          Fix bugs in filesystem\n");
	printf ("  -n, --new          Create new filesystem, -s required\n");
	printf ("  -s NUM, --size=NUM Size in bytes for created filesystem\n");
	printf ("  -b FILE, --boot=FILE Boot sector, -B required if -S\n");
	printf ("  -B FILE, --boot2=FILE Secondary boot sector, -b required\n");
	/* printf ("  -F, --flat         Flat mode, no sector remapping (default)\n"); */
	/* printf ("  -S, --shuffle      Shuffle mode, remap 128-byte sectors\n"); */
	printf ("  -v, --verbose      Print verbose information\n");
	printf ("  -V, --version      Print version information and then exit\n");
	printf ("  -h, --help         Print this message\n");
	printf ("\n");
	printf ("Report bugs to \"%s\".\n", program_bug_address);
}

void print_file (lsxfs_inode_t *file,
	char *dirname, char *filename, FILE *out)
{
	fprintf (out, "%s/%s", dirname, filename);
	switch (file->mode & INODE_MODE_FMT) {
	case INODE_MODE_FDIR:
		fprintf (out, "/\n");
		break;
	case INODE_MODE_FCHR:
		fprintf (out, " - char %d %d\n",
			file->addr[0] >> 8, file->addr[0] & 0xff);
		break;
	case INODE_MODE_FBLK:
		fprintf (out, " - block %d %d\n",
			file->addr[0] >> 8, file->addr[0] & 0xff);
		break;
	default:
		fprintf (out, " - %lu bytes\n", file->size);
		break;
	}
}

void extract_file (lsxfs_inode_t *file, char *path)
{
	int fd, n;
	unsigned long offset;
	unsigned char data [512];

	fd = open (path, O_CREAT | O_WRONLY, file->mode & 0x777);
	if (fd < 0) {
		perror (path);
		return;
	}
	for (offset = 0; offset < file->size; offset += 512) {
		n = file->size - offset;
		if (n > 512)
			n = 512;
		if (! lsxfs_file_read (file, offset, data, n)) {
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

void extractor (lsxfs_inode_t *dir, lsxfs_inode_t *file,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	if (verbose)
		print_file (file, dirname, filename, out);

	if ((file->mode & INODE_MODE_FMT) != INODE_MODE_FDIR &&
	    (file->mode & INODE_MODE_FMT) != 0)
		return;

	path = alloca (strlen (dirname) + strlen (filename) + 2);
	strcpy (path, dirname);
	strcat (path, "/");
	strcat (path, filename);

	if ((file->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		if (mkdir (path, 0775) < 0)
			perror (path);
		/* Scan subdirectory. */
		lsxfs_directory_scan (file, path, extractor, arg);
	} else {
		extract_file (file, path);
	}
}

void scanner (lsxfs_inode_t *dir, lsxfs_inode_t *file,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	print_file (file, dirname, filename, out);

	if (verbose) {
		lsxfs_inode_print (file, out);
		printf ("--------\n");
	}
	if ((file->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		/* Scan subdirectory. */
		path = alloca (strlen (dirname) + strlen (filename) + 2);
		strcpy (path, dirname);
		strcat (path, "/");
		strcat (path, filename);
		lsxfs_directory_scan (file, path, scanner, arg);
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
	/*p = strrchr (name, '!');
	if (p) {
		*p++ = 0;
		add_device (fs, name, p);
		return;
	}*/
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
		printf ("read %d bytes from %s\n", len, name);
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

int main (int argc, char **argv)
{
	int i, key;
	lsxfs_t fs;
	lsxfs_inode_t inode;

    for (;;) {
        key = getopt_long (argc, argv, "vaxncfFSs:b:B:Vh",
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
		if (boot_sector && boot_sector2) {
			if (! lsxfs_install_boot (&fs, boot_sector,
			    boot_sector2)) {
				fprintf (stderr, "%s: incorrect boot sector\n",
					boot_sector);
				return -1;
			}
			printf ("Boot sectors %s and %s installed\n",
				boot_sector, boot_sector2);
		}
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
		lsxfs_inode_print (&inode, stdout);
		if (verbose > 1) {
			printf ("--------\n");
			lsxfs_directory_scan (&inode, "", scanner, (void*) stdout);
		}
	}
	lsxfs_close (&fs);
	return 0;
}
