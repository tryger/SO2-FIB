#ifndef DEVICES_H__
#define  DEVICES_H__

#include <cbuffer.h>

int sys_write_console(char *buffer,int size);

extern struct cbuffer keyboard_buffer;
#endif /* DEVICES_H__*/
