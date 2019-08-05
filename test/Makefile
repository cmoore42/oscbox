CFLAGS = -g
PROGRAMS = gpio_sample i2c_sample i2c_sample2 tft tft2 touchscreen

all:	$(PROGRAMS)

clean:
	rm *.o $(PROGRAMS)

gpio_sample:	gpio_sample.o
	cc -o gpio_sample gpio_sample.o -lgpiod

i2c_sample:	i2c_sample.o

i2c_sample2:	i2c_sample2.o ../i2c.o

tft:	tft.o

tft2:	tft2.o ../display.o
	cc -o tft2 tft2.o ../display.o

touchscreen:	touchscreen.o
