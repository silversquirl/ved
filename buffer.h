#ifndef __VED_BUFFER_H__
#define __VED_BUFFER_H__

#include <stddef.h>
#include <stdio.h>

struct buffer {
	struct {
		int fd;
		size_t len;
		char *buf;
	} file;

	struct {
		size_t start, end, len;
		char *buf;
	} edit;
};

int buf_init(struct buffer *b, char *filename);
void buf_free(struct buffer b);
void buf_sync(struct buffer b);

#endif
