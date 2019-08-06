#ifndef EOS_H
#define EOS_H

#include <stdint.h>
#include "tinyosc.h"

#define CAT_NONE 0
#define CAT_INTENSITY 1
#define CAT_FOCUS 2
#define CAT_COLOR 3
#define CAT_IMAGE 4
#define CAT_FORM 5
#define CAT_SHUTTER 6

extern char* category_name[];

void handle_encoders(uint8_t from, uint8_t to);
char *name_to_param(const char *name);
void subscribe(char *param);
void process_packet(char *buffer, int len);
void process_message(tosc_message *msg);
void eos_startup();

#endif
