// TODO: modifiers (<Ctrl-A>, <Mod4-@>, etc.)
// TODO: default key handler (for edit mode)
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include "command.h"
#include "ui_internal.h"
#include "util.h"

#define CMD_ALLOC_INIT 32

struct command_matcher {
	vtk_key key;
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

static void cmd_node_free(struct command_node *node) {
	if (!node->leaf)
		for (size_t i = 0; i < node->n; ++i)
			cmd_node_free(node->options[i].node);

	free(node);
}

struct commands *cmd_create(void) {
	struct command_node *node = cmd_node_create();
	struct commands *c = malloc(sizeof *c);
	c->root = node;
	c->current = node;
	return c;
}

void cmd_free(struct commands *cmds) {
	cmd_node_free(cmds->root);
	free(cmds);
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

static inline struct command_matcher *cmd_find_option(struct command_node *node, vtk_key key) {
	for (size_t i = 0; i < node->n; ++i)
		if (node->options[i].key == key) return node->options + i;
	return NULL;
}

static vtk_key parse_key(char **s) {
	char *p;
	vtk_key ret;
	switch (**s) {
	case '<':
		++*s;
		p = strchr(*s, '>');
		if (!p) return VTK_K_NONE;
		*p = '\0';

		// FIXME: Would be nice if vtk provided this
		if (!strcasecmp(*s, "BACKSPACE")) ret = VTK_K_BACKSPACE;
		else if (!strcasecmp(*s, "TAB")) ret = VTK_K_TAB;
		else if (!strcasecmp(*s, "RETURN")) ret = VTK_K_RETURN;
		else if (!strcasecmp(*s, "ESCAPE")) ret = VTK_K_ESCAPE;
		else if (!strcasecmp(*s, "SPACE")) ret = VTK_K_SPACE;
		else if (!strcasecmp(*s, "DELETE")) ret = VTK_K_DELETE;
		else if (!strcasecmp(*s, "INSERT")) ret = VTK_K_INSERT;

		else if (!strcasecmp(*s, "PAGE UP")) ret = VTK_K_PAGE_UP;
		else if (!strcasecmp(*s, "PAGE DOWN")) ret = VTK_K_PAGE_DOWN;
		else if (!strcasecmp(*s, "HOME")) ret = VTK_K_HOME;
		else if (!strcasecmp(*s, "END")) ret = VTK_K_END;
		else if (!strcasecmp(*s, "UP")) ret = VTK_K_UP;
		else if (!strcasecmp(*s, "DOWN")) ret = VTK_K_DOWN;
		else if (!strcasecmp(*s, "LEFT")) ret = VTK_K_LEFT;
		else if (!strcasecmp(*s, "RIGHT")) ret = VTK_K_RIGHT;

		else return VTK_K_NONE;

		*s = p + 1;
		return ret;

	case '\\':
		++*s;
	default:
		if (**s >= ' ' && **s <= '~') return *(*s)++;
		return VTK_K_NONE;
	}
}

int cmd_register(struct commands *cmds, char *keys, command_callback cb, void *userdata) {
	struct command_node *node = cmds->root;
	struct command_matcher opt, *tmp;
	bool leaf;
	do {
		if ((opt.key = parse_key(&keys)) == VTK_K_NONE) return -2;
		leaf = !*keys;

		tmp = cmd_find_option(node, opt.key);
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

int cmd_handle_key(struct commands *cmds, struct ui *ui, int key) {
	struct command_matcher *m = cmd_find_option(cmds->current, key);
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
