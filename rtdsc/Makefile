SHELL = /bin/sh

OBJ = main.o
CFLAGS = -Wall -O
CC = gcc
INCLUDE =
LIBS =

all: rtdsc

rtdsc: ${OBJ}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJ} ${LIBS}

clean:
	-rm -f *.o core *.core rtdsc

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

