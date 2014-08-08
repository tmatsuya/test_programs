#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <arpa/inet.h>

//#include "util.h"
#include "../rtdsc/rtdsc.h"

#define	USE_MEMCPY
//#define DEBUG

#define	MEM_DEVICE	"/dev/mem"

TimeWatcher tw;

int main(int argc,char **argv)
{
	unsigned int *mmapped;
	char *ptr;
#ifdef USE_MEMCPY
	char *buf;
#endif
	int i, j, fd;
	unsigned long long st=0xe9000000,len=0x8000,poff;
	if (argc!=3) {
		fprintf(stderr,"%s address r|w\n", argv[0]);
		return 1;
	}

	get_cpu_cycle_per_sec();

	st = strtoll( argv[1], NULL, 16);
	printf("%llX\n", st);
	poff=st % 4096;
#ifdef USE_MEMCPY
	buf = malloc(len);
#endif
	if ((fd=open(MEM_DEVICE,O_RDWR)) <0) {
		fprintf(stderr,"cannot open %s\n",MEM_DEVICE);
		return 1;
	}
	fprintf(stdout,"mmap: start %012llX len:%08llX Total=%dMB\n",st-poff,len+poff, (int)len/1000);

	mmapped = mmap(0, len+poff, PROT_READ|PROT_WRITE, MAP_SHARED, fd, st-poff);
	if(mmapped==MAP_FAILED) {
		fprintf(stderr,"cannot mmap\n");
		return 1;
	}
	start(&tw);
	if ( !strcmp( argv[2], "w") ) {
		for ( i=0; i < 1000; ++i )
#ifdef USE_MEMCPY
			memcpy(mmapped, buf, len);
#else
			for ( j=0; j < len/4; ++j ) {
				*(int *)(mmapped  + j) = j;
			}
#endif
	}
	if ( !strcmp( argv[2], "r") ) {
		for ( i=0; i < 1000; ++i )
#ifdef USE_MEMCPY
			memcpy(buf, mmapped, len);
#else
			for ( j=0; j < len/4; ++j ) {
				ptr = (char *)(mmapped + j);
				if (((int)ptr & 63) == 0)
					asm volatile ("prefetcht0 %0" :: "m" (mmapped[j]) : "memory");
				if ( *(int *)(mmapped  + j) != j )
					fprintf(stderr,"data error j=%x,mem=%x\n", j, *(int *)(mmapped + j));
			}
#endif
	}
	end(&tw);
	print_time_sec(&tw);
	munmap(mmapped,len);
	close(fd);
	return(0);
}
