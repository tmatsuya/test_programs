#include <stdio.h>

main()
{
	int a;
	while ( (a=getchar()) > 0)
		putchar(++a);
}

