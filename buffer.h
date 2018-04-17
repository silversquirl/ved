#ifndef __VED_BUFFER_H__
#define __VED_BUFFER_H__

#include <stddef.h>
#include <stdio.h>

enum buffer_section {
	BUF_SEC_FILE = 1 << 1,
	BUF_SEC_EDIT = 1 << 2,
	BUF_SEC_ALL  = -1,
};

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

	// sections is an OR of values from buffer_section
	void (*damage_cb)(unsigned int sections, void *userdata);
	void *damage_data;
};

int buf_init(struct buffer *b, char *filename);
void buf_free(struct buffer b);
void buf_sync(struct buffer b);
// TODO: editing functions. Make sure they call `damage_cb`!

#endif
