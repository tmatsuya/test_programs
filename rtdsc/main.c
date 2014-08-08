#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include "rtdsc.h"

TimeWatcher tw;

int main()
{
	get_cpu_cycle_per_sec();

	start(&tw);
	end(&tw);

	print_time_sec(&tw);

	return 0;
}
