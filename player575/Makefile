all: player575 encode lucky_star.bin madoka.bin haruhi.bin

player575: player575.s
	as --64 -o player575.o player575.s 
	ld -s -o player575 player575.o
	echo "usage1: ./player575 < madoka.bin or lucky_star.bin"
	echo "usage2: ./player575 </dev/tcp/macchan.sfc.wide.ad.jp/8081 or 8082"

encode: encode.c
	gcc -O -o encode encode.c

lucky_star.bin: lucky_star.bin.bz2
	bunzip2 -fk lucky_star.bin.bz2

madoka.bin: madoka.bin.bz2
	bunzip2 -fk madoka.bin.bz2

haruhi.bin: haruhi.bin.bz2
	bunzip2 -fk haruhi.bin.bz2

clean:
	rm -f player575.o player575 encode.o encode
