PROGRAM = oscbox
OBJS = main.o tinyosc.o
LIBS = -lpthread -lgpiod

$(PROGRAM):	$(OBJS)
	cc -g -o $(PROGRAM) $(OBJS) $(LIBS)

main.o:	main.c
	cc -g -c main.c

tinyosc.o:	tinyosc.c
	cc -g -c tinyosc.c

clean:
	rm -f $(PROGRAM) $(OBJS)
