SHELL = /bin/sh

OBJS = memory.o
#CFLAGS = -Wall -O -S
CFLAGS = -Wall -O
CC = gcc
INCLUDE =
LIBS = -lcurses

all: memory

memory: ${OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${OBJS} ${LIBS}

install:
	install -m 755 memory /usr/bin

clean:
	-rm -f *.o core *.core memory

.c.o:
	${CC} ${CFLAGS} ${INCLUDES} -c $<

