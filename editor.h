#ifndef __VED_EDITOR_H__
#define __VED_EDITOR_H__

#include "rope.h"

struct editor {
  rope buffer;
};

// TODO: make this read a file instead of a string
int editor_init(struct editor *ved, char *s);

#endif
