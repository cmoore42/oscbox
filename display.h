#ifndef DISPLAY_H
#define DISPLAY_H

void disp_open();
void disp_close();
void disp_point(int x, int y, int r, int g, int b);
void disp_line(int from_x, int from_y, int to_x, int to_y, int r, int g, int b);
void disp_char(int x, int y, char c, int r, int g, int b);
void disp_string(int x, int y, char *s, int r, int g, int b);
void disp_flush();
void disp_clear();
void disp_clear_range(int x0, int y0, int x1, int y1);
int disp_font_height();
int disp_font_width();

extern int disp_fd;

#endif
