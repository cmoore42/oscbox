#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include "i2c.h"

int i2c_open(int bus, int addr) {
	int fd;
	char fname[80];

	sprintf(fname, "/dev/i2c-%d", bus);
	fd = open(fname, O_RDWR);
	if (fd < 0) {
		perror("I2C open");
		return fd;
	}

	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		perror("I2C set address");
		close(fd);
		return -1;
	}

	return fd;
}


void i2c_init(int f) {
	/* Mirror interrupt lines */
	i2c_write(f, REG_IOCON, 0x40);

	/* All lines are inputs */
	i2c_write(f, REG_IODIRA, 0xff);
	i2c_write(f, REG_IODIRB, 0xff);

	/* All lines have pullups */
	i2c_write(f, REG_GPPUA, 0xff);
	i2c_write(f, REG_GPPUB, 0xff);

	/* All lines are normal polarity */
	i2c_write(f, REG_IPOLA, 0x00);
	i2c_write(f, REG_IPOLB, 0x00);

	/* Enable interrupts on all lines */
	i2c_write(f, REG_GPINTENA, 0xff);
	i2c_write(f, REG_GPINTENB, 0xff);

	/* Interrupt on change, all lines */
	i2c_write(f, REG_INTCONA, 0x00);
	i2c_write(f, REG_INTCONB, 0x00);
}

uint32_t i2c_read(int f, int reg) {
	struct i2c_smbus_ioctl_data args;
        union i2c_smbus_data data;

        args.read_write = I2C_SMBUS_READ;
        args.command = reg;
        args.size = I2C_SMBUS_BYTE_DATA;
        args.data = &data;
        if (ioctl(f, I2C_SMBUS, &args) !=  0)  {
                perror("I2C read");
		fprintf(stderr, "Unable to access I2C\n");
		return 0;
        }

        return data.byte & 0xff;
}

void i2c_write(int f, int reg, int value) {
	struct i2c_smbus_ioctl_data args;
        union i2c_smbus_data data;

	data.byte = value;
        args.read_write = I2C_SMBUS_WRITE;
        args.command = reg;
        args.size = I2C_SMBUS_BYTE_DATA;
        args.data = &data;
        if (ioctl(f, I2C_SMBUS, &args) !=  0)  {
                perror("I2C write");
		fprintf(stderr, "Unable to access I2C\n");
        }
}

