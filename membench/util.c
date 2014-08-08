#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "util.h"

// =================
// measure time(sec)
// =================

double gettimeofday_sec()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec * 1e-6;
}

void start(TimeWatcher* tw)
{
	tw->start = gettimeofday_sec();
}

void end(TimeWatcher* tw)
{
	tw->end = gettimeofday_sec();
}

void print_time_sec(TimeWatcher* tw)
{
	printf("%10.10f(sec)\n", tw->end - tw->start);
}
