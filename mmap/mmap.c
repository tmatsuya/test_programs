#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#define	MEM_DEVICE	"/dev/mem"
int main(int argc,char **argv)
{
	int *mmaped;
	int i, j, d, r, fd;
	unsigned int st,len=0x400000,poff;
//	unsigned int st,len=0x1000,poff;
	if (argc!=2) {
		fprintf(stderr,"%s <start addr>\n", argv[0]);
		return 1;
	}
	st=strtoul(argv[1],NULL,16);
	poff=st % 4096;
	if ((fd=open(MEM_DEVICE,O_RDWR)) <0) {
		fprintf(stderr,"cannot open %s\n",MEM_DEVICE);
		return 1;
	}
	fprintf(stderr,"mmap: start %08X	len:%08X\n",st-poff,len+poff);
	mmaped = mmap(0, len+poff, PROT_READ|PROT_WRITE, MAP_SHARED, fd, st-poff);
	if(mmaped==MAP_FAILED) {
		fprintf(stderr,"cannot mmap\n");
		return 1;
	}

	printf("writing %d bytes\n", len);
	for ( i=0; i < len/4; ++i ) {
		*(mmaped + i) = i;
	}
for (j=0; j<10;++j) {
	printf("count=%d, sequential verifing %d bytes\n", j, len);
	for ( i=0; i < len/4; ++i ) {
		d = *(mmaped + i);
		if (d != i) {
			printf("Memory unmatch addr=%x write=%x verify=%x\n", i, i, d);
		}
	}
}
for (j=0; j<10;++j) {
	printf("count=%d, random verifing %d bytes\n", j, len);
	for ( i=0; i < len/4; ++i ) {
		r = rand() % (len/4);
		d = *(mmaped + r);
		if (d != r) {
			printf("Memory unmatch addr=%x write=%x verify=%x\n", r, r, d);
		}
	}
}

//	write(1,mmaped+poff,len);
	munmap(mmaped,len);
	close(fd);
	return 0;
}
