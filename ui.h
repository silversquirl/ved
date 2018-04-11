#ifndef __VED_UI_H__
#define __VED_UI_H__

#include "editor.h"

struct ui;

struct ui *ui_init(struct editor *ved);
void ui_mainloop(struct ui *ui);
int ui_add_action(struct ui *ui, char *key_name, void (*press)(void *), void (*release)(void *), void *data);
void ui_quit(struct ui *ui);

#endif
