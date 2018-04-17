#include "editor.h"
#include "ui.h"

int main() {
	struct editor ved;
	switch (editor_init(&ved, "ui.c")) {
	case -1:
		perror("editor_init");
		return 1;

	case -2:
		fprintf(stderr, "editor_init: Invalid format for 'keys'\n");
		return 1;
	case -3:
		fprintf(stderr, "editor_init: Command exists\n");
		return 1;
	}

	struct ui *ui;
	if (!(ui = ui_init(&ved))) {
		perror("ui_init");
		return 1;
	}

	ui_mainloop(ui);

	ui_free(ui);
	editor_destroy(ved);
}
