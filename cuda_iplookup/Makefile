SHELL = /bin/sh

# see http://archive.routeviews.org
BGPDATA=http://archive.routeviews.org/bgpdata/2014.04/RIBS/rib.20140423.1000.bz2

SRC1 = cuda_iplookup.cu
SRC2 = cuda_iplookup.c
#NVCFLAGS = -O -gencode arch=compute_30,code=sm_30
#NVCFLAGS = -O -gencode arch=compute_13,code=sm_13
NVCFLAGS = -O
CFLAGS = -O -fopenmp
CC = gcc
NVCC = nvcc
INCLUDE =
LIBS =
NVLIBS = -lcuda -lcudart

all: openmp_iplookup cuda_iplookup

ipv4.txt:
	wget ${BGPDATA}
	bgpdump -m rib.*.bz2 | grep -v ":" | cut -d '|' -f 6 | sort | uniq > ipv4.txt

${SRC2}: ${SRC1}
	if [ ! -f ${SRC2} ] ; then \
		ln -s ${SRC1} ${SRC2}; \
	fi

openmp_iplookup: ${SRC2} ipv4.txt
	${CC} ${CFLAGS} ${INCLUDES} -o $@ ${SRC2} ${LIBS}

cuda_iplookup: ${SRC1} ipv4.txt
	${NVCC} ${NVCFLAGS} ${INCLUDES} -o $@ ${SRC1} ${NVLIBS}

clean:
	rm -f *.o core *.core cuda_iplookup openmp_iplookup ${SRC2}

.SUFFIXES: clean
