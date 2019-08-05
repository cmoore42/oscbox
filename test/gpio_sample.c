#include <stdio.h>
#include <gpiod.h>

int main() {
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	int rv;
	int value;

	chip = gpiod_chip_open("/dev/gpiochip0");
	if (!chip) {
		perror("gpiod_chip_open");
		return 1;
	}

	line = gpiod_chip_get_line(chip, 6);
	if (!line) {
		perror("gpiod_chip_get_line");
		gpiod_chip_close(chip);
		return 1;
	}

	rv = gpiod_line_request_input(line, "GPIO6");
	if (rv) {
		perror("gpiod_line_request_input");
		gpiod_chip_close(chip);
		return 1;
	}

	value = gpiod_line_get_value(line);

	printf("Value is %d\n", value);

	gpiod_chip_close(chip);
	return 0;
}
		
