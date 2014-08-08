#include <stdio.h>

int main(void) {
	unsigned long sum;
	unsigned short i;
	unsigned short packet[] = { 0x4500, 0x0400, 0x019b, 0x0000, 0x8011, 0x0000, 0xc0a8, 0x0101, 0xe000, 0x0001 };

	sum=0;
	for ( i=0; i<(sizeof(packet)/sizeof(unsigned short)); ++i) {
		sum += packet[i];
	}
	sum = ( sum & 0xffff ) + ( sum >> 16 );
	sum ^= 0xffff;

	printf("SUM=%4X\n", sum);
}
