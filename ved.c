#include "editor.h"
#include "ui.h"
#include "ui_internal.h"

void handle_keypress(XKeyEvent *xk, struct ui *ui) {
	cmd_handle_key(ui->ved->modes.current, ui, xk->keycode);
}

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

	vev_register(ui->ev.keypress, handle_keypress, ui);
	if (errno) {
		perror("vev_register");
		return 1;
	}

	ui_mainloop(ui);
}
