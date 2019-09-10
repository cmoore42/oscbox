#include <stdio.h>
#include <unistd.h>
#include "../i2c.h"

int bus=0;
int addr=0x20;


int main() {
	int fd;
	unsigned long funcs;
	int res;
	int i;

	fd = i2c_open(bus, addr);
	if (fd < 0) {
		perror("i2c_open");
		return 1;
	}

	i2c_init(fd);

	res = i2c_read(fd, REG_IOCON);
	printf("REG_IOCON is 0x%02x\n", res);

	while (1) {
		res = i2c_read(fd, REG_GPIOA);
		printf("REG_GPIOA is 0x%02x\n", res);

		sleep(1);
	}

	close(fd);
}

