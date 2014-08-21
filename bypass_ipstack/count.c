#include <stdio.h>

int main() {
	int start,end,i;
	scanf("%d,%d,%d", &start,&i,&i);
	sleep(10);
	scanf("%d,%d,%d", &end,&i,&i);
	printf("start=%lu\n", start);
	printf("end  =%lu\n", end);
	printf("diff =%lu\n", end-start);
}

