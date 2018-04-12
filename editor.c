#include "editor.h"

int editor_init(struct editor *ved, char *filename) {
	if (buf_init(&ved->buffer, filename)) return -1;
	return 0;
}
