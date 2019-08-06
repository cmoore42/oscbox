#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <gpiod.h>
#include "tinyosc.h"
#include "i2c.h"
#include "display.h"
#include "encoders.h"
#include "slip.h"
#include "touch.h"
#include "eos.h"
#include "globals.h"



int running = 1;

void startup(void);



int main(int argc, char *argv[]) {
	char c;
	struct termios options;
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

void startup() {


	slip_init();

	eos_startup();


}






