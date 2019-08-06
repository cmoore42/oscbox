#ifndef TOUCH_H
#define TOUCH_H

typedef void (*touch_callback)(void *arg);

struct touch_region {
	struct touch_region *next;
	int	x0;	// Upper left x in pixels
	int 	y0;	// Upper left y in pixels
	int 	x1;	// Lower right x in pixels
	int	y1;	// Lower right y in pixels
	touch_callback cb;
	void	*arg;
};

void *touch_func(void *);
void add_region(int x0, int y0, int x1, int y1, touch_callback cb, void *arg);

extern struct touch_region *regions;

#endif
