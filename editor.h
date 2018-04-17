#ifndef __VED_EDITOR_H__
#define __VED_EDITOR_H__

#include "buffer.h"
#include "command.h"

struct editor {
	struct buffer buffer;
	struct {
		struct commands *command, *edit, *current;
	} modes;
};

int editor_init(struct editor *ved, char *filename);
void editor_destroy(struct editor ved);

#endif
