#ifndef _CIRCULAR_BUFFER_H
#define _CIRCULAR_BUFFER_H

#define BUFFER_SIZE 4

struct cbuffer {
	char first[BUFFER_SIZE];
	char* read_pointer;
	char* write_pointer;
};

void cb_init(struct cbuffer *cb);

int cb_read(struct cbuffer *cb, char *buffer, int count);

int cb_write(struct cbuffer *cb, char c);

int cb_overwrite(struct cbuffer *cb, char *buffer, int size);

int cb_size(struct cbuffer *cb);

char cb_full(struct cbuffer *cb);

#endif /* _CIRCULAR_BUFFER_H */
