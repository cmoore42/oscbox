#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include "globals.h"
#include "touch.h"

static char* types[] = {
	"SYN", "KEY", "REL", "ABS", "MSC", "SW"};

static int mapx(int x);
static int mapy(int y);
static void process_touch(int x, int y);

struct touch_region *regions;

/**
 * This function processes touches on the touch screen
 */
void* touch_func(void *ptr) {
	struct input_event event;
	int fd = open("/dev/input/event0", O_RDONLY);
	int keydown;
	int last_x;
	int last_y;

	keydown = 0;
	regions = NULL;

	while (1) {
		read(fd, &event, sizeof(event));
		switch (event.type) {
			case EV_SYN:
				break;
			case EV_KEY:
				if (event.code != 330) {
					if (debug) {
						printf("Unexpected KEY code %d (0x%x)\n", event.code, event.code);
					}
				} else {
					keydown = event.value;
					if (event.value == 0) {
						if (debug) {
							printf("Touch at %d, %d\n", last_x, last_y);
							printf("Pixel %d, %d\n", mapx(last_x), mapy(last_y));
						}
						process_touch(mapx(last_x), mapy(last_y));
					}
				}
				break;
			case EV_ABS:
				switch (event.code) {
					case ABS_X:
						last_x = event.value;
						break;
					case ABS_Y:
						last_y = event.value;
						break;
					case ABS_PRESSURE:
						break;
					default:
						if (debug) {
							printf("Unexpected ABS code %d (0x%x)\n", event.code, event.code);
						}
						break;
				}
				break;
			default:
				if (debug) {
					printf("Got unexpected event type %d\n", event.type);
				}
				break;
		}

	}
}

void add_region(int x0, int y0, int x1, int y1, touch_callback cb, void *arg) {
	struct touch_region *new_region;
	struct touch_region *next_region;

	new_region = malloc(sizeof(*new_region));

	new_region->x0 = x0;
	new_region->y0 = y0;
	new_region->x1 = x1;
	new_region->y1 = y1;
	new_region->cb = cb;
	new_region->arg = arg;
	new_region->next = NULL;

	if (regions == NULL) {
		regions = new_region;
		return;
	}

	next_region = regions;
	while (1) {
		if (next_region->next == NULL) {
			next_region->next = new_region;
			return;
		}
		next_region = next_region->next;
	}
}

static int mapx(int x) {
	double slope = -0.12858;
	double offset = 505.7806;

	double output = (double)x * slope + offset;
	return (int)output;
}

static int mapy(int y) {
	double slope = 0.086685;
	double offset = -18.958;

	double output = (double)y * slope + offset;
	return (int)output;
}

static void process_touch(int x, int y) {
	struct touch_region *next_region;

	next_region = regions;
	while (next_region != NULL) {
		if ((x >= next_region->x0) &&
		    (y >= next_region->y0) &&
		    (x <= next_region->x1) &&
		    (y <= next_region->y1)) {
			next_region->cb(next_region->arg);
			return;
		}

		next_region = next_region->next;
	}
}
