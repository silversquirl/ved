#include "editor.h"
#include "ui.h"

void quit(void *ui) {
	ui_quit(ui);
}

int main() {
	struct editor ved;
	if (editor_init(&ved, "ui.c")) {
		perror("editor_init");
		return 1;
	}

	struct ui *ui;
	if (!(ui = ui_init(&ved))) {
		perror("ui_init");
		return 1;
	}

	if (ui_add_action(ui, "Escape", quit, NULL, ui))
	return 1;

	ui_mainloop(ui);
}
