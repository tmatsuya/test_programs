#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

unsigned long long rtdsc_cycles_per_sec;

typedef struct TimeWatcher
{
	unsigned long long start;
	unsigned long long end;
} TimeWatcher;


#ifdef __x86_64__
static __inline unsigned long long int rdtsc(void)
{
	unsigned a, d;

	__asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

	return ((unsigned long long)a) | (((unsigned long long)d) << 32);;
}
#endif

#ifdef __aarch64__
static __inline long long int rdtsc() {
	long long int cntvct;

	asm volatile ("mrs %0, cntvct_el0; " : "=r"(cntvct) :: "memory");

	return cntvct;
}
#endif

static __inline unsigned long long get_rtdsc_cycle_per_sec()
{
	unsigned long long startnsec;
	unsigned long long start, now, diff;
	struct timespec ts;


	clock_gettime(CLOCK_REALTIME, &ts);
	startnsec = ts.tv_nsec;
	start = rdtsc();

	do {
		clock_gettime(CLOCK_REALTIME, &ts);
		if (startnsec > ts.tv_nsec)
			ts.tv_nsec += 1000000000;
	} while (ts.tv_nsec < (startnsec + 100000000));
	
	now = rdtsc();
	diff = now - start;

	rtdsc_cycles_per_sec = diff * 10;
	return rtdsc_cycles_per_sec = diff * 10;
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
	printf("CPU Frequency:%lldMHz (%lld cycles/sec)\n", rtdsc_cycles_per_sec / 1000000, rtdsc_cycles_per_sec);
	printf("time:%llu ns, CPU cycles:%llu\n", (tw->end - tw->start) * 1000000000 / rtdsc_cycles_per_sec, tw->end - tw->start);
}
