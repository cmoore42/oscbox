#ifndef ENCODERS_H
#define ENCODERS_H

#define I2C_BUS 0
#define I2C_ADDR 0x20

extern int i2c_fd;

void* encoder_func(void *ptr);
void encoder_init();

#endif
