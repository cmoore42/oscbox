#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "tinyosc.h"
#include "i2c.h"
#include "display.h"
#include "encoders.h"
#include "slip.h"
#include "touch.h"
#include "eos.h"
#include "globals.h"

void startup(void);
static void add_button(int x0, int y0, int x1, int y1, touch_callback cb, void *arg, char *text);

int category;

int main(int argc, char *argv[]) {
	char c;
	char message[1024];
	int message_len;
	pthread_t recv_thread;
	pthread_t gpio_thread;
	pthread_t touch_thread;
	int i;

	if ((argc > 1) && (argv[1][1] == 'd')) {
		debug = 1;
	}

	if ((argc > 1) && (argv[1][1] == 'v')) {
		verbose = 1;
		debug = 1;
	}

	for (i=0; i<=MAX_WHEELS; i++) {
		wheels[i].category = CAT_NONE;
	}

	category = CAT_NONE;

	open_port();
	if (slip_fd == -1) {
		return 1;
	}

	if (debug) {
		printf("Serial port open\n");
	}

	encoder_init();

	disp_open();
	disp_clear();

	pthread_create(&recv_thread, NULL, recv_func, NULL);
	pthread_create(&gpio_thread, NULL, encoder_func, NULL);
	pthread_create(&touch_thread, NULL, touch_func, NULL);

	startup();

	pthread_join(recv_thread, NULL);
	pthread_join(gpio_thread, NULL);
	pthread_join(touch_thread, NULL);
}


void test_cb(void *arg) {
	int iarg = (int)arg;
	switch(iarg) {
		case 1:
			category = CAT_FOCUS;
			break;
		case 2:
			category = CAT_COLOR;
			break;
		case 3:
			category = CAT_IMAGE;
			break;
		case 4:
			category = CAT_FORM;
			break;
		case 5:
			category = CAT_SHUTTER;
			break;
	}

	printf("Category %s\n", category_name[category]);
}

/**
 * Setup code to be run after the threads are started.
 */
void startup() {
	slip_init();

	eos_startup();

	/* BEGIN TEST CODE */
	disp_line(0, 119, 479, 119, COLOR_WHITE);
	disp_line(119, 0, 119, 119, COLOR_WHITE);
	disp_line(239, 0, 239, 119, COLOR_WHITE);
	disp_line(359, 0, 359, 119, COLOR_WHITE);


	add_button(8, 180, 88, 260, test_cb, (void *)1, "Focus");
	add_button(104, 180, 184, 260, test_cb, (void *)2, "Color");
	add_button(200, 180, 280, 260, test_cb, (void *)3, "Image");
	add_button(296, 180, 376, 260, test_cb, (void *)4, "Form");
	add_button(391, 180, 471, 260, test_cb, (void *)5, "Shutter");

	/* END TEST CODE */
}

static void add_button(int x0, int y0, int x1, int y1, touch_callback cb, void *arg, char *text) {
	int center_x = x0 + (x1 - x0) / 2;
	int center_y = y0 + (y1 - y0) / 2;

	disp_line(x0, y0, x1, y0, COLOR_WHITE);
	disp_line(x1, y0, x1, y1, COLOR_WHITE);
	disp_line(x1, y1, x0, y1, COLOR_WHITE);
	disp_line(x0, y1, x0, y0, COLOR_WHITE);
	add_region(x0, y0, x1, y1, cb, arg);

	int text_start_x = center_x - (strlen(text) / 2 * 10);
	int text_start_y = center_y - 8;
	disp_string(text_start_x, text_start_y, text, COLOR_WHITE);

}





