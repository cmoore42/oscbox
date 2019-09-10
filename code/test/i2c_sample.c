#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <stdint.h>

int bus=0;
int addr=0x20;

uint32_t i2c_read(int f, int reg);

int main() {
	int fd;
	unsigned long funcs;
	unsigned char res;
	int i;

	fd = open("/dev/i2c-0", O_RDWR);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		perror("ioctl");
		return 2;
	}


	ioctl(fd, I2C_FUNCS, &funcs);
	printf("Funcs: 0x%lx\n", funcs);

	for (i=0; i<0x16; i++) {
		res = i2c_read(fd, i);
		printf("Reg 0x%02x is 0x%02x\n", i, res);
	}


	close(fd);
}

uint32_t i2c_read(int f, int reg) {
	struct i2c_smbus_ioctl_data args;
	union i2c_smbus_data data;

	args.read_write = I2C_SMBUS_READ;
	args.command = reg;
	args.size = I2C_SMBUS_BYTE_DATA;
	args.data = &data;
	if (ioctl(f, I2C_SMBUS, &args) !=  0)  {
		perror("ioctl");
	}

	return data.byte & 0xff;
}
