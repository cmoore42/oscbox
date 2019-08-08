#ifndef GLOBALS_H
#define GLOBALS_H

extern int debug;
extern int verbose;

#define MAX_WHEELS 40
#define NUM_ENCODERS 4

struct wheel {
	int index;
	char name[40];
	char param[40];
	int category;
	float min;
	float max;
	float current;
};

extern struct wheel wheels[MAX_WHEELS+1];
extern int encoder_map[];

#endif
