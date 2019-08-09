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

/* Veritcal Layout */
#define ENCODER_WINDOW_SIZE_Y 80
#define CAT_BUTTON_SIZE_Y 60
#define SOFTKEY_SIZE_Y 60
#define SPACE_Y 14

/* Horizontal Layout */
#define SOFTKEY_SIZE_X 140
#define SOFTKEY_SPACE_X 15

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


void category_cb(void *arg) {
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

void softkey_cb(void *arg) {
	int iarg = (int)arg;
	handle_softkey(iarg);
}

/**
 * Setup code to be run after the threads are started.
 */
void startup() {
	slip_init();

	eos_startup();

	/* Frames around encoder text */
	disp_line(0, ENCODER_WINDOW_SIZE_Y, 479, ENCODER_WINDOW_SIZE_Y, COLOR_WHITE);
	disp_line(119, 0, 119, ENCODER_WINDOW_SIZE_Y, COLOR_WHITE);
	disp_line(239, 0, 239, ENCODER_WINDOW_SIZE_Y, COLOR_WHITE);
	disp_line(359, 0, 359, ENCODER_WINDOW_SIZE_Y, COLOR_WHITE);


	/* Category butotns */
	int cat_button_top = ENCODER_WINDOW_SIZE_Y + SPACE_Y;
	int cat_button_bottom = cat_button_top + CAT_BUTTON_SIZE_Y;
	add_button(8, cat_button_top, 
			88, cat_button_bottom,
			category_cb, (void *)1, "Focus");
	add_button(104, cat_button_top, 
			184, cat_button_bottom,
			category_cb, (void *)2, "Color");
	add_button(200, cat_button_top, 
			280, cat_button_bottom,
			category_cb, (void *)3, "Image");
	add_button(296, cat_button_top, 
			376, cat_button_bottom,
			category_cb, (void *)4, "Form");
	add_button(391, cat_button_top, 
			471, cat_button_bottom,
			category_cb, (void *)5, "Shutter");

	/* Softkey buttons */
	int row;
	int col;

	for (row=0; row<2; row++) {
		for (col=0; col<3; col++) {
			int sk_num = row*3 + col + 1;
			int x0 = col*SOFTKEY_SIZE_X + (col+1)*SOFTKEY_SPACE_X;
			int x1 = x0 + SOFTKEY_SIZE_X;
			int y0 = ENCODER_WINDOW_SIZE_Y + CAT_BUTTON_SIZE_Y + row*SOFTKEY_SIZE_Y + (row+2) * SPACE_Y;
			int y1 = y0 + SOFTKEY_SIZE_Y;
			char sk_name[10];
			sprintf(sk_name, "SK %d", sk_num);
			add_button(x0, y0, x1, y1, softkey_cb, (void *)sk_num, sk_name);
		}
	}
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
	int encoder_start_y = 8;

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
		encoder_start_y += 16;
	}
	disp_string(encoder_start_x, encoder_start_y, value, COLOR_WHITE);
}

void clear_encoder_text(int encoder_num) {
	if (debug) {
		printf("Clear encoder name %d\n", encoder_num);
	}
	switch(encoder_num) {
		case 0:
			disp_clear_range(0, 0, 118, ENCODER_WINDOW_SIZE_Y-1);
			break;
		case 1:
			disp_clear_range(120, 0, 238, ENCODER_WINDOW_SIZE_Y-1);
			break;
		case 2:
			disp_clear_range(240, 0, 358, ENCODER_WINDOW_SIZE_Y-1);
			break;
		case 3:
			disp_clear_range(360, 0, 479, ENCODER_WINDOW_SIZE_Y-1);
			break;
	}
}

void update_wheel(int wheel_num) {
	set_category(category);
}

/**
 * Set the text for a softkey button
 *
 * @param sk_num Softkey numbers, 1..6
 */

void set_sk_text(int sk_num, char *text) {
	int row;
	int col;
	int x0, y0;
	int x1, y1;

	if (sk_num > 6) {
		return;
	}

	row = (sk_num - 1) / 3;
	col = (sk_num - 1) % 3;

	x0 = col*SOFTKEY_SIZE_X + (col+1)*SOFTKEY_SPACE_X;
	x1 = x0 + SOFTKEY_SIZE_X;
	y0 = ENCODER_WINDOW_SIZE_Y + CAT_BUTTON_SIZE_Y + row*SOFTKEY_SIZE_Y + (row+2) * SPACE_Y;
	y1 = y0 + SOFTKEY_SIZE_Y;

	disp_clear_range(x0+1, y0+1, x1-1, y1-1);

	int center_x = x0 + (x1 - x0) / 2;
	int center_y = y0 + (y1 - y0) / 2;

	int text_start_x = center_x - (strlen(text) / 2 * 10);
	int text_start_y = center_y - 8;
	disp_string(text_start_x, text_start_y, text, COLOR_WHITE);
}
