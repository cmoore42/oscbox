#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>

static char* types[] = {
	"SYN", "KEY", "REL", "ABS", "MSC", "SW"};

int mapx(int x);
int mapy(int y);


int main() {
	struct input_event event;
	int fd = open("/dev/input/event0", O_RDONLY);
	int keydown;
	int last_x;
	int last_y;

	keydown = 0;

	while (1) {
		read(fd, &event, sizeof(event));
		switch (event.type) {
			case EV_SYN:
				break;
			case EV_KEY:
				if (event.code != 330) {
					printf("Unexpected KEY code %d (0x%x)\n", event.code, event.code);
				} else {
					keydown = event.value;
					if (event.value == 0) {
						printf("Touch at %d, %d\n", last_x, last_y);
						printf("Pixel %d, %d\n", mapx(last_x), mapy(last_y));
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
						printf("Unexpected ABS code %d (0x%x)\n", event.code, event.code);
						break;
				}
				break;
			default:
				printf("Got unexpected event type %d\n", event.type);
				break;
		}

	}
}

int mapx(int x) {
	double slope = -0.12858;
	double offset = 505.7806;

	double output = (double)x * slope + offset;
	return (int)output;
}

int mapy(int y) {
	double slope = 0.086685;
	double offset = -18.958;

	double output = (double)y * slope + offset;
	return (int)output;
}
