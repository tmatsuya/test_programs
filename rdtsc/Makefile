SHELL = /bin/sh

OBJ = main.o
CFLAGS = -Wall -O
CC = gcc
INCLUDE =
LIBS =

all: rdtsc

rdtsc: ${OBJ}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJ} ${LIBS}

clean:
	-rm -f *.o core *.core rdtsc

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

