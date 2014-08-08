#ifndef UTIL_H_INCLUDE
#define UTIL_H_INCLUDE

// =================
// measure time(sec)
// =================

typedef struct TimeWatcher
{
	double start;
	double end;
} TimeWatcher;

void start(TimeWatcher* tw);
void end(TimeWatcher* tw);
void print_time_sec(TimeWatcher* tw);

#endif // #ifndef UTIL_H_INCLUDE
