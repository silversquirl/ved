#include "editor.h"

int editor_init(struct editor *ved, char *s) {
  ved->buffer = make_rope(1);
  if (!ved->buffer) return -1;
  if (rope_init(&ved->buffer, s, NULL)) return -1;
  return 0;
}
