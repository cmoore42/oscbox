#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "slip.h"
#include "eos.h"

int state;
int slip_fd;
char recv_buffer[46*1024];
int recv_buffer_len;

extern int debug;
extern int verbose;

void* recv_func(void *ptr) {
	int n;
	unsigned char c;

	while (1) {
		/* Read the incoming bytes */
		n = read(slip_fd, &c, 1);
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

void slip_init() {
	state = STATE_IDLE;
	recv_buffer_len = 0;
	recv_buffer[recv_buffer_len] = '\0';
}

void slip_send(char *text, int len) {
	char end = END;

	if (verbose) {
		printf("Sending '%s'\n", text);
	}
	write(slip_fd, &end, 1);
	write(slip_fd, text, len);
	write(slip_fd, &end, 1);
}

void open_port() {
	struct termios options;

	slip_fd = open("/dev/ttyGS0", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (slip_fd == -1) {
		perror("open");
	} else {
		fcntl(slip_fd, F_SETFL, 0);
		tcgetattr(slip_fd, &options);
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);
		options.c_cflag |= (CLOCAL | CREAD);
		options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
		tcsetattr(slip_fd, TCSANOW, &options);
	}
}
