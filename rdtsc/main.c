#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include "rdtsc.h"

TimeWatcher tw;

int main()
{
	get_rdtsc_cycle_per_sec();

	start(&tw);
	end(&tw);

	print_time_sec(&tw);

	return 0;
}
