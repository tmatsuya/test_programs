#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/time.h>
#include <sys/types.h>

/* Takeshi MATSUYA macchan@sfc.wide.ad.jp */

unsigned long long cpu_cycles_per_sec;

typedef struct TimeWatcher
{
	unsigned long long start;
	unsigned long long end;
} TimeWatcher;

static __inline unsigned long long int rdtsc(void)
{
  unsigned a, d;

  __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

  return ((unsigned long long)a) | (((unsigned long long)d) << 32);;
}

static __inline unsigned long long get_cpu_cycle_per_sec()
{
	unsigned long long startusec;
	unsigned long long start, now, diff;
	struct timeval tv;

	start = rdtsc();
	gettimeofday(&tv, NULL);
	startusec = tv.tv_sec * 1000000 + tv.tv_usec;

	do {
		gettimeofday(&tv, NULL);
	} while ((tv.tv_sec * 1000000 + tv.tv_usec) < (startusec + 1000000));
	
	now = rdtsc();
	diff = now - start;

	cpu_cycles_per_sec = diff;
	return diff;
}

static __inline void start(TimeWatcher* tw)
{
	 tw->start = rdtsc();
}

static __inline void end(TimeWatcher* tw)
{
	 tw->end = rdtsc();
}

void print_time_sec(TimeWatcher* tw)
{
	printf("CPU Frequency:%lldMHz (%lld cycles/sec)\n", cpu_cycles_per_sec / 1000000, cpu_cycles_per_sec);
	printf("time:%llu ns, CPU cycles:%llu\n", (tw->end - tw->start) * 1000000000 / cpu_cycles_per_sec, tw->end - tw->start);
}

/** refer "linux/arch/x86/include/asm/checksum_64.h"
 * http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/arch/x86/include/asm/checksum_64.h
 * ip_fast_csum - Compute the IPv4 header checksum efficiently.
 * iph: ipv4 header
 * ihl: length of header / 4
 */
static inline unsigned short ip_fast_csum(const void *iph, unsigned int ihl)
{
        unsigned int sum;

        asm("  movl (%1), %0\n"
            "  subl $4, %2\n"
            "  jbe 2f\n"
            "  addl 4(%1), %0\n"
            "  adcl 8(%1), %0\n"
            "  adcl 12(%1), %0\n"
            "1: adcl 16(%1), %0\n"
            "  lea 4(%1), %1\n"
            "  decl %2\n"
            "  jne      1b\n"
            "  adcl $0, %0\n"
            "  movl %0, %2\n"
            "  shrl $16, %0\n"
            "  addw %w2, %w0\n"
            "  adcl $0, %0\n"
            "  notl %0\n"
            "2:"
/* Since the input registers which are loaded with iph and ihl
 *            are modified, we must also specify them as outputs, or gcc
 *            will assume they contain their original values. */
            : "=r" (sum), "=r" (iph), "=r" (ihl)
            : "1" (iph), "2" (ihl)
            : "memory");
        return (unsigned short)sum;
}


TimeWatcher tw;


int main(int argc, char **argv)
{
	int checksum, loss, i;
	char buf[16] __attribute__((aligned(64))) = {};

	get_cpu_cycle_per_sec();

	for (i=0; i<sizeof(buf);++i)
		buf[i] = i;


	start(&tw);
	end(&tw);
	loss = tw.end - tw.start;

	start(&tw);
	checksum = ip_fast_csum(buf, sizeof(buf));
	end(&tw);

	tw.end -= loss;
	printf("checksum=%X\n", checksum);
	print_time_sec(&tw);

	exit(EXIT_SUCCESS);
}

