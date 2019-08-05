#include <stdio.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <fcntl.h>
#include <stdint.h>

void point(int x, int y, int r, int g, int b);
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
uint16_t *bits;

int main() {
	int fd;
	int num_pixels;
	int i;

	fd = open("/dev/tty1", O_WRONLY);
	if (fd < 0) {
		perror("open /dev/tty1");
		return 1;
	}
	ioctl(fd, KDSETMODE, KD_GRAPHICS);
	close(fd);

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
		perror("ioctl FSCREENINFO");
		return 2;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
		perror("ioctl VSCREENINFO");
		return 3;
	}

	bits = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	num_pixels = vinfo.xres * vinfo.yres;
	for (i=0; i<num_pixels; i++) {
		bits[i] = 0;
	}

	for (i=0; i<100; i++) {
		point(240, i, 0x1f, 0x3f, 0x1f);
	}
	for (i=0; i<240; i++) {
		point(i, 100, 0x1f, 0x00, 0x00);
	}

}

void point(int x, int y, int r, int g, int b) {
	int offset = y * vinfo.xres + x;
	uint16_t val = ((r & 0x1f) << 11) | ((g & 0x3f) << 5) | (b & 0x1f);
	bits[offset] = val;
}



