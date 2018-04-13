#include "editor.h"
#include "ui.h"

void quit(void *ui) {
	ui_quit(ui);
}

void keypress(XKeyEvent *xk, struct ui *ui) {
	if (XKeysymToKeycode(ui->dpy, XK_Escape) == xk->keycode) ui_quit(ui);
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

	vev_register(ui->ev.keypress, keypress, ui);
	if (errno) {
		perror("vev_register");
		return 1;
	}

	ui_mainloop(ui);
}
