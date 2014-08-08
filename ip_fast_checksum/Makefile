SHELL = /bin/sh

OBJS = ip_fast_checksum.o
#CFLAGS = -Wall -O -S
CFLAGS = -Wall -O
CC = gcc
INCLUDE =
LIBS =

all: ip_fast_checksum

ip_fast_checksum: ${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}
clean:
	-rm -f *.o core *.core ip_fast_checksum

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

