#ifndef __VED_BUFFER_H__
#define __VED_BUFFER_H__

#include <stddef.h>

#define EDIT_ALLOC_STEP (1<<10) // 1KiB

struct buffer {
	struct {
		int fd;
		size_t len;
		char *buf;
	} file;

	struct {
		// Always starts at 0 (the beginning of the file)
		size_t end, len, alloc;
		char *buf;
	} edit;

	// Called when the file is modified
	void (*damage_cb)(void *userdata);
	void *damage_data;
};

int buf_init(struct buffer *b, char *filename);
void buf_free(struct buffer b);
void buf_sync(struct buffer b);
int buf_view_extend(struct buffer *b);
int buf_view_shrink(struct buffer *b, size_t decrement);
// TODO: editing functions. Make sure they call `damage_cb`!

#endif
