#include <cbuffer.h>

void cb_inc_p(struct cbuffer *cb, char **pointer) {
	(*pointer)++;
	if ((*pointer) == cb->first + BUFFER_SIZE) (*pointer) = cb->first;
}

char *cb_previous(struct cbuffer *cb, char *pointer) {
	return (pointer == cb->first) ? cb->first + BUFFER_SIZE : pointer - 1;
}

void cb_init(struct cbuffer *cb) {
	cb->write_pointer = cb->first;
	cb->read_pointer = cb->first;
}

int cb_read(struct cbuffer *cb, char *buffer, int count) {
	int i;
	for (i = 0; i < count; ++i) {
		if (cb->read_pointer == cb->write_pointer) return i;
		
		buffer[i] = *(cb->read_pointer);
		cb_inc_p(cb, &cb->read_pointer);
	}
	return i;
}

int cb_write(struct cbuffer *cb, char c) {
	int i;
	if (cb->write_pointer == cb_previous(cb, cb->read_pointer)) return i;

	*(cb->write_pointer) = c;
	
	cb_inc_p(cb, &cb->write_pointer);
	return i;
}

int cb_overwrite(struct cbuffer *cb, char *buffer, int size) {
	int i;
	for (i = 0; i < size; ++i) {
		*(cb->write_pointer) = buffer[i];
		cb_inc_p(cb, &cb->write_pointer);
		if (cb->write_pointer == cb_previous(cb, cb->read_pointer)) cb_inc_p(cb, &cb->read_pointer);
	}
	return i;
}

int cb_size(struct cbuffer *cb) {
	char *rp = cb->read_pointer;
	char *wp = cb->write_pointer;
	char *end = cb->first + BUFFER_SIZE;
	char *first = cb->first;
	return (unsigned int)(rp <= wp) ? wp - rp : (end - rp) + (wp - first);
}

char cb_full(struct cbuffer *cb) {
	return cb_size(cb) == BUFFER_SIZE - 1;
}
