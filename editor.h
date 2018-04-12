#ifndef __VED_EDITOR_H__
#define __VED_EDITOR_H__

#include "buffer.h"

struct editor {
	struct buffer buffer;
};

int editor_init(struct editor *ved, char *filename);

#endif
