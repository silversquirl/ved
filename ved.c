#include "editor.h"
#include "ui.h"

int main() {
  struct editor ved;
  if (editor_init(&ved, "Hello, world!\n")) {
    perror("editor_init");
    return 1;
  }
  struct ui ui;
  if (ui_init(&ui, &ved)) {
    perror("ui_init");
    return 1;
  }
  ui_mainloop(&ui);
}
