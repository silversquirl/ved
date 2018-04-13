#include <stdint.h>
#include <stdbool.h>
#include "command.h"
#include "ui_internal.h"
#include "util.h"

#define CMD_ALLOC_INIT 32

struct command_matcher {
	KeySym sym;
	struct command_node *node;
};

struct command_node {
	bool leaf;
	union {
		// if leaf
		struct {
			void *ud;
			command_callback cb;
		};

		// if !leaf
		struct {
			size_t alloc, n;
			struct command_matcher *options;
		};
	};
};

struct commands {
	struct command_node *root, *current;
};

static struct command_node *cmd_node_create(void) {
	struct command_node *node = malloc(sizeof *node);
	if (!node) return NULL;
	node->leaf = false;
	node->alloc = 0;
	node->options = NULL;
	return node;
};

struct commands *cmd_create(void) {
	struct command_node *node = cmd_node_create();
	struct commands *c = malloc(sizeof *c);
	c->root = node;
	c->current = node;
	return c;
}

static int cmd_add_option(struct command_node *node, struct command_matcher opt) {
	if (node->alloc <= node->n + 1) {
		node->alloc = node->alloc ? CMD_ALLOC_INIT : node->alloc * 2;
		struct command_matcher *opts = realloc_array(node->options, node->alloc);
		if (!opts) return -1;
		node->options = opts;
	}

	node->options[node->n++] = opt;
	return 0;
}

static inline struct command_matcher *cmd_find_option(struct command_node *node, KeySym sym) {
	for (size_t i = 0; i < node->n; ++i)
		if (node->options[i].sym == sym) return node->options + i;
	return NULL;
}

static inline struct command_matcher *cmd_find_option_by_keycode(struct command_node *node, Display *dpy, KeyCode c) {
	for (size_t i = 0; i < node->n; ++i)
		if (XKeysymToKeycode(dpy, node->options[i].sym) == c)
			return node->options + i;
	return NULL;
}

static KeySym parse_keysym(char **s) {
	char *p;
	KeySym ret;
	switch (**s) {
	case '<':
		p = strchr(*s, '>');
		if (!p) return NoSymbol;
		*p = '\0';
		ret = XStringToKeysym(*s + 1);
		*s = p + 1;
		return ret;

	case '\\':
		++*s;
	default:
		if (**s >= XK_space && **s <= XK_asciitilde) return *(*s)++;
		return NoSymbol;
	}
}

int cmd_register(struct commands *cmds, char *keys, command_callback cb, void *userdata) {
	struct command_node *node = cmds->root;
	struct command_matcher opt, *tmp;
	bool leaf;
	do {
		if ((opt.sym = parse_keysym(&keys)) == NoSymbol) return -2;
		leaf = !*keys;

		tmp = cmd_find_option(node, opt.sym);
		if (tmp) {
			if (tmp->node->leaf || leaf) return -3;
		} else {
			opt.node = cmd_node_create();
			opt.node->leaf = leaf;
			if (leaf) {
				opt.node->ud = userdata;
				opt.node->cb = cb;
			}
			cmd_add_option(node, opt);
			node = opt.node;
		}
	} while (!leaf);
	return 0;
}

int cmd_handle_key(struct commands *cmds, struct ui *ui, unsigned int keycode) {
	struct command_matcher *m = cmd_find_option_by_keycode(cmds->current, ui->dpy, keycode);
	if (!m) {
		cmd_reset_pos(cmds);
		return -1;
	}

	if (m->node->leaf) {
		m->node->cb(ui, m->node->ud);
		cmd_reset_pos(cmds);
		return 1;
	} else {
		cmds->current = m->node;
	}
	return 0;
}

void cmd_reset_pos(struct commands *cmds) { cmds->current = cmds->root; }
