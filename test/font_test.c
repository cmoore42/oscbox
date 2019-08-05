#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int height;
char *rows;
void dump_char(int ch);

int main() {
	char header[4];
	int fd;
	int i;
	int mode;
	int num_chars;
	int ch;

	fd = open("Tamzen8x15.psf", O_RDONLY);

	read(fd, header, 4);

	for (i=0; i<4; i++) {
		printf("header[%d] is %d (0x%02x)\n", i, header[i], header[i]);
	}

	if ((header[0] != 0x36) || (header[1] != 0x04)) {
		printf("bad magic\n");
		return 1;
	}

	mode = header[2];
	height = header[3];

	if (mode & 0x01)  {
		num_chars = 512;
	} else {
		num_chars = 256;
	}

	rows = malloc(num_chars * height);

	read(fd, rows, num_chars * height);

	printf("Read %d bytes of font data\n", num_chars * height);

	for (ch=30; ch<64; ch++) {
		dump_char(ch);
	}
}

void dump_char(int ch) {
	int i;
	int j;
	unsigned char row;

	for (i=0; i<height; i++) {
		row = rows[ch * height + i];
		for (j=0; j<8; j++) {
			if (row & 0x80) {
				printf("X");
			} else {
				printf(" ");
			}
			row = row << 1;
		}
		printf("\n");
	}
	printf("\n");
}


