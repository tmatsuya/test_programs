SHELL = /bin/sh

OBJ = membench.o
CFLAGS = -Wall
CC = gcc
INCLUDE =
LIBS =

all: membench

membench: ${OBJ}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJ} ${LIBS}

clean:
	-rm -f *.o core *.core membench

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

