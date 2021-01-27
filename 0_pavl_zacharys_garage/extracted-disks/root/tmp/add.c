/* ADD takes two file arguments, a source file <src>
	and a destination file <dst>.  the contents of <src> are
	appended to <dst>.  If <dst> does not exist it will be
	created.  P. Benson 17-Oct-78.

	ex:	add stm/ahb.d stm/bah.d

*/
main(argc,argv)
int argc;
char *argv[];
{
int buffer[9000];
int fd,i,nbytes,od,obytes;
/* open the file */

	if (argc != 3)	/* check for correct number of args  */
		{printf("\nUsage: add <src> <dst>\n");exit(0);}

	if((fd = open(argv[1],0)) == -1)
			{printf("\ncan't open %s\n",argv[1]);
			exit(0);}

	if((nbytes = read(fd,buffer,9000)) == -1)
		{printf("\ncan't read %s\n",argv[1]);exit(0);}

	printf("%d bytes read from %s\n",nbytes,argv[1]);
	if((od = open(argv[2],2)) == -1)
		if ((od = creat(argv[2],0600)) == -1)
			{printf("\ncan't open %s",argv[2]);exit(0);}

	seek(od,0,2);	/* wind to end of output file */
	obytes = write(od,buffer,nbytes);
	printf("%d bytes put out on %s\n\n",obytes,argv[2]);
	close(fd);
	close(od);
}
