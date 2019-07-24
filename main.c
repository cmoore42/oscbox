#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <stdlib.h>
#include <ctype.h>
#include <gpiod.h>
#include "tinyosc.h"


#define STATE_IDLE 0
#define STATE_INFRAME 1
#define STATE_ESC 2

#define END 0xC0
#define ESC 0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

int fd;
int verbose = 0;
int state;
char recv_buffer[46*1024];
int recv_buffer_len;

int running = 1;

void process_packet(char *, int len);
void open_port(void);
void subscribe(char *);
void slip_send(char *, int);
void startup(void);
void handle_receive(void);
void handle_user_input(void);
void *recv_func(void *);
void *gpio_func(void *);
char *name_to_param(const char *);

struct wheel {
	int index;
	char name[40];
	char param[40];
	float min;
	float max;
	float current;
} wheels[13];;

int main(int argc, char *argv[]) {
	char c;
	struct termios options;
	char message[1024];
	int message_len;
	pthread_t recv_thread;
	pthread_t gpio_thread;

	open_port();
	if (fd == -1) {
		return 1;
	}

	if ((argc > 1) && (argv[1][1] == 'v')) {
		verbose = 1;
	}

	pthread_create(&recv_thread, NULL, recv_func, NULL);
	pthread_create(&gpio_thread, NULL, gpio_func, NULL);

	startup();

	pthread_join(recv_thread, NULL);
	pthread_join(gpio_thread, NULL);
}

void startup() {
	char send_buffer[1024];
	int send_buffer_len;

	slip_send("OK", 2);

	state = STATE_IDLE;
	recv_buffer_len = 0;
	recv_buffer[recv_buffer_len] = '\0';

	send_buffer_len = tosc_writeMessage(send_buffer, sizeof(send_buffer), "/eos/get/version", "");
	slip_send(send_buffer, send_buffer_len);

	/*
	send_buffer_len = tosc_writeMessage(send_buffer, sizeof(send_buffer), 
			"/eos/filter/add", "s",
			"/eos/out/param/*");
	slip_send(send_buffer, send_buffer_len);
	*/

	subscribe("pan");
	subscribe("tilt");
	subscribe("scroller");
	subscribe("color_select");
	subscribe("color_select_2");
	subscribe("gobo_ind\\spd");
	subscribe("gobo_select");
	subscribe("gobo_select_2");
	subscribe("beam_fx_select");
	subscribe("iris");
	subscribe("edge");
	subscribe("shutter_strobe");
	subscribe("zoom");
	subscribe("red");
	subscribe("green");
	subscribe("blue");
	subscribe("white");
	subscribe("intensity");
	subscribe("hue");
	subscribe("saturation");
}

void* recv_func(void *ptr) {
	int n;
	unsigned char c;

	while (1) {
		/* Read the incoming bytes */
		n = read(fd, &c, 1);
		if (verbose) {
			printf("%d: '%c' (0x%02x)\n", n, c, c);
		}
		switch (state) {
			case STATE_IDLE:
				if (c == END) {
					if (verbose) {
						printf("Got packet start\n");
					}
					state = STATE_INFRAME;
					recv_buffer_len = 0;
				}
				break;
			case STATE_INFRAME:
				if (c == END) {
					if (verbose) {
						printf("Got packet end\n");
					}
					if (recv_buffer_len == 0) {
						/* We got two ENDs in a row, which means 
						 * we're out of sync and
						 * this END is really a START.
						 */
						if (verbose) {
							printf("Got two ENDs, resyncing\n");
						}
						state = STATE_INFRAME;
						recv_buffer_len = 0;
					} else {
						state = STATE_IDLE;
						process_packet(recv_buffer, recv_buffer_len);
					}
				} else if (c == ESC) {
					if (verbose) {
						printf("Got escape\n");
					}
					state = STATE_ESC;
				} else {
					recv_buffer[recv_buffer_len] = c;
					++recv_buffer_len;
				}
				break;
			case STATE_ESC:
				if (c == ESC_END) {
					if (verbose) {
						printf("Got escaped END\n");
					}
					recv_buffer[recv_buffer_len] = END;
					++recv_buffer_len;
					state = STATE_INFRAME;
				} else if (c == ESC_ESC) {
					if (verbose) {
						printf("Got escaped ESC\n");
					}
					recv_buffer[recv_buffer_len] = ESC;
					++recv_buffer_len;
					state = STATE_INFRAME;
				}
				break;
		}
	}
}

void* gpio_func(void *ptr) {
	char line[80];
	char outbuf[1024];
	int outbuf_len;
	struct gpiod_chip *chip;
	struct gpiod_line *gpio_line;
	struct gpiod_line_event event;
	struct timespec ts = {0, 1000000 };
	int rv;

	chip = gpiod_chip_open("/dev/gpiochip0");
        if (!chip) {
                perror("gpiod_chip_open");
                return NULL;
        }

        gpio_line = gpiod_chip_get_line(chip, 7);
        if (!gpio_line) {
                perror("gpiod_chip_get_line");
                gpiod_chip_close(chip);
                return NULL;
        }

        rv = gpiod_line_request_both_edges_events(gpio_line, "GPIO7");
        if (rv) {
                perror("gpiod_line_request_rising_edge_events");
                gpiod_chip_close(chip);
                return NULL;
        }



	/* If there is user input (button, encoder, etc), handle it */
	while (1) {
#if USE_STDIN
		fgets(line, sizeof(line), stdin);
		line[strlen(line)-1] = '\0';
		if (strncmp(line, "live", 4) == 0) {
			outbuf_len = tosc_writeMessage(outbuf, sizeof(outbuf),
					"/eos/key/live", "");
			slip_send(outbuf, outbuf_len);
		} else if (strncmp(line, "sk", 2) == 0) {
			char cmd[80];
			strcpy(cmd, "/eos/key/softkey_");
			strcat(cmd, &(line[2]));
			outbuf_len = tosc_writeMessage(outbuf, sizeof(outbuf),
					cmd, "");
			slip_send(outbuf, outbuf_len);
		} else if (strncmp(line, "pan", 3) == 0) {
			char cmd[80];
			float dir;
			strcpy(cmd, "/eos/wheel/pan");
			if (line[3] == '+') {
				dir = 1.0;
			} else {
				dir = -1.0;
			}
			outbuf_len = tosc_writeMessage(outbuf, sizeof(outbuf),
					cmd, "f", dir);
			slip_send(outbuf, outbuf_len);
		} else if (strncmp(line, "w", 1) == 0) {
			char cmd[80];
			int index;
			float dir;

			/* /eos/wheel/<param>=1.0 or -1.0 */
			index = atoi(&(line[2]));
			strcpy(cmd, "/eos/wheel/");
			strcat(cmd, wheels[index].param);
			if (line[1] == '+') {
				dir = 1.0;
			} else {
				dir = -1.0;
			}
			outbuf_len = tosc_writeMessage(outbuf, sizeof(outbuf),
					cmd, "f", dir);
			slip_send(outbuf, outbuf_len);
		}
#else
		do {
			rv = gpiod_line_event_wait(gpio_line, &ts);
		} while (rv <= 0);

		rv = gpiod_line_event_read(gpio_line, &event);
		if (!rv) {
			char cmd[80];

			printf("event: %d timestamp: [%8ld.%09ld]\n",
					event.event_type, event.ts.tv_sec, event.ts.tv_nsec);
			strcpy(cmd, "/eos/wheel/coarse/");
			strcat(cmd, wheels[1].param);
			outbuf_len = tosc_writeMessage(outbuf, sizeof(outbuf),
					cmd, "f", 1.0);
			slip_send(outbuf, outbuf_len);
		}
#endif
	}
}

void process_message(tosc_message *msg) {
	char *address;
	int index;

	tosc_printMessage(msg);
	tosc_reset(msg);
	address = tosc_getAddress(msg);
	if (strncmp(address, "/eos/out/active/wheel", 21) == 0) {
		const char *name;
		int value;

		index = atoi(address+22);
		name = tosc_getNextString(msg);
		value = tosc_getNextInt32(msg);
		strcpy(wheels[index].name, name);
		strcpy(wheels[index].param, name_to_param(name));
		printf("==> wheel %d, name \"%s\", param \"%s\", value %d\n",
				index, name, name_to_param(name), value);
	} else if (strncmp(address, "/eos/out/softkey", 16) == 0) {
		index = atoi(address+17);
		printf("==> softkey %d is named \"%s\"\n", index, tosc_getNextString(msg));
	}

}

void process_packet(char *buffer, int len) {


	/* Special case - tell the console we're here */
	if (strncmp(buffer, "ETCOSC?", 7) == 0) {
		startup();
		return;
	}

	if (tosc_isBundle(buffer)) {
          tosc_bundle bundle;
          tosc_parseBundle(&bundle, buffer, len);
          const uint64_t timetag = tosc_getTimetag(&bundle);
          tosc_message osc;
          while (tosc_getNextMessage(&bundle, &osc)) {
            process_message(&osc);
          }
        } else {
          tosc_message osc;
          tosc_parseMessage(&osc, buffer, len);
          process_message(&osc);
        }
}

void open_port() {
	struct termios options;

	fd = open("/dev/ttyGS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	// fd = open("/dev/ttyGS0", O_RDWR | O_NOCTTY );
	if (fd == -1) {
		perror("open");
	} else {
		fcntl(fd, F_SETFL, 0);
		tcgetattr(fd, &options);
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);
		options.c_cflag |= (CLOCAL | CREAD);
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		tcsetattr(fd, TCSANOW, &options);
	}
}

void slip_send(char *text, int len) {
	char end = END;

	printf("Sending '%s'\n", text);
	write(fd, &end, 1);
	write(fd, text, len);
	write(fd, &end, 1);
}

void subscribe(char *param) {
	char cmd[1024];
	char buffer[1024];
	int buffer_len;

	strcpy(cmd, "/eos/subscribe/param/");
	strcat(cmd, param);

	buffer_len = tosc_writeMessage(buffer, sizeof(buffer), cmd, "i", 1);
	slip_send(buffer, buffer_len);
}

char *name_to_param(const char *name) {
	static char result[40];
	const char *src;
	char *dst;
	int leading = 1;

	dst = result;
	src = name;

	while (*src != '[') {
		if (*src == ' ') {
			if (leading) {
				/* ignore it */
			} else {
				*dst = '_';
			}
		} else if (*src == '/') {
			*dst = '\\';
			leading = 0;
		} else {
			*dst = tolower(*src);
			leading = 0;
		}
		++src;
		++dst;
	}

	*dst = '\0';
	--dst;
	while (*dst == '_') {
		*dst = '\0';
		--dst;
	}

	return result;
}


