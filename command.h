#ifndef __VED_KEYBOARD_H__
#define __VED_KEYBOARD_H__

struct ui;

typedef void (*command_callback)(struct ui *ui, void *);
struct commands;

// Returns NULL on error. Check errno
struct commands *cmd_create(void);

void cmd_free(struct commands *cmds);

// 'keys' may be permuted
// Error codes:
//  -1 UNIX error. Check errno
//  -2 Invalid format for 'keys'
//  -3 Command already exists for a prefix of 'keys' OR
//     'keys' is a prefix of an existing command
int cmd_register(struct commands *cmds, char *keys, command_callback cb, void *userdata);

// Returns 0 if branch found, -1 if not found, 1 if command executed
int cmd_handle_key(struct commands *cmds, struct ui *ui, int key);

void cmd_reset_pos(struct commands *cmds);

#endif
