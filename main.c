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
void set_category(int category);
void set_encoder_text(int encoder_num, char *text);
void clear_encoder_text(int encoder_num);
void update_wheel(int wheel_num);

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

	category = CAT_COLOR;
	for (i=0; i<NUM_ENCODERS; i++) {
		encoder_map[i] = -1;
	}

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
			set_category(CAT_FOCUS);
			break;
		case 2:
			set_category(CAT_COLOR);
			break;
		case 3:
			set_category(CAT_IMAGE);
			break;
		case 4:
			set_category(CAT_FORM);
			break;
		case 5:
			set_category(CAT_SHUTTER);
			break;
	}

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

void set_category(int c) {
	int wheel_num;
	int encoder_num;

	category = c;

	if (debug) {
		printf("Category %s\n", category_name[category]);
	}

	encoder_num = 0;
	for (wheel_num=0; wheel_num <= MAX_WHEELS; wheel_num++) {
		if (wheels[wheel_num].category == category) {
			encoder_map[encoder_num] = wheel_num;
			set_encoder_text(encoder_num, wheels[wheel_num].name);
			if (debug) {
				printf("Encoder %d maps to wheel %d\n", encoder_num, wheel_num);
			}
			++encoder_num;
			if (encoder_num >= NUM_ENCODERS) {
				break;
			}
		}
	}

	while (encoder_num < NUM_ENCODERS) {
		if (debug) {
			printf("Encoder %d maps to -1\n", encoder_num);
		}
		clear_encoder_text(encoder_num);
		encoder_map[encoder_num++] = -1;
	}
}

void set_encoder_text(int encoder_num, char *text) {
	char trimmed_text[80];
	char value[80];
	char *tok;

	if (debug) {
		printf("Encoder %d is named %s\n", encoder_num, text);
	}
	clear_encoder_text(encoder_num);
	int encoder_start_x = 120*encoder_num + 5;
	int encoder_start_y = 20;

	strcpy(trimmed_text, text);
	if (tok = strchr(trimmed_text, '[')) {
		strcpy(value, tok);
		*tok = '\0';
	}

	tok = strtok(trimmed_text, " ");
	while (tok != NULL) {
		if (debug) {
			printf("Line: '%s'\n", tok);
		}
		disp_string(encoder_start_x, encoder_start_y, tok, COLOR_WHITE);
		tok = strtok(NULL, " ");
		encoder_start_y += 20;
	}
	disp_string(encoder_start_x, encoder_start_y, value, COLOR_WHITE);
}

void clear_encoder_text(int encoder_num) {
	if (debug) {
		printf("Clear encoder name %d\n", encoder_num);
	}
	switch(encoder_num) {
		case 0:
			disp_clear_range(0, 0, 118, 118);
			break;
		case 1:
			disp_clear_range(120, 0, 238, 118);
			break;
		case 2:
			disp_clear_range(240, 0, 358, 118);
			break;
		case 3:
			disp_clear_range(360, 0, 479, 118);
			break;
	}
}

void update_wheel(int wheel_num) {
	set_category(category);
}
