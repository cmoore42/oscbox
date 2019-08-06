#include "display.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int disp_fd;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
uint16_t *bits;
int font_height;
int font_width;
int charsize;
char *font_rows;

// #define FONT_FILENAME "zap-light20.psf"
#define FONT_FILENAME "Tamzen8x15.psf"

static void load_fonts();

struct psf1_header {
	unsigned char magic[2];	    /* Magic number */
	unsigned char mode;	    /* PSF font mode */
	unsigned char charsize;	    /* Character size */
};

struct psf2_header {
	unsigned char magic[4];
	unsigned int version;
	unsigned int headersize;    /* offset of bitmaps in file */
	unsigned int flags;
	unsigned int length;	    /* number of glyphs */
	unsigned int charsize;	    /* number of bytes for each character */
	unsigned int height, width; /* max dimensions of glyphs */
	/* charsize = height * ((width + 7) / 8) */
};

void disp_open() {
	int tty_fd;

	tty_fd = open("/dev/tty1", O_WRONLY);
	if (tty_fd < 0) {
                perror("open /dev/tty1");
		return;
        }
        if (ioctl(tty_fd, KDSETMODE, KD_GRAPHICS)) {
		perror("ioctl KDSETMODE");
	}
        close(tty_fd);

        disp_fd = open("/dev/fb0", O_RDWR);
        if (disp_fd < 0) {
                perror("open /dev/fb0");
		return;
        }

	if (ioctl(disp_fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
                perror("ioctl FSCREENINFO");
                return;
        }

        if (ioctl(disp_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
                perror("ioctl VSCREENINFO");
                return;
        }

        bits = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, disp_fd, 0);

	load_fonts();
}
	
static void load_fonts() {
	int font_fd;
	char font_header[4];
	int num_chars;
	int format;
	struct psf2_header header2;

	font_fd = open(FONT_FILENAME, O_RDONLY);
	if (font_fd < 0) {
		perror("open font file");
		return;
	}

	read(font_fd, font_header, 4);
	if ((font_header[0] == 0x36) && (font_header[1] == 0x04)) {
		format = 1;
	} else if ((font_header[0] == 0x72) && (font_header[1] == 0xb5)) {
		format = 2;
	} else {
		perror("font: bad magic");
                return;
        }

	if (format == 1) {
		font_height = font_header[3];
		font_width = 8;

		if (font_header[2] & 0x01) {
			num_chars = 512;
		} else {
			num_chars = 256;
		}
		charsize = 1;
	} else {
		lseek(font_fd, 0l, SEEK_SET);
		read(font_fd, &header2, sizeof(header2));
		font_height = header2.height;
		font_width = header2.width;
		num_chars = header2.length;
		charsize = header2.charsize / font_height;
	}

	printf("Font height: %d\n", font_height);
	printf("Font width: %d\n", font_width);
	printf("Bytes per char: %d\n", charsize * font_height);

	font_rows = malloc(num_chars * font_height * charsize);
	read(font_fd, font_rows, num_chars * font_height);

	close(font_fd);
}

void disp_close() {
	close(disp_fd);
}

void disp_point(int x, int y, int r, int g, int b) {
	if (x >= vinfo.xres) {
		x = vinfo.xres - 1;
	}
	if (y >= vinfo.yres) {
		y = vinfo.yres - 1;
	}
	if (x < 0) {
		x = 0;
	}
	if (y < 0) {
		y = 0;
	}
        int offset = y * vinfo.xres + x;
        uint16_t val = ((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f);
        bits[offset] = val;
}

void disp_flush() {
        ioctl(disp_fd, FBIOGET_VSCREENINFO, &vinfo);
	vinfo.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
	ioctl(disp_fd, FBIOPUT_VSCREENINFO, &vinfo);
}

void disp_clear() {
	memset(bits, 0, finfo.smem_len);
}

void disp_line(int x0, int y0, int x1, int y1, int r, int g, int b) {
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2, e2;

	for(;;){
		disp_point(x0, y0, r, g, b);
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}
}

void disp_char(int x, int y, char c, int r, int g, int b) {
	int row_num;
	int byte_num;
	int bit_num;
	unsigned char row;

	for (row_num=0; row_num<font_height; row_num++) {
		for (byte_num=0; byte_num < charsize; byte_num++) {
			row = font_rows[c * font_height * charsize + row_num * charsize + byte_num];
			for (bit_num=0; bit_num<8; bit_num++) {
				if (row & 0x80) {
					disp_point(x + byte_num*8 + bit_num, y+row_num, r, g, b);
				}
				row = row << 1;
			}
		}
	}
}


void disp_string(int x, int y, char *s, int r, int g, int b) {
	int i;

	for (i=0; i<strlen(s); i++) {
		disp_char(x + (i * (font_width + 2)), y, s[i], r, g, b);
	}

}


int disp_font_height() {
	return font_height;
}

int disp_font_width() {
	return font_width;
}

void disp_clear_range(int x0, int y0, int x1, int y1) {
	int x;
	int y;

	for (x=x0; x<=x1; x++) {
		for (y=y0; y<=y1; y++) {
			disp_point(x, y, 0, 0, 0);
		}
	}
}
