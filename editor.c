#include <stdlib.h>
#include <string.h>
#include "editor.h"
#include "ui.h"

struct command_pair {
	char *keys;
	command_callback cb;
};

static void editor_quit(struct ui *ui, void *d) {
	ui_quit(ui);
}

// TODO: mode switching
const struct command_pair command_mode[] = {
	{ "<Escape>", editor_quit },
	{ NULL }
};

// TODO: edit mode
const struct command_pair edit_mode[] = {
	{ NULL }
};

static int cmd_init(struct editor *ved, struct commands *cmds, const struct command_pair *tbl) {
	int ret;
	for (const struct command_pair *p = tbl; p->keys; ++p) {
		char *k = strdup(p->keys);
		if ((ret = cmd_register(cmds, k, p->cb, ved))) return ret;
		free(k);
	}
	return 0;
}

int editor_init(struct editor *ved, char *filename) {
	int ret;
	if (buf_init(&ved->buffer, filename)) return -1;
	if (!(ved->modes.command = cmd_create())) return -1;
	if ((ret = cmd_init(ved, ved->modes.command, command_mode))) return ret;
	if (!(ved->modes.edit = cmd_create())) return -1;
	if ((ret = cmd_init(ved, ved->modes.edit, edit_mode))) return ret;
	ved->modes.current = ved->modes.command;
	return 0;
}

void editor_destroy(struct editor ved) {
	buf_free(ved.buffer);
	cmd_free(ved.modes.command);
	cmd_free(ved.modes.edit);
}
