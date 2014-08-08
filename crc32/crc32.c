#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

unsigned char rev8(unsigned char c)
{
  unsigned char r = 0;
  int i;
  for (i = 0; i < 8; i++)
    r |= (c & (1<<i)) ? (1<<(7-i)) : 0;
  return r;
}

unsigned int rev32(unsigned int c)
{
  unsigned int r = 0;
  int i;
  for (i = 0; i < 32; i++)
    r |= (c & (1<<i)) ? (1<<(31-i)) : 0;
  return r;
}

int main(int argc, char *argv[])
{
  int c0, c1, i;
  char s[3];

  unsigned int value;
  unsigned int crc0, crc1;

  crc0 = 0xffffffff;

  while (1) {
    while (1) {
      c0 = fgetc(stdin);
      if ((c0 == EOF) || (c0 == '!')) goto exit_loop;
      if (isxdigit(c0)) break;
    }
    while (1) {
      c1 = fgetc(stdin);
      if ((c1 == EOF) || (c1 == '!')) goto exit_loop;
      if (isxdigit(c1)) break;
    }
    s[0] = c0;
    s[1] = c1;
    s[2] = '\0';
    value = strtol(s, NULL, 16);
    for (i = 0; i < 8; i++) {

#define VALUE 0x04c11db7
#if 0
      crc0 = ((crc0 << 1) | (value & 1)) ^ ((crc0 & (1<<31)) ? VALUE : 0);
#else
      crc0 = (crc0 << 1) ^ ((((crc0 >> 31) & 1) ^ (value & 1)) ? VALUE : 0);
#endif

      value >>= 1;
    }
  }

exit_loop:

  crc1  = rev8(crc0 >> 24) << 24;
  crc1 |= rev8(crc0 >> 16) << 16;
  crc1 |= rev8(crc0 >>  8) <<  8;
  crc1 |= rev8(crc0);

#define XOR 0xffffffff
//  printf("crc0 = %08x\n", crc0 ^ XOR);
  printf("Ethernet FCS = %08X\n", crc1 ^ XOR);

  exit (0);
}


