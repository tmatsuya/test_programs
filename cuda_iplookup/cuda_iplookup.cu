#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/types.h>
#ifdef __CUDACC__
#include <cuda.h>
#endif
#ifdef _OPENMP
#include <omp.h>
#endif
#include "rtdsc.h"

/* Takeshi MATSUYA macchan@sfc.wide.ad.jp */

//#define DEBUG
#define	MAX_ROUTING_TABLE	(1000000)
#define	FIB_FILE		"ipv4.txt"

//#define MAX_TEST_ROUTE	(1488095)	// Giga Ethernet
#define MAX_TEST_ROUTE	(14880950)	// 10G Ethernet

TimeWatcher tw;

struct _fib {
	unsigned int ip;
	unsigned char subnet;
	unsigned char gw;
} *fib;
int fib_count;

unsigned int *fib_table_h, *fib_table_d;	// FIB table

unsigned int *routing_table_h, *routing_table_d;// routing table
int routing_count;

unsigned int *results_h, *results_d;		// IP lookup results

void load_fib()
{
	FILE *fp;
	int ip1, ip2, ip3, ip4, subnet;
	if ((fp = fopen(FIB_FILE, "r")) == NULL ) {
		fprintf(stderr, "file %s can not open.", FIB_FILE);
		exit (-1);
	}
	fib_count = 0;
	while ( fscanf(fp, "%d.%d.%d.%d/%d", &ip1, &ip2, &ip3, &ip4, &subnet) != EOF) {
		if (ip1 > 255 || subnet > 24)
			continue;
		(fib+fib_count)->ip = (ip1<<24) | (ip2<<16) | (ip3<<8) | ip4;
		(fib+fib_count)->subnet = subnet;
		(fib+fib_count)->gw = rand() & 0xff;
#ifdef DEBUG
		printf("%d.%d.%d.%d/%d\n", ip1, ip2, ip3, ip4, subnet);
#endif
		++fib_count;
	}
	printf("Routing tables=%d\n", fib_count);
	fclose(fp);
}

void mapping_fib()
{
	int i, subnet;
	unsigned int ip_start, ip_end, ip, addr;
	for (subnet=2; subnet<=24; ++subnet) {
#ifdef DEBUG
		printf("subnet=%d\n", subnet);
#endif
		for (i=0; i<fib_count; ++i) {
			if (fib[i].subnet == subnet) {
				ip_start = (fib+i)->ip & ~((unsigned int)(1<<(32-subnet))-1);
				ip_end   = (fib+i)->ip |  ((unsigned int)(1<<(32-subnet))-1);
#ifdef DEBUG
				printf("start=%08X, end=%08X\n", ip_start, ip_end);
#endif
				for ( ip=(ip_start>>8); ip<=(ip_end>>8); ++ip) {
					addr = (ip & 0xffffff);
					*(fib_table_h + addr) = (fib+i)->gw;
				}

			}
		}
	}
}

#ifdef __CUDACC__
__global__ void iplookup_gpu(unsigned int *fib_table, unsigned int *routing_table, unsigned int *results, int routing_count)
{
	int i;
	int total = gridDim.x * blockDim.x;			// number of total thrads
	int idx = blockIdx.x * blockDim.x + threadIdx.x;	// get current thread ID

	for (i=routing_count*idx/total; i<(routing_count*(idx+1)/total); ++i)
		results[i] = fib_table[routing_table[i]>>8];
}
#endif

void iplookup_host(unsigned int *fib_table, unsigned int *routing_table, unsigned int *results, int routing_count)
{
	int i;

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (i=0; i<routing_count; ++i)
		results[i] = fib_table[routing_table[i]>>8];
#ifdef _OPENMP
#pragma barrier
#endif
}

int main(int argc, char **argv)
{
	int i, loss;

	routing_count = MAX_TEST_ROUTE;

#ifdef _OPENMP
//omp_set_num_threads(1);		// max number of threads
#pragma omp parallel
{
	printf("thread_num=%d of %d\n", omp_get_thread_num(), omp_get_num_threads());
}
#endif
#ifdef __CUDACC__
	dim3 blocks(128);
	dim3 threads(1);
#endif

	get_cpu_cycle_per_sec();

	// memory allocation and clear
	fib = (struct _fib *)malloc( sizeof(struct _fib)*MAX_ROUTING_TABLE);
	fib_table_h = (unsigned int *)malloc( sizeof(int)*16*1024*1024);
	routing_table_h = (unsigned int *)malloc( sizeof(int)*MAX_TEST_ROUTE);
	results_h = (unsigned int *)malloc( sizeof(int)*MAX_TEST_ROUTE);
	bzero( fib_table_h, sizeof(int)*16*1024*1024);
	bzero( routing_table_h, sizeof(int)*MAX_TEST_ROUTE);
#ifdef __CUDACC__
	cudaMalloc((void **)&fib_table_d, sizeof(int)*16*1024*1024);
	cudaMalloc((void **)&routing_table_d, sizeof(int)*MAX_TEST_ROUTE);
	cudaMalloc((void **)&results_d, sizeof(int)*MAX_TEST_ROUTE);
#endif

	load_fib();
	mapping_fib();
	for (i=0; i<routing_count; ++i) {
		routing_table_h[i] = (rand() & 0xffff) << 16; //(fib+(rand() % fib_count))->ip;
	}

	start(&tw);
	end(&tw);
	loss = tw.end - tw.start;

	start(&tw);
	iplookup_host(fib_table_h, routing_table_h, results_h, routing_count);
	end(&tw);

	tw.end -= loss;
	printf("HOST\n");
	print_time_sec(&tw);

#ifdef __CUDACC__
	cudaMemcpy(fib_table_d, fib_table_h, sizeof(int)*16*1024*1024, cudaMemcpyHostToDevice);
	cudaMemcpy(routing_table_d, routing_table_h, sizeof(int)*MAX_TEST_ROUTE, cudaMemcpyHostToDevice);
	cudaMemset( results_d, 0, sizeof(int)*MAX_TEST_ROUTE);

	start(&tw);
	iplookup_gpu <<< blocks, threads >>>(fib_table_d, routing_table_d, results_d, routing_count);
	cudaThreadSynchronize();
	end(&tw);

	tw.end -= loss;
	printf("GPU\n");
	print_time_sec(&tw);

	cudaMemcpy(results_h, results_d, sizeof(int)*MAX_TEST_ROUTE, cudaMemcpyDeviceToHost);
#endif


#ifdef	DEBUG
	for (i=0; i<routing_count; ++i)
//		if (results_h[i] == 0)
		printf( "%08X = %d\n", routing_table_h[i], results_h[i]);
#endif

	free(fib);
	free(fib_table_h);
	free(routing_table_h);
	free(results_h);

#ifdef __CUDACC__
	cudaFree(fib_table_d);
	cudaFree(routing_table_d);
	cudaFree(results_d);
#endif

	exit(EXIT_SUCCESS);
}

