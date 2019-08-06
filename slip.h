#ifndef SLIP_H
#define SLIP_H

#define STATE_IDLE 0
#define STATE_INFRAME 1
#define STATE_ESC 2

#define END 0xC0
#define ESC 0xDB
#define ESC_END 0xDC
#define ESC_ESC 0xDD

void* recv_func(void *ptr);
void slip_init();
void slip_send(char *text, int len);
void open_port();

extern int state;
extern int slip_fd;
extern char recv_buffer[46*1024];
extern int recv_buffer_len;

#endif
