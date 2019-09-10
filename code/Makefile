PROGRAM = oscbox
OBJS = main.o tinyosc.o i2c.o display.o encoders.o slip.o touch.o eos.o globals.o
LIBS = -lpthread -lgpiod

all: $(PROGRAM)

$(PROGRAM):	$(OBJS)
	cc -g -o $(PROGRAM) $(OBJS) $(LIBS)

main.o:	main.c
	cc -g -c main.c

tinyosc.o:	tinyosc.c
	cc -g -c tinyosc.c

i2c.o:	i2c.c
	cc -g -c i2c.c

display.o:	display.c
	cc -g -c display.c
	
encoders.o:	encoders.c
	cc -g -c encoders.c
	
slip.o:	slip.c
	cc -g -c slip.c
	
touch.o:	touch.c
	cc -g -c touch.c
	
eos.o:	eos.c
	cc -g -c eos.c	
	
globals.o:	globals.c
	cc -g -c globals.c

clean:
	rm -f $(PROGRAM) $(OBJS)
