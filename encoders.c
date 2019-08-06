#include <stdio.h>
#include <gpiod.h>
#include "encoders.h"
#include "eos.h"
#include "i2c.h"

/**
 * This file covers input from the encoders and buttons.
 * This includes monitoring the GPIO pins and interacting
 * with the I2C I/O extender.
 */

int i2c_fd;
extern int debug;


/**
 * encoder thread
 * This function is executed in a separate thread.
 * It monitors the GPIO lines (encoders and buttons)
 * and responds to changes.
 *
 * GPIO6 is connected to the INT line of the I/O
 * expander, and the expander is programmed to interrupt
 * on any line change.
 *
 * This function monitors GPIO6 for a falling edge, indicating
 * an interrupt.  When GPIO6 goes low it sends I2C commands
 * to read the current state of the encoders and buttons
 * and acts accordingly.
 */

void* encoder_func(void *ptr) {
	char line[80];
	struct gpiod_chip *chip;
	struct gpiod_line *gpio_line;
	struct gpiod_line_event event;
	struct timespec ts = {0, 1000000 };
	int rv;
	uint8_t current_encoders;
	uint8_t previous_encoders;

	chip = gpiod_chip_open("/dev/gpiochip0");
        if (!chip) {
                perror("gpiod_chip_open");
                return NULL;
        }

        gpio_line = gpiod_chip_get_line(chip, 6);
        if (!gpio_line) {
                perror("gpiod_chip_get_line");
                gpiod_chip_close(chip);
                return NULL;
        }

        rv = gpiod_line_request_falling_edge_events(gpio_line, "GPIO6");
        if (rv) {
                perror("gpiod_line_request_falling_edge_events");
                gpiod_chip_close(chip);
                return NULL;
        }

	previous_encoders = i2c_read(i2c_fd, REG_GPIOA);

	/* If there is user input (button, encoder, etc), handle it */
	while (1) {

		do {
			rv = gpiod_line_event_wait(gpio_line, &ts);
		} while (rv <= 0);

		rv = gpiod_line_event_read(gpio_line, &event);
		if (!rv) {
			/* Got an interrupt */
			char cmd[80];

			if (debug) {
				printf("event: %d timestamp: [%8ld.%09ld]\n",
						event.event_type, event.ts.tv_sec, event.ts.tv_nsec);
			}

			/* Clear the interrupt */
			i2c_read(i2c_fd, REG_INTCAPA);
			i2c_read(i2c_fd, REG_INTCAPB);

			current_encoders = i2c_read(i2c_fd, REG_GPIOA);
			if (current_encoders != previous_encoders) {
				handle_encoders(previous_encoders, current_encoders);
				previous_encoders = current_encoders;
			}
		}
	}
}

void encoder_init() {
	int i;

	i2c_fd = i2c_open(I2C_BUS, I2C_ADDR);
	if (i2c_fd < 0) {
		perror("i2c_open");
		return;
	}
	i2c_init(i2c_fd);
	if (debug) {
		printf("I2C initialized\n");
		printf("I2C registers:\n");
		for (i = 0; i < 0x16; i++) {
			printf("0x%02x: 0x%02x\n", i, i2c_read(i2c_fd, i));
		}
	}
}
