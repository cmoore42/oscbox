#include "../display.h"
#include <stdio.h>

int main() {
	int i;
	char c;

	disp_open();

	disp_clear();
	printf("Screen cleared, press enter\n");
	fgetc(stdin);
	
	disp_line(240, 160, 0, 0, 0, 0, 0xff);
	disp_line(240, 160, 240, 0, 0, 0xff, 0);
	disp_line(240, 160, 479, 0, 0, 0xff, 0xff);
	disp_line(240, 160, 479, 160, 0xff, 0, 0);
	disp_line(240, 160, 479, 319, 0xff, 0, 0xff);
	disp_line(240, 160, 240, 319, 0xff, 0xff, 0);
	disp_line(240, 160, 0, 319, 0xff, 0xff, 0xff);
	disp_line(240, 160, 0, 160, 0, 0, 0xff);

	disp_flush();
	printf("Line test done, press enter\n");
	fgetc(stdin);

	disp_clear();
	disp_line(0, 0, 480, 320, 0xff, 0xff, 0xff);

	disp_flush();
	printf("Overflow test done, press enter\n");
	fgetc(stdin);

	disp_clear();
	disp_line(-100, -100, 480, 320, 0xff, 0xff, 0xff);

	disp_flush();
	printf("Underflow test done, press enter\n");
	fgetc(stdin);

	disp_clear();
	disp_flush();

	int row = 0;
	int col = 0;
	int ch = 0x21;
	while (ch < 0x7f) {
		disp_char(30 + (col * (disp_font_width() + 3)), (row * 30), ch, 0xff, 0xff, 0xff);
		++ch;
		++col;
		if (col > 15) {
			col = 0;
			++row;
		}
	}
	disp_flush();
	printf("Char test done, press enter\n");
	fgetc(stdin);

	disp_clear();
	disp_flush();
	disp_string(30, 30, "Hello world", 0xff, 0xff, 0xff);
	disp_flush();
	printf("String test done, press enter\n");
	fgetc(stdin);

	disp_close();

}




