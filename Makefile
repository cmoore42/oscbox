PROGRAM = oscbox
OBJS = main.o tinyosc.o i2c.o
LIBS = -lpthread -lgpiod

$(PROGRAM):	$(OBJS)
	cc -g -o $(PROGRAM) $(OBJS) $(LIBS)

main.o:	main.c
	cc -g -c main.c

tinyosc.o:	tinyosc.c
	cc -g -c tinyosc.c

i2c.o:	i2c.c
	cc -g -c i2c.c

clean:
	rm -f $(PROGRAM) $(OBJS)
