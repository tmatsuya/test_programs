Usage:
	./player575 </dev/tcp/macchan.sfc.wide.ad.jp/8081  ... from network
	./player575 <lucky_sttar.bin                       ... from file

How to build:
	sudo yum install libcaca
	git clone git://source.ffmpeg.org/ffmpeg.git ffmpeg
	cd ffmpeg
	make distclean
	./configure --enable-libfdk-aac
	make

How to convert AAA movie:
	./ffmpeg -i input_file -pix_fmt rgb24 -window_size 80x25 -f caca -driver ncurses - > output.txt

Server: If you need server program, below commands are example on server side.
	nc -l -k 8081 -e "/usr/bin/cat movie1.bin"
	nc -l -k 8082 -e "/usr/bin/cat movie2.bin"

