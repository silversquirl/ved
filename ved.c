#include <stdio.h>
#include "editor.h"

int main() {
  struct editor ved;
  editor_init(&ved, "Hello, world!\n");
  char buf[256];
  while (fgets(buf, sizeof buf, stdin)) {
    rope_print(ved.buffer);
  }
}
