#include <stdio.h>

int main() {
	int start,end;
	scanf("%d", &start);
	sleep(10);
	scanf("%d", &end);
	printf("start=%lu\n", start);
	printf("end  =%lu\n", end);
	printf("diff =%lu\n", end-start);
}

