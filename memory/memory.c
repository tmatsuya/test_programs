#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <curses.h>
#include <string.h>

// Mac OS:
// sudo nvram boot-args="kmem=1"
//

#ifdef __APPLE__
#define	MEM_DEVICE	"/dev/kmem"
#else
#define	MEM_DEVICE	"/dev/mem"
#endif

WINDOW *win;
int mem_fd;
unsigned long long top_addr, input_addr;

int display_memory(unsigned long long addr)
{
	int i, y, x;
	unsigned char c;
	mvwaddstr(win, 1, 2, "Address");
	mvwaddstr(win, LINES-2, 1, "Exit:[Ctrl+X] PageDown:[Ctrl+F] PageUp:[Ctrl+B] Redraw On/Off:[Ctrl+L] Redraw:[ESC]");
	for (i=0; i<16; ++i) {
		mvwprintw(win, 1, i*3+18, "+%X", i);
	}
	for (x=1; x<COLS-2; ++x)
		mvwaddch(win, 2, x, '-');

	addr &= 0xfffffffffffffff0;
	for (y=0; y<LINES-5; ++y) {
		mvwprintw(win, y+3, 1, "%016llX", (addr + (y << 4) ));
		for (x=0; x<16; ++x) {
			lseek(mem_fd, (unsigned long long)addr+(unsigned long long)(y<<4)+(unsigned long long)x, SEEK_SET);
			read(mem_fd, &c, 1);
			c &= 0xff;
			// hex code
			mvwprintw(win, y+3, x*3+18, "%02X", (int)c);
			if (x == 7)
				waddch(win, '-');
			// ascii data
			mvwaddch(win, y+3, x+66, (c>=' ' && c<=0x7f) ? c : '.');

		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int c;
	int y, x, n;
	int autoredraw = 1;
	int idata;
	char device_file[256];

	strcpy(device_file, MEM_DEVICE);

	if (argc == 2) {
		input_addr = strtoull(argv[1], NULL, 0);
	} else if (argc == 3) {
		strcpy(device_file, argv[1]);
		input_addr = strtoull(argv[2], NULL, 0);
	} else {
		printf("usage:\t%s [file] memory_address\n", argv[0]);
		printf("\tmemory 0xb80000\n");
		printf("\tmemory /dev/kmem 0xb80000\n");
		printf("file: default /dev/mem0\n");
		exit(0);
	}

	top_addr = input_addr & ~0xf;
	x = input_addr & 0xf;
	y = n = idata = 0;

	if ((mem_fd = open(device_file, O_RDWR) ) < 0 ) {
		fprintf( stderr, "cannot open file '%s'\n", device_file);
		fprintf( stderr, "if your OS is Mac OS X then; you have to type \'sudo nvram boot-args=\"kmem=1\"\' and reboot\n");
		return(-1);
	}

	initscr();

	noecho();
	cbreak();

	win = newwin(LINES,COLS-1,0,0);
	box(win,'|','-');
	keypad(win, TRUE);
	wtimeout(win, 500);

	display_memory(top_addr);

	do {
		wrefresh(win);
		wmove(win,(3+y),18+(3*x)+n);
		c = wgetch(win);
		switch (c) {
			case KEY_UP:
			case 'k':
			case 'K':
				if (y > 0) {
					n = 0;
					--y;
				} else {
					top_addr -= 0x10;
					display_memory(top_addr);
				}
				break;
			case KEY_DOWN:
			case 'j':
			case 'J':
				if (y < LINES-6) {
					n = 0;
					++y;
				} else {
					top_addr += 0x10;
					display_memory(top_addr);
				}
				break;
			case KEY_LEFT:
			case 'h':
			case 'H':
				if (x > 0) {
					n = 0;
					--x;
				}
				break;
			case KEY_RIGHT:
			case 'l':
			case 'L':
				if (x < 15) {
					n = 0;
					++x;
				}
				break;
			// Page Down
			case 'F'-0x40:
				top_addr += (LINES-6)*0x10; 
				display_memory(top_addr);
				break;
			// Page Up
			case 'B'-0x40:
				top_addr -= (LINES-6)*0x10; 
				display_memory(top_addr);
				break;
			// Auto Redraw
			case -1:
				if (autoredraw)
					display_memory(top_addr);
				break;
			// Redraw
			case 0x1b:
				display_memory(top_addr);
				break;
			// Redraw On/Off
			case 'L'-0x40:
				autoredraw = !autoredraw;
				display_memory(top_addr);
				break;
			default:
				if (isxdigit(c)) {
					if (islower(c))	// upper
						c -= 0x20;
					if (n == 0) {	// upper 4bit
						waddch(win, c);
						if (c >= '0' && c <= '9')
							idata = c - '0';
						else
							idata = c - 'A' + 10;
						n = 1;
					} else {	// lower 4bit
						waddch(win, c);
						idata = (idata << 4);
						if (c >= '0' && c <= '9')
							idata |= c - '0';
						else
							idata |= c - 'A' + 10;
						// write memory
						lseek(mem_fd, (unsigned long long)top_addr+(unsigned long long)(y<<4)+(unsigned long long)x, SEEK_SET);
						write(mem_fd, &idata, 1);

						// skip a byte
						n = 0;
						if (x < 15) {
							++x;
						} else {
							x = 0;
							if (y < LINES-6) {
								++y;
							}
						}
					}
				}
		}
	} while(c != 'X'-0x40);

	wclear(win);
	wrefresh(win);
	endwin();
	exit(0);
}
