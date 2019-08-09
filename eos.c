#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "globals.h"
#include "slip.h"
#include "eos.h"

extern void update_wheel(int wheel_num);
extern void set_sk_text(int sk_num, char *text);

char* category_name[] = {
	"None", "Intensity", "Focus", "Color",
	"Image", "Form", "Shutter"};

void handle_encoders(uint8_t from, uint8_t to) {
	int bit;
	char cmd[80];
	char outbuf[1024];
	int outbuf_len;

	if (verbose) {
		printf("Encoder change from 0x%02x to 0x%02x\n",
				from, to);
	}

	for (bit=0; bit<8; bit += 2) {
		int mask = (1 << bit);
		if ((from & mask) != (to & mask)) {
			int a = (to & mask);
			int b = (to & (mask << 1)) >> 1;
			int encoder = 3 - (bit / 2);
			int wheel = encoder_map[encoder];
			if (debug) {
				printf("Encoder %d, wheel %d moved ", encoder, wheel);
				if (a == b) {
					printf("forward\n");
				} else {
					printf("reverse\n");
				}
			}

			if (wheel >= 0) {
				strcpy(cmd, "/eos/wheel/coarse/");
				strcat(cmd, wheels[wheel].param);
				float dir;
				if (a == b) {
					dir = 1.0;
				} else {
					dir = -1.0;
				}
				outbuf_len = tosc_writeMessage(outbuf, sizeof(outbuf),
						cmd, "f", dir);
				slip_send(outbuf, outbuf_len);
			}
		}
	}
}

void handle_softkey(int sk_num) {
	char cmd[80];
	char outbuf[1024];
	int outbuf_len;
	
	sprintf(cmd, "/eos/softkey/%d", sk_num);
	outbuf_len = tosc_writeMessage(outbuf, sizeof(outbuf), cmd, "");
	slip_send(outbuf, outbuf_len);
}

void process_message(tosc_message *msg) {
	char *address;
	int index;

	if (verbose) {
		tosc_printMessage(msg);
	}
	tosc_reset(msg);
	address = tosc_getAddress(msg);
	if (strncmp(address, "/eos/out/active/wheel", 21) == 0) {
		const char *name;
		int category;

		index = atoi(address+22);
		name = tosc_getNextString(msg);
		category = tosc_getNextInt32(msg);
		strcpy(wheels[index].name, name);
		strcpy(wheels[index].param, name_to_param(name));
		wheels[index].category = category;
		if (debug) {
			printf("==> wheel %d, name \"%s\", param \"%s\", category %s\n",
				index, name, name_to_param(name), category_name[category]);
		}
		update_wheel(index);
	} else if (strncmp(address, "/eos/out/softkey", 16) == 0) {
		char name[80];
		index = atoi(address+17);
		strcpy(name, tosc_getNextString(msg));
		if (debug) {
			printf("==> softkey %d is named \"%s\"\n", index, name);
		}
		set_sk_text(index, name);
	}

}

void process_packet(char *buffer, int len) {

	/* Special case - tell the console we're here */
	if (strncmp(buffer, "ETCOSC?", 7) == 0) {
		eos_startup();
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
				++dst;
			}
		} else if (*src == '/') {
			*dst = '\\';
			++dst;
			leading = 0;
		} else {
			*dst = tolower(*src);
			++dst;
			leading = 0;
		}
		++src;
	}

	*dst = '\0';
	if (strlen(result) != 0) {
		--dst;
		while (*dst == '_') {
			*dst = '\0';
			--dst;
		}
	}

	return result;
}

void eos_startup() {
	char send_buffer[1024];
	int send_buffer_len;

	slip_send("OK", 2);

	send_buffer_len = tosc_writeMessage(send_buffer, sizeof(send_buffer),
			"/eos/get/version", "");
	slip_send(send_buffer, send_buffer_len);

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

