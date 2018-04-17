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
		size_t start, end, len, alloc;
		char *buf;
	} edit;

	// Called when the file is modified
	void (*damage_cb)(void *userdata);
	void *damage_data;
};

int buf_init(struct buffer *b, char *filename);
void buf_free(struct buffer b);
void buf_sync(struct buffer b);
void buf_view_init(struct buffer *b, size_t start);
int buf_view_extend(struct buffer *b);
// TODO: editing functions. Make sure they call `damage_cb`!

#endif
