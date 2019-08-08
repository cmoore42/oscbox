#include "globals.h"

int debug;
int verbose;

struct wheel wheels[MAX_WHEELS+1];
int encoder_map[NUM_ENCODERS];  // Map of physical encoders to EOS wheels

